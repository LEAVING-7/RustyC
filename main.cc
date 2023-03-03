

#include <cassert>
#include <iostream>

#include <cassert>

#include <iostream>

template <typename Base, typename Base::Type type>
struct RTTI2 : public Base {
  template <typename... Args>
  RTTI2(Args... args) : Base(type, std::forward<Args>(args)...){};
  static void* ClassID() { return &ID; }
  inline static typename Base::Type ID = type;
};

#define DefID(...)                                                                                                     \
  enum class Type : size_t { __VA_ARGS__ };                                                                            \
  Type mType;

struct Base {
  DefID(d1, d2);

  int foo;
  Base(Type type, int foo) : mType(type), foo(foo) {}
  virtual ~Base() { puts("delete base"); }

public:
  virtual void print() { printf("%d", foo); }
  template <typename T>
  decltype(auto) as()
  {
    assert(T::ID == mType);
    return static_cast<T*>(this);
  }
};

class Derive1 : public RTTI2<Base, Base::Type::d1> {
public:
  DefID(d2);
  int fuck;

public:
  Derive1(int fucn) : RTTI2(123), fuck(fucn) {}
  ~Derive1() override { puts("delete d1"); }
  void print() override { printf("%d", fuck); }
};

struct Derive2 : public RTTI2<Derive1, Derive1::Type::d2> {};

template <class D, class B>
bool isa(B* b)
{
  return b->mType == D::ID;
}

class RTTIRoot {
public:
  virtual ~RTTIRoot() = default;

  static void const* ClassID() { return &ID; }
  virtual void const* dynamicClassID() const = 0;

  virtual bool isA(void const* const classID) const { return classID == ClassID(); }

  template <typename QueryT>
  bool isA() const
  {
    return isA(QueryT::classID);
  }

private:
  virtual void anchor();

  static char ID;
};

template <typename ThisT, typename ParentT>
class RTTIExtends : public ParentT {
public:
  using ParentT::ParentT;

  static void const* ClassID() { return &ThisT::ID; }
  void const* dynamicClassID() const override { return &ThisT::ID; }
  bool isA(void const* const classID) const override { return classID == ClassID() || ParentT::isA(classID); }
  static bool classof(RTTIRoot const* r) { return r->isA<ThisT>(); }
};

struct A {
  double x;
  virtual void f1(){};
  virtual void f2(){};
  virtual void f3(){};
};

struct B : virtual A {
  int a;
  void f1() override{};
  // void f2() override{};
  void f3() override{};
};

struct C : virtual A {
  double c;
  // void f1() override{};
  void f2() override{};
  // void f3() override{};
};

struct K : B, C {};

int main(int argc, char* argv[])
{
  static_assert(sizeof(C));
  auto&& D = K{};
  auto p = (uintptr_t*)&D;
  for (int i = 0; i < 6; ++i) {
    printf("%p\n", (void*)*(p + i));
  }
}