#ifndef _CLS_HPP_
#define _CLS_HPP_

namespace NN {
namespace MM {

class A {
protected:
  int a;
public:
  A(int a0 = 0);
  virtual ~A();
  virtual void foo();
};

class B : virtual public A {
public:
  B(int a0 = 0);
  virtual ~B();
  virtual void foo();
};

class C : virtual public A {
public:
  C(int a0 = 0);
  virtual ~C();
  virtual void foo();
};

class D : public B, public C {
public:
  D(int a0 = 0);
  virtual ~D();
  virtual void foo();
};

}
}

#endif

