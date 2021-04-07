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

  uint32_t ReadItem(const size_t i, const size_t j) { return buckets[i][j]; }

  uint32_t WriteItem(const size_t i, const size_t j, const uint32_t item) {
    buckets[i][j] = item;
  }

  bool DeleteItemFromBucket(const size_t i, const uint32_t item) {
    for (size_t j = 0; j < k_items_per_bucket; j++) {
      if (ReadItem(i, j) == item) {
        WriteItem(i, j, 0);

        return true;
      }
    }

    return false;
  }

  bool InsertItemToBucket(const size_t i, const uint32_t item,
                          const bool kickout, uint32_t &old_item) {
    for (size_t j = 0; j < k_tags_per_bucket; j++) {
      if (ReadTag(i, j) == 0) {
        WriteTag(i, j, item);

        return true;
      }
    }

    if (kickout) {
      size_t r = rand() % k_items_per_bucket;
      old_item = ReadTag(i, r);
      WriteTag(i, r, item);
    }

    return false;
  }
};

};  // namespace cuckoofilter