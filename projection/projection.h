// Algorithm to compute metric projection onto the intersection of a box and a
// hyperplane in Euclidean space.

#include <cassert>
#include <cstddef>
#include <tuple>
#include <vector>

// Computes the projection of x0 onto the intersection of a box B and a
// hyperplane H, where
//
//     B = { x | l_i <= x_i <= u_i,  for all i = 1,..., N },
//     H = { x | <a, x> = b }.
//
// Namely, solves the following quadratic program:
//
//       minimize  ||x - x0||^2
//     subject to  x \in B
//                 x \in H
//
// The algorithm requires O(N) memory space to solve the problem in O(N) on
// average, where N is the dimension of vector space.
template <typename RealType, typename NormalVectorIterator,
          typename LowerBoundIterator, typename UpperBoundIterator>
class Projector final {
 public:
  Projector(std::size_t dim,
            NormalVectorIterator nv_first, RealType plane_translation,
            LowerBoundIterator lb_first, UpperBoundIterator ub_first)
      : dim_(dim), normal_vector_begin_(nv_first),
        lower_bound_begin_(lb_first), upper_bound_begin_(ub_first),
        target_(plane_translation) {
    auto num_nonzeros = dim;
    for (; dim-- > 0; ++nv_first, ++lb_first, ++ub_first) {
      const auto v = *nv_first;
      if (v > 0) {
        target_ -= v * *ub_first;
      } else if (v < 0) {
        target_ -= v * *lb_first;
      } else {
        --num_nonzeros;
      }
    }
    breakpoints_.resize(2 * num_nonzeros);
  }

  template <typename ForwardIterator, typename OutputIterator>
  bool Project(ForwardIterator vector_first,
               OutputIterator projection_first) const {
    bool feasible;
    RealType solution;
    std::tie(feasible, solution) = SolveEquation(vector_first);
    if (!feasible) return false;
    auto nv_iter = normal_vector_begin_;
    auto lb_iter = lower_bound_begin_;
    auto ub_iter = upper_bound_begin_;
    for (auto dim = dim_; dim-- > 0;
         ++vector_first, ++nv_iter, ++lb_iter, ++ub_iter, ++projection_first) {
      const auto x = *vector_first - solution * *nv_iter;
      *projection_first =
          (x < *lb_iter) ? *lb_iter : ((x > *ub_iter) ? *ub_iter : x);
    }
    return true;
  }

 private:
  struct Breakpoint {
    enum Category {
      POSITIVE_LOWER = 0,
      POSITIVE_UPPER,
      NEGATIVE_LOWER,
      NEGATIVE_UPPER
    };

    Breakpoint() : category(POSITIVE_LOWER), value(0), square(0) {}
    Breakpoint(Category c, RealType v, RealType s)
        : category(c), value(v), square(s) {}

    Category category;
    RealType value, square;
  };

  struct BreakpointGreater {
    bool operator()(const Breakpoint& l, const Breakpoint& r) const {
      return l.value > r.value;
    }
  };

  template <typename InputIterator, typename OutputIterator>
  void CalculateBreakpoints(InputIterator vector_first,
                            OutputIterator out_first) const {
    auto nv_iter = normal_vector_begin_;
    auto lb_iter = lower_bound_begin_;
    auto ub_iter = upper_bound_begin_;
    for (auto dim = dim_; dim-- > 0;
         ++vector_first, ++nv_iter, ++lb_iter, ++ub_iter) {
      const auto nv = *nv_iter;
      if (nv == 0) continue;
      const auto vec = *vector_first;
      const auto lower_bp = (vec - *lb_iter) / nv;
      const auto upper_bp = (vec - *ub_iter) / nv;
      const auto square = nv * nv;
      if (nv > 0) {
        *out_first++ = Breakpoint(Breakpoint::POSITIVE_LOWER, lower_bp, square);
        *out_first++ = Breakpoint(Breakpoint::POSITIVE_UPPER, upper_bp, square);
      } else {
        *out_first++ = Breakpoint(Breakpoint::NEGATIVE_LOWER, lower_bp, square);
        *out_first++ = Breakpoint(Breakpoint::NEGATIVE_UPPER, upper_bp, square);
      }
    }
  }

  template <typename InputIterator>
  std::pair<bool, RealType> SolveEquation(InputIterator vector_first) const {
    enum {
      PL = Breakpoint::POSITIVE_LOWER, PU = Breakpoint::POSITIVE_UPPER,
      NL = Breakpoint::NEGATIVE_LOWER, NU = Breakpoint::NEGATIVE_UPPER
    };
    CalculateBreakpoints(vector_first, std::begin(breakpoints_));
    struct { RealType slope, intercept; } param{0, 0};
    struct { RealType x, y; } left{0, 0}, right{0, 0};
    auto first = std::begin(breakpoints_), last = std::end(breakpoints_);
    while (first != last) {
      auto middle = std::next(first, std::distance(first, last) / 2);
      std::nth_element(first, middle, last, BreakpointGreater());
      decltype(param) delta[4]{{0, 0}, {0, 0}, {0, 0}, {0, 0}};
      for (auto iter = middle; iter != last; ++iter) {
        auto& d = delta[iter->category];
        d.slope += iter->square;
        d.intercept += iter->square * iter->value;
      }
      const decltype(param) new_param{
          (param.slope + delta[PL].slope - delta[PU].slope
           - delta[NL].slope + delta[NU].slope),
          (param.intercept - delta[PL].intercept + delta[PU].intercept
           + delta[NL].intercept - delta[NU].intercept)};
      const auto y = new_param.slope * middle->value + new_param.intercept;
      if (y < target_) {
        left.x = middle->value;
        left.y = y;
        first = ++middle;
      } else {
        right.x = middle->value;
        right.y = y;
        param = new_param;
        last = middle;
      }
    }
    if (first == std::end(breakpoints_) ||
        (first == std::begin(breakpoints_) && right.y != target_))
      return std::pair<bool, RealType>(false, 0);
    assert(left.y != right.y);
    const auto solution =
        left.x + (target_ - left.y) * (right.x - left.x) / (right.y - left.y);
    return std::pair<bool, RealType>(true, solution);
  }

  const std::size_t dim_;
  const NormalVectorIterator normal_vector_begin_;
  const LowerBoundIterator lower_bound_begin_;
  const UpperBoundIterator upper_bound_begin_;
  RealType target_;
  mutable std::vector<Breakpoint> breakpoints_;

  Projector() = delete;
  Projector(const Projector&) = delete;
  Projector(Projector&&) = delete;
  Projector& operator=(const Projector&) = delete;
  Projector& operator=(Projector&&) = delete;
};
