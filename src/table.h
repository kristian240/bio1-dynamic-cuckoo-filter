#include <math.h>
#include <stdio.h>

#include <iostream>
#include <memory>
#include <sstream>
#include <string>
// #include <string.h>

using namespace std;

namespace cuckoofilterbio1 {

template <size_t bits_per_item>
class Table {
  static const size_t k_items_per_bucket = 4;
  static const size_t k_bytes_per_bucket =
      ceil((bits_per_item * k_items_per_bucket) / 8.);

  class Bucket {
    char bits[k_bytes_per_bucket];
  };

  shared_ptr<Bucket[]> buckets;
  size_t bucket_count;

 public:
  Table(const size_t bucket_count) : bucket_count(bucket_count) {
    buckets = make_shared<Bucket[]>(k_bytes_per_bucket * bucket_count);
    memset(buckets.get(), 0, k_bytes_per_bucket * bucket_count);
  }

  virtual ~Table() = default;

  size_t BucketCount() const { return bucket_count; }

  size_t SizeTable() const { return k_items_per_bucket * bucket_count; }

  size_t SizeInBytes() const { return k_bytes_per_bucket * bucket_count; }

  uint32_t ReadItem(const uint32_t i, const uint32_t j) {
    return buckets[i][j];
  }

  uint32_t WriteItem(const uint32_t i, const uint32_t j,
                     const uint32_t fingerprint) {
    buckets[i][j] = fingerprint;
  }

  /* On mi samo treba za Compact()
  vector<uint32_t> getBucket(const uint32_t i) {
    vector<uint32_t> bucket;
    for (uint32_t j = 0; j < k_items_per_bucket; j++) {
      if (buckets[i][j] != 0) bucket.push_back(buckets[i][j]);
    }
    return bucket;
  } */

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
    ss << "\t\tAssociativity: " << k_items_per_bucket << "\n";
    ss << "\t\tTotal # of rows: " << bucket_count << "\n";
    ss << "\t\tTotal # slots: " << SizeTable() << "\n";
    return ss.str();
  }
};
}  // namespace cuckoofilterbio1