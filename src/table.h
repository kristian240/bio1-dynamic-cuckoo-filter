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
    uint32_t operator[](const uint32_t j) { return bits[j]; }
    void write(const uint32_t j, const uint32_t fingerprint) {
      bits[j] = fingerprint;
    }
  };

  unique_ptr<Bucket[]> buckets;
  size_t bucket_count;

 public:
  Table(const size_t bucket_count) : bucket_count(bucket_count) {
    if (std::is_same<uintx, uint8_t>::value) {
      bits_per_item = 8;
    } else if (std::is_same<uintx, uint16_t>::value) {
      bits_per_item = 16;
    } else if (std::is_same<uintx, uint32_t>::value) {
      bits_per_item = 32;
    }
    k_bytes_per_bucket = (bits_per_item * k_items_per_bucket) / 8.;

    buckets = make_unique<Bucket[]>(bucket_count);
    memset(buckets.get(), 0, k_bytes_per_bucket * bucket_count);
  }

  virtual ~Table() = default;

  size_t BucketCount() const { return bucket_count; }

  size_t SizeTable() const { return k_items_per_bucket * bucket_count; }

  size_t SizeInBytes() const { return k_bytes_per_bucket * bucket_count; }

  uint32_t ReadItem(const uint32_t i, const uint32_t j) {
    return buckets[i][j];
  }

  void WriteItem(const uint32_t i, const uint32_t j,
                 const uint32_t fingerprint) {
    buckets[i].write(j, fingerprint);
  }

  vector<uint32_t> GetBucket(const uint32_t i) {
    vector<uint32_t> bucket;

    for (uint32_t j = 0; j < k_items_per_bucket; j++)
      if (buckets[i][j] != 0) bucket.push_back(buckets[i][j]);

    return bucket;
  }

  bool DeleteItemFromBucket(const uint32_t &index,
                            const uint32_t &fingerprint) {
    for (uint32_t j = 0; j < k_items_per_bucket; j++) {
      if (ReadItem(index, j) == fingerprint) {
        WriteItem(index, j, 0);

        return true;
      }
    }

    return false;
  }

  bool FindFingerprintInBuckets(const uint32_t &index1, const uint32_t &index2,
                                const uint32_t &fingerprint) {
    for (uint32_t j = 0; j < k_items_per_bucket; j++) {
      if (ReadItem(index1, j) == fingerprint ||
          ReadItem(index2, j) == fingerprint)
        return true;
    }

    return false;
  }

  bool InsertItemToBucket(const uint32_t &index, const uint32_t &fingerprint,
                          const bool &kickout, uint32_t &old_fingerprint) {
    for (uint32_t j = 0; j < k_items_per_bucket; j++) {
      if (ReadItem(index, j) == 0) {
        WriteItem(index, j, fingerprint);

        return true;
      }
    }

    if (kickout) {
      uint32_t r = rand() % k_items_per_bucket;
      old_fingerprint = ReadItem(index, r);
      WriteItem(index, r, fingerprint);
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