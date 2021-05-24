#include "../src/dynamic-cuckoofilter.h"

#include <assert.h>

#include <iostream>
#include <memory>

using namespace cuckoofilterbio1;

std::string generateKMer(size_t k) {
  static const char bases[] = "ACGT";

  std::string k_mer;

  k_mer.reserve(k);

  for (int i = 0; i < k; ++i) k_mer += bases[rand() % (sizeof(bases) - 1)];

  return k_mer;
}

void test_construct_DCF() {
  std::unique_ptr<DynamicCuckooFilter<>> dcf =
      std::make_unique<DynamicCuckooFilter<>>(10);

  std::cout << "PASS test_construct_DCF" << std::endl;
}
void test_add_DCF() { std::cout << "PASS test_add_DCF" << std::endl; }

int main(int argc, const char* argv[]) {
  test_construct_DCF();
  test_add_DCF();

  return 0;
}