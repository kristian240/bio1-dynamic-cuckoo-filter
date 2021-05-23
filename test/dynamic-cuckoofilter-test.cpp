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
  std::unique_ptr<DynamicCuckooFilter<uint32_t>> dcf =
      std::make_unique<DynamicCuckooFilter<uint32_t>>(10);

  std::cout << "PASS test_construct_DCF" << std::endl;
}

int main(int argc, const char* argv[]) {
  test_construct_DCF();

  return 0;
}