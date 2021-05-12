#include "../src/table.h"

#include <assert.h>

#include <iostream>

/*void test_different_string() {
  std::string s1("ACATATGTCCGTATGTACATACCTACGGACGTACATACGA");
  std::string s2("TGGTACGGTCGTATGTGCTTGAGTAAGTACGTAAGTAACT");
  cuckoofilterbio1::Hash hasher;
  assert(hasher(s1) != hasher(s2));
  std::cout << "PASS test_different_string" << std::endl;
}*/

void test_construct_table() {
  cuckoofilterbio1::Table<7> table(10);
  std::cout << table.BucketCount() << std::endl;
}