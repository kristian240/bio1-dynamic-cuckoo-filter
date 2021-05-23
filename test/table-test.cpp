#include "../src/table.h"

#include <assert.h>

#include <iostream>
#include <memory>

using namespace cuckoofilterbio1;

/*for (int i = 0; i < 10; ++i) {
    for (int j = 0; j < 4; ++j) {
      std::cout << table.ReadItem(i, j) << " ";
    }
    std::cout << "\n";
  }*/

void test_construct_table() {
  std::unique_ptr<Table<>> table = std::make_unique<Table<>>(10);
  for (int i = 0; i < 10; ++i) {
    for (int j = 0; j < 4; ++j) {
      assert(table->ReadItem(i, j) == 0);
    }
  }
  assert(table->BucketCount() == 10);
  assert(table->SizeTable() == 4 * 10);
  std::cout << "PASS test_construct_table" << std::endl;
}
void test_add_items_table() {
  std::unique_ptr<Table<>> table = std::make_unique<Table<>>(10);

  for (int i = 0; i < 10; ++i) {
    for (int j = 0; j < 4; ++j) {
      table->WriteItem(i, j, i + j);
    }
  }

  for (int i = 0; i < 10; ++i) {
    for (int j = 0; j < 4; ++j) {
      assert(table->ReadItem(i, j) == i + j);
    }
  }
  std::cout << "PASS test_add_items_table" << std::endl;
}
void test_get_bucket_table() {
  std::unique_ptr<Table<>> table = std::make_unique<Table<>>(10);

  for (int i = 0; i < 10; ++i) {
    for (int j = 0; j < 4; ++j) {
      table->WriteItem(i, j, i + j);
    }
  }
  vector<uint32_t> bucket = table->GetBucket(0);
  vector<uint32_t> v{1, 2, 3};
  assert(bucket == v);
  std::cout << "PASS test_get_Bucket_table" << std::endl;
}
void test_delete_item_table() {
  std::unique_ptr<Table<>> table = std::make_unique<Table<>>(10);

  for (int i = 0; i < 10; ++i) {
    for (int j = 0; j < 4; ++j) {
      table->WriteItem(i, j, i + j);
    }
  }
  for (int i = 0; i < 10; ++i) {
    for (int j = 0; j < 4; ++j) {
      table->DeleteItemFromBucket(i, i + j);
    }
  }

  for (int i = 0; i < 10; ++i) {
    for (int j = 0; j < 4; ++j) {
      assert(table->ReadItem(i, j) == 0);
    }
  }
  std::cout << "PASS test_delete_item_table" << std::endl;
}
void test_find_fingerprints_in_buckets_table() {
  std::unique_ptr<Table<>> table = std::make_unique<Table<>>(10);

  for (int i = 0; i < 10; ++i) {
    for (int j = 0; j < 4; ++j) {
      table->WriteItem(i, j, i + j);
    }
  }
  assert(table->FindFingerprintInBuckets(0, 1, 4));
  assert(!table->FindFingerprintInBuckets(0, 1, 6));
  assert(table->FindFingerprintInBuckets(0, 7, 10));
  assert(table->FindFingerprintInBuckets(2, 6, 5));
  std::cout << "PASS test_find_fingerprints_in_buckets_table" << std::endl;
}
void test_insert_item_with_kickout_table() {
  std::unique_ptr<Table<>> table = std::make_unique<Table<>>(10);

  for (int i = 0; i < 10; ++i) {
    for (int j = 0; j < 4; ++j) {
      table->WriteItem(i, j, i + j);
    }
  }
  uint32_t old_fingerprint;
  assert(table->InsertItemToBucket(0, 99, false, old_fingerprint));

  assert(!table->InsertItemToBucket(7, 77, false, old_fingerprint));
  assert(!table->FindFingerprintInBuckets(0, 7, 77));

  assert(!table->InsertItemToBucket(5, 55, true, old_fingerprint));
  assert(table->FindFingerprintInBuckets(0, 5, 55));

  std::cout << "PASS test_insert_item_with_kickout_table" << std::endl;
}

int main(int argc, const char* argv[]) {
  test_construct_table();
  test_add_items_table();
  test_get_bucket_table();
  test_delete_item_table();
  test_find_fingerprints_in_buckets_table();
  test_insert_item_with_kickout_table();

  return 0;
}