#include <math.h>
#include <stdio.h>
// #include <string.h>

using namespace std;

namespace cuckoofilter {

template <size_t bits_per_item>
class Table {
  static const size_t k_items_per_bucket = 4;
  static const size_t k_bytes_per_bucket =
      ceil((bits_per_tag * k_items_per_bucket) / 8.);

  shared_ptr<Bucket[]> buckets;
  size_t bucket_count;

  class Bucket {
    char bits[k_bytes_per_bucket];
  };

 public:
  SingleTable(const size_t bucket_count) : bucket_count(bucket_count) {
    buckets = make_shared<Bucket[]>(k_bytes_per_bucket * bucket_count);
    memset(buckets, 0, k_bytes_per_bucket * bucket_count);
  }

  virtual ~SingleTable() = default;

  size_t BucketCount() const { return bucket_count; }

  size_t SizeTable() const { return k_items_per_bucket * bucket_count; }

  size_t SizeInBytes() const { return k_bytes_per_bucket * bucket_count; }

  uint32_t ReadItem(const size_t i, const size_t j) { return buckets[i][j]; }

  uint32_t WriteItem(const size_t i, const size_t j,
                     const uint32_t fingerprint) {
    buckets[i][j] = fingerprint;
  }

  bool DeleteItemFromBucket(const size_t &index, const uint32_t &fingerprint) {
    for (size_t j = 0; j < k_items_per_bucket; j++) {
      if (ReadItem(index, j) == fingerprint) {
        WriteItem(index, j, 0);

        return true;
      }
    }

    return false;
  }

  bool FindFingerprintInBuckets(const size_t &index1, const size_t &index2,
                                const size_t &fingerprint) {
    for (size_t j = 0; j < k_items_per_bucket, j++) {
      if (ReadItem(index1, j) == fingerprint ||
          ReadItem(index2, j) == fingerprint)
        return true;
    }
    return false;
  }

  bool InsertItemToBucket(const size_t &index, const uint32_t &fingerprint,
                          const bool &kickout, uint32_t &old_fingerprint) {
    for (size_t j = 0; j < k_items_per_bucket; j++) {
      if (ReadItem(index, j) == 0) {
        WriteItem(index, j, fingerprint);

        return true;
      }
    }

    if (kickout) {
      size_t r = rand() % k_items_per_bucket;
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
}  // namespace cuckoofilter