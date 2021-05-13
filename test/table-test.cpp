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
// Za sad je bits_per_item 32 to treba kasnije promijeniti
// i testirati sa razlicitim bitovima

/*for (int i = 0; i < 10; ++i) {
    for (int j = 0; j < 4; ++j) {
      std::cout << table.ReadItem(i, j) << " ";
    }
    std::cout << "\n";
  }*/

void test_construct_table() {
  cuckoofilterbio1::Table<32> table(10);
  for (int i = 0; i < 10; ++i) {
    for (int j = 0; j < 4; ++j) {
      assert(table.ReadItem(i, j) == 0);
    }
  }
  assert(table.BucketCount() == 10);
  assert(table.SizeTable() == 4 * 10);
  std::cout << "PASS test_construct_table" << std::endl;
}
void test_add_items_table() {
  cuckoofilterbio1::Table<32> table(10);

  for (int i = 0; i < 10; ++i) {
    for (int j = 0; j < 4; ++j) {
      table.WriteItem(i, j, i + j);
    }
  }

  for (int i = 0; i < 10; ++i) {
    for (int j = 0; j < 4; ++j) {
      assert(table.ReadItem(i, j) == i + j);
    }
  }
  std::cout << "PASS test_add_items_table" << std::endl;
}
void test_get_bucket_table() {
  cuckoofilterbio1::Table<32> table(10);

  for (int i = 0; i < 10; ++i) {
    for (int j = 0; j < 4; ++j) {
      table.WriteItem(i, j, i + j);
    }
  }
  vector<uint32_t> bucket = table.GetBucket(0);
  vector<uint32_t> v{1, 2, 3};
  assert(bucket == v);
  std::cout << "PASS test_get_Bucket_table" << std::endl;
}
