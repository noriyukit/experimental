#include "overload.h"
#include <iostream>

using namespace std;
using namespace overload;

struct A {
  ~A() {
    cout << __func__ << endl;
  }

  A() {
  }

  A(const A&) {
    cout << "copy!" << endl;
  }

  A(A&&) {
    cout << "move!" << endl;
  }

  int operator()() {
    cout << "void void" << __func__ << endl;
    return 0;
  }

  int operator()(int) {
    cout << __func__ << endl; 
    return a;
  }
  int operator()(int, int) {
    cout << __func__ << endl; 
    return 2;
  }
  void operator()(double) {
    cout << "void double" << endl;
  }
  int a;
};

int main() {
  {
    A a;
    Overload<int(), int(int), int(int, int), void(double)> oo = a;
    decltype(oo) o = std::move(oo);
    o();
    cout << o(10) << endl;
    cout << o(10, 20) << endl;
    o(1.0);
  }
  return 0;
}
