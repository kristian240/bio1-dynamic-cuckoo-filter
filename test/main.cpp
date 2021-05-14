#include <iostream>

//#include "cuckoofilter-test.cpp"
#include "hash-test.cpp"
#include "table-test.cpp"

int main(int argc, const char* argv[]) {
  // hash
  test_different_string();
  test_same_string();

  // table
  test_construct_table();
  test_add_items_table();
  test_get_bucket_table();
  test_delete_item_table();
  test_find_fingerprints_in_buckets_table();
  test_insert_item_with_kickout_table();

  // cuckoofilter

  return 0;
}
