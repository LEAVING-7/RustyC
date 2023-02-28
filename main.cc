#include <iostream>
struct Base {
  int id;
  Base(int id) : id(id) {}
  virtual void foo() const { puts("base"); }
};
struct Derive : Base {
  int a[44];
  Derive() : Base(1) {}
  virtual void foo() const override { puts("1"); }
};
struct Derive2 : Base {
  int c[66];
  Derive2() : Base(2) {}
  virtual void foo() const override { puts("2"); }
};

int main(int argc, char* argv[]) { Derive2 d2; }