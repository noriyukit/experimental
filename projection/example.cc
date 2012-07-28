// An example of usage.
//
// BUILD:
//   clang++ -std=c++0x example.cc

#include <iostream>
#include "projection.h"

using namespace std;

template <typename T, size_t N>
void Print(T (&array)[N]) {
  cout << "(";
  for (size_t i = 0; i < N - 1; ++i) {
    cout << array[i] << ", ";
  }
  cout << array[N - 1] << ")";
}

template <typename T, size_t N>
size_t ArraySize(T (&)[N]) {
  return N;
}

int main() {
  // Example for double type.
  {
    const double l[]{0, 0};
    const double u[]{1, 1};
    const double a[]{1, 1};
    const double b = 1.5;
    Projector<double, const double*, const double*, const double*>
        proj(2, a, b, l, u);

    const double x0[][2]{ {1, 1}, {2, 0.5} };
    for (size_t i = 0; i < ArraySize(x0); ++i) {
      double x[2];
      proj.Project(x0[i], x);
      cout << "P";
      Print(x0[i]);
      cout << " = ";
      Print(x);
      cout << endl;
    }
  }
  // Example for float type.
  {
    const float l[]{0, 0};
    const float u[]{1, 1};
    const float a[]{1, 1};
    const float b = 1.5;
    Projector<float, const float*, const float*, const float*>
        proj(2, a, b, l, u);

    const float x0[][2]{ {1, 3}, {-1, -1} };
    for (size_t i = 0; i < ArraySize(x0); ++i) {
      float x[2];
      proj.Project(x0[i], x);
      cout << "P";
      Print(x0[i]);
      cout << " = ";
      Print(x);
      cout << endl;
    }
  }
  return 0;
}
