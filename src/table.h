#include <math.h>
#include <stdio.h>

#include <cstring>
#include <iostream>
#include <memory>
#include <new>
#include <sstream>
#include <vector>

using namespace std;

namespace cuckoofilterbio1 {
template <class uintx = uint8_t>
// Class Table is used for storing data into buckets.
class Table {
  static const size_t k_items_per_bucket = 4;
  size_t bits_per_item;
  size_t k_bytes_per_bucket;

  /*template <typename T>
  struct array_deleter {
    void operator()(T const *p) { delete[] p; }
  };*/

  class Bucket {
    uintx bits[k_items_per_bucket];

   public:
    uint32_t operator[](const uint32_t &j) { return bits[j]; }
    void write(const uint32_t &j, const uint32_t &fingerprint) {
      bits[j] = fingerprint;
    }
  };

  std::unique_ptr<Bucket[]> buckets;
  size_t bucket_count;

 public:
  // Table constructor takes bucket_count as a parameter and will create an
  // empty array of buckets
  Table(const size_t bucket_count) : bucket_count(bucket_count) {
    if (std::is_same<uintx, uint8_t>::value) {
      bits_per_item = 8;
    } else if (std::is_same<uintx, uint16_t>::value) {
      bits_per_item = 16;
    } else if (std::is_same<uintx, uint32_t>::value) {
      bits_per_item = 32;
    }
    k_bytes_per_bucket = (bits_per_item * k_items_per_bucket) / 8.;

    buckets = std::make_unique<Bucket[]>(bucket_count);
    memset(buckets.get(), 0, k_bytes_per_bucket * bucket_count);
  }

  // Table destructor
  virtual ~Table() = default;

  // BucketCount returns number of bucket in a Table
  size_t BucketCount() const { return bucket_count; }

  // SizeTable returns total size of a Table (used and unused)
  size_t SizeTable() const { return k_items_per_bucket * bucket_count; }

  // SizeTable returns total size of a Table in bytes (used and unused)
  size_t SizeInBytes() const { return k_bytes_per_bucket * bucket_count; }

  // ReadItem returns an item at bucket i and column j
  uint32_t ReadItem(const uint32_t &i, const uint32_t &j) {
    return buckets[i][j];
  }

  // WriteItem writes an item (fingerprint) at bucket i and column j
  void WriteItem(const uint32_t &i, const uint32_t &j,
                 const uint32_t &fingerprint) {
    buckets[i].write(j, fingerprint);
  }

  // GetBucket returns all items from bucket i
  vector<uint32_t> GetBucket(const uint32_t &i) {
    vector<uint32_t> bucket;

    for (uint32_t j = 0; j < k_items_per_bucket; j++)
      if (buckets[i][j] != 0) bucket.push_back(buckets[i][j]);

    return bucket;
  }

  // DeleteItemFromBucket deletes an item (fingerprint) from bucket i
  bool DeleteItemFromBucket(const uint32_t &i, const uint32_t &fingerprint) {
    for (uint32_t j = 0; j < k_items_per_bucket; j++) {
      if (ReadItem(i, j) == fingerprint) {
        WriteItem(i, j, 0);

        return true;
      }
    }

    return false;
  }

  // FindFingerprintInBuckets returns a true if item is found in bucket i1 or
  // i2, false otherwise
  bool FindFingerprintInBuckets(const uint32_t &i1, const uint32_t &i2,
                                const uint32_t &fingerprint) {
    for (uint32_t j = 0; j < k_items_per_bucket; j++) {
      if (ReadItem(i1, j) == fingerprint || ReadItem(i2, j) == fingerprint)
        return true;
    }

    return false;
  }

  // InsertItemToBucket inserts an item (fingerprint) to a bucket i. If
  // insertion was successful, returns true. If insertions was unsuccessful,
  // returns false and if kickout is true will make a kickout of a random item
  // from bucket i.
  bool InsertItemToBucket(const uint32_t &i, const uint32_t &fingerprint,
                          const bool &kickout, uint32_t &old_fingerprint) {
    for (uint32_t j = 0; j < k_items_per_bucket; j++) {
      if (ReadItem(i, j) == 0) {
        WriteItem(i, j, fingerprint);

        return true;
      }
    }

    if (kickout) {
      uint32_t r = rand() % k_items_per_bucket;
      old_fingerprint = ReadItem(i, r);
      WriteItem(i, r, fingerprint);
    }

    return false;
  }

  std::string Info() const {
    std::stringstream ss;
    ss << "SingleHashtable with fingerprint size: " << bits_per_item
       << " bits \n";
    ss << "\t\tItems per bucket: " << k_items_per_bucket << "\n";
    ss << "\t\tTotal # of rows: " << bucket_count << "\n";
    ss << "\t\tTotal # slots: " << SizeTable() << "\n";
    return ss.str();
  }
};
}  // namespace cuckoofilterbio1