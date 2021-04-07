

#include <string>

#include "hash.h"
#include "table.h"

namespace cuckoofilter {
enum Status {
  Ok = 0,
  NotFound = 1,
  NotEnoughSpace = 2,
};

const size_t max_num_kicks = 500;

template <typename item_type = string, size_t bits_per_item = 7,
          template <size_t> class table_type = Table, typename hash_used = Hash>
class CuckooFilter {
  shared_ptr<table_type<bits_per_item>> table;
  size_t num_items;

  hash_used hasher;
  static uint32_t mask = (1ULL << bits_per_item) - 1;

  // PAZI OVDJE RADI SAMO ZA STRING ZA SAD..
  size_t GenerateFingerprint(item_type& item) { return hasher(item) & mask; }

  size_t GetIndex1(item_type& item) {
    // BUCKETCOUNT MI NE PREPOZNAJE
    return hasher(item) % (table->BucketCount());
  }

  size_t GetIndex2(const size_t& index1, const uint32_t& fingerprint) {
    return (index1 ^ hasher(fingerprint)) % (table->BucketCount());
  }

  Status Add(const item_type& item) {
    size_t index = GetIndex1(item);
    uint32_t fingerprint = GenerateFingerprint(item);

    return AddImpl(index, fingerprint);
  }

  Status AddImpl(const size_t index, const uint32_t fingerprint) {
    size_t current_index = index;
    uint32_t current_fingerprint = fingerprint;
    uint32_t old_fingerprint;

    for (uint32_t count = 0; count < max_num_kicks; count++) {
      bool kickout = count > 0;

      old_fingerprint = 0;

      if (table->InsertItemToBucket(current_index, current_fingerprint, kickout,
                                    old_fingerprint)) {
        num_items++;

        return Ok;
      }

      if (kickout) current_fingerprint = old_fingerprint;

      current_index = GetIndex2(current_index, current_fingerprint);
    }

    return NotEnoughSpace;
  }
};

}  // namespace cuckoofilter