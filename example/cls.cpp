#include <iostream>
#include "cls.hpp"

using namespace std;
using namespace NN::MM;

A::A(int a0) : a(a0) {
  cout << "A::A(int)" << endl;
}
A::~A() {
  cout << "A::~A()" << endl;
}
void A::foo() {
  cout << "A::foo()" << a << endl;
}

B::B(int a0) : A(a0) {
  cout << "B::B(int)" << endl;
}
B::~B() {
  cout << "B::~B()" << endl;
}
void B::foo() {
  cout << "B::foo()" << a << endl;
}

C::C(int a0) : A(a0) {
  cout << "C::C(int)" << endl;
}
C::~C() {
  cout << "C::~C()" << endl;
}
void C::foo() {
  cout << "C::foo()" << a << endl;
}

D::D(int a0) : A(a0), B(a0 + 1), C(a0 + 2) {
  cout << "D::D(int)" << endl;
}
D::~D() {
  cout << "D::~D()" << endl;
}
void D::foo() {
  cout << "D::foo()" << a << endl;
}

