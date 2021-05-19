#include "../src/hash.h"

#include <assert.h>

#include <iostream>

void test_different_string() {
  std::string s1("ACATATGTCCGTATGTACATACCTACGGACGTACATACGA");
  std::string s2("TGGTACGGTCGTATGTGCTTGAGTAAGTACGTAAGTAACT");
  cuckoofilterbio1::Hash hasher;
  assert(hasher(s1) != hasher(s2));
  std::cout << "PASS test_different_string" << std::endl;
}

void test_same_string() {
  std::string s1("TCGATCTCTGTTCGGTATGCCACCAATTTCAAGGTAAACT");
  std::string s2("TCGATCTCTGTTCGGTATGCCACCAATTTCAAGGTAAACT");
  cuckoofilterbio1::Hash hasher;
  assert(hasher(s1) == hasher(s2));
  std::cout << "PASS test_same_string" << std::endl;
}

int main(int argc, const char* argv[]) {
  test_different_string();
  test_same_string();
  return 0;
}