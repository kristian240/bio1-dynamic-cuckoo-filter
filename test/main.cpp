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

  // cuckoofilter

  return 0;
}
