#include "../src/dynamic-cuckoofilter.h"

#include <assert.h>

#include <algorithm>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "generators.h"

using namespace cuckoofilterbio1;

void test_construct_DCF() {
  std::unique_ptr<DynamicCuckooFilter<>> dcf =
      std::make_unique<DynamicCuckooFilter<>>(10);

  std::cout << "PASS test_construct_DCF" << std::endl;
}
void test_add_DCF() {
  std::unique_ptr<DynamicCuckooFilter<>> dcf =
      std::make_unique<DynamicCuckooFilter<>>(512);
  for (int i = 0; i < 1000; i++) {
    assert(Ok == dcf->Add(generateKMer(20)));
  }

  std::cout << "PASS test_add_DCF" << std::endl;
}

void test_delete_DCF() {
  std::unique_ptr<DynamicCuckooFilter<uint16_t>> dcf =
      std::make_unique<DynamicCuckooFilter<uint16_t>>(512);
  std::vector<std::string> s;
  int br = 1;
  for (int i = 0; i < 1000; i++) {
    if (i == br * 10 && br < 12) {
      s.push_back(generateKMer(20));
      assert(Ok == dcf->Add(s[br - 1]));
      br++;
    } else {
      assert(Ok == dcf->Add(generateKMer(20)));
    }
  }
  for (int i = 0; i < 10; i++) {
    assert(Ok == dcf->Delete(s[i]));
  }

  std::cout << "PASS test_delete_DCF" << std::endl;
}

void test_contains_DCF() {
  std::unique_ptr<DynamicCuckooFilter<uint32_t>> dcf =
      std::make_unique<DynamicCuckooFilter<uint32_t>>(512);
  std::vector<std::string> s;
  int br = 1;
  for (int i = 0; i < 1000; i++) {
    if (i == br * 10 && br < 12) {
      s.push_back(generateKMer(20));
      assert(Ok == dcf->Add(s[br - 1]));
      br++;
    } else {
      assert(Ok == dcf->Add(generateKMer(20)));
    }
  }
  for (int i = 0; i < 10; i++) {
    assert(Ok == dcf->Contains(s[i]));
  }

  std::cout << "PASS test_contains_DCF" << std::endl;
}

void test_compact_DCF() {
  std::unique_ptr<DynamicCuckooFilter<uint16_t>> dcf =
      std::make_unique<DynamicCuckooFilter<uint16_t>>(512);
  std::vector<std::string> s;
  for (int i = 0; i < 2000; i++) {
    if (i < 300) {
      s.push_back(generateKMer(20));
      assert(Ok == dcf->Add(s[i]));
    } else {
      assert(Ok == dcf->Add(generateKMer(20)));
    }
  }
  std::for_each(s.begin(), s.end(),
                [&](std::string &str) { assert(Ok == dcf->Delete(str)); });
  vector<size_t> sizes = dcf->SizeOfEachCF();
  assert(Ok == dcf->Compact());
  vector<size_t> sizes2 = dcf->SizeOfEachCF();
  assert(sizes != sizes2);
  assert(Ok == dcf->Add(generateKMer(20)));
  std::cout << "PASS test_compact_DCF" << std::endl;
}

int main(int argc, const char *argv[]) {
  test_construct_DCF();
  test_add_DCF();
  test_delete_DCF();
  test_contains_DCF();
  test_compact_DCF();
  return 0;
}