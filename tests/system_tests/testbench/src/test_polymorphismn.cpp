#include "test_polymorphismn.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>

// Polymorphismn tests
class Base {
public:
  virtual uint32_t Call() { return 0xBEEFCAFE; }
};

class Derived : public Base {
public:
  virtual uint32_t Call() override { return 0xAFFE; }
};

void TestPolymorphismn() {
  printf("# Test: Polymorphismn\n");
  auto derived = Derived();
  assert(derived.Call() == 0xAFFE);
  auto *base = reinterpret_cast<Base *>(&derived);
  assert(base->Call() == 0xAFFE);
  auto base_constructed = Base();
  assert(base_constructed.Call() == 0xBEEFCAFE);
}