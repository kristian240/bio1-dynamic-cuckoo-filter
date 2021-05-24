#include <cmath>
#include <memory>
#include <string>
#include <vector>

#include "hash.h"
#include "table.h"

namespace cuckoofilterbio1 {
// enum Status that is used to indicate the end status of CF methods
enum Status {
  Ok = 0,
  NotFound = 1,
  NotEnoughSpace = 2,
};

// class Victim is used when there is no more space in a CF and there were
// too many kickouts
class Victim {
 public:
  uint32_t index;
  uint32_t fingerprint;
  bool used;
};

// Max limit of how much kickouts can happen in an Add method
const size_t max_num_kicks = 500;

// class CuckooFilter contains a reference to one table
// unitx - size of a fingerprint; uint8_t (default), uint16_t, uint32_t
// item_type - type of a items that will be added to a CuckooFilter (std::string
// by defualt) class table_type - table class that will be used for storing
// items hash_used - class used for calculating a hash for an item (uses ()
// operator)
template <typename uintx = uint8_t, typename item_type = std::string,
          class table_type = Table<uintx>, typename hash_used = Hash>
class CuckooFilter {
  std::unique_ptr<table_type> table;
  size_t num_items;
  size_t max_items;
  size_t bits_per_item;

  Victim victim;

  hash_used hasher;
  uint32_t item_mask;

  // GenerateFingerprint will generate a fingerprint for an item
  uint32_t GenerateFingerprint(const item_type& item) {
    return hasher(item) & item_mask;
  }

  // GenerateFingerprint will calculate first index for an item
  uint32_t GetIndex1(const item_type& item) {
    return hasher(item) % (table->BucketCount());
  }

  // GenerateFingerprint will calculate second index for an item based on the
  // first index and the fingerptint
  uint32_t GetIndex2(const uint32_t& index1, const uint32_t& fingerprint) {
    std::string s = std::to_string(fingerprint);
    return (index1 ^ hasher(s)) % (table->BucketCount());
  }

 public:
  // CuckooFilter constructor takes max_items as an argument and will create an
  // empty CF
  CuckooFilter(const size_t max_items)
      : max_items(max_items), num_items(0), victim(), hasher() {
    if (std::is_same<uintx, uint8_t>::value) {
      bits_per_item = 8;
    } else if (std::is_same<uintx, uint16_t>::value) {
      bits_per_item = 16;
    } else if (std::is_same<uintx, uint32_t>::value) {
      bits_per_item = 32;
    }

    size_t k_items_per_bucket = 4;
    item_mask = (1ULL << bits_per_item) - 1;

    // zaokruzi na sljedecu potenciju broja 2
    size_t num_buckets =
        max_items < k_items_per_bucket
            ? 1
            : pow(2, ceil(log2(((double)max_items) / k_items_per_bucket)));

    victim.used = false;

    table = std::make_unique<table_type>(num_buckets);
  }

  // CuckooFilter destructor
  virtual ~CuckooFilter() = default;

  // Method Add will check if there is room to add an item and if the victim is
  // not in use. If both conditions are satisified, method will create first
  // index and a fingerprint for an item. Then, it will 100% add an item in the
  // CF.
  Status Add(const item_type& item) {
    if (num_items == max_items) return NotEnoughSpace;

    if (victim.used) return NotEnoughSpace;

    uint32_t index = GetIndex1(item);
    uint32_t fingerprint = GenerateFingerprint(item);

    return AddImpl(index, fingerprint);
  }

  // Method AddImpl will try to add an item to a bucket[index]. If there is no
  // space left, will calculate alternate index and will try to add it to
  // bucket[index2]. If there is no space left on this bucket too, kickout will
  // occure. Once kickout occures, alternate index is calculated for a kickouted
  // item and the whole adding process will be repeated.
  // In order to prevent continous loop, max_num_kicks will limit the number of
  // kickouts. Once the limit is exceeded, victim will be used. Victim will hold
  // the last kickout value and will prevent further adding to the CF. Victim
  // can become unused once one item is deleted from the CF.
  Status AddImpl(const uint32_t& index, const uint32_t& fingerprint) {
    uint32_t current_index = index;
    uint32_t current_fingerprint = fingerprint;
    uint32_t old_fingerprint;

    for (uint32_t count = 0; count < max_num_kicks; count++) {
      // If insertion is not possible on first try, there is second index that
      // can be used for insertion. So kickout must not happen on first try, and
      // can happen on any of the next insertions for this item
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
    // Koristim victima.. tablica je puna
    victim.used = true;
    victim.index = current_index;
    victim.fingerprint = current_fingerprint;
    num_items++;

    return Ok;
  }

  // Contain method will check if provided item is stored in the CF
  Status Contain(const item_type& item) {
    bool found = false;
    uint32_t fingerprint = GenerateFingerprint(item);
    uint32_t index1 = GetIndex1(item);
    uint32_t index2 = GetIndex2(index1, fingerprint);

    found = (victim.used && victim.fingerprint == fingerprint &&
             (index1 == victim.index || index2 == victim.index));
    if (found || table->FindFingerprintInBuckets(index1, index2, fingerprint))
      return Ok;
    else
      return NotFound;
  }

  // Delete method will delete an item from the CF. If vitcim was in use, it
  // will try to add it again.
  Status Delete(const item_type& item) {
    uint32_t fingerprint = GenerateFingerprint(item);
    uint32_t index1 = GetIndex1(item);
    uint32_t index2 = GetIndex2(index1, fingerprint);

    if (table->DeleteItemFromBucket(index1, fingerprint)) {
      num_items--;

      if (victim.used) {
        victim.used = false;
        AddImpl(victim.index, victim.fingerprint);
      }

      return Ok;
    } else if (table->DeleteItemFromBucket(index2, fingerprint)) {
      num_items--;

      if (victim.used) {
        victim.used = false;
        AddImpl(victim.index, victim.fingerprint);
      }

      return Ok;
    } else if (victim.used && fingerprint == victim.fingerprint &&
               (index1 == victim.index || index2 == victim.index)) {
      num_items--;
      victim.used = false;
      return Ok;
    }
    return NotFound;
  }

  // Size returns number of items stored in the CF
  size_t Size() const { return num_items; }

  // SizeInBytes returns number of bytes stored in the CF
  size_t SizeInBytes() const { return table->SizeInBytes(); }

  // LoadFactor returns load factor of the CF
  double LoadFactor() const { return 1.0 * Size() / max_items; }

  // BitsPerItem returns bits per item
  double BitsPerItem() const { return 8.0 * table->SizeInBytes() / Size(); }

  // GetBucketCount returns bucket count
  size_t GetBucketCount() const { return table->BucketCount(); }

  // GetBucketCount returns a bucket at index i from the table
  vector<uint32_t> GetBucketFromTable(const uint32_t& i) {
    return table->GetBucket(i);
  }

  // DeleteItemFromBucketDirect deletes an item from a bucket at index i from
  // the table
  Status DeleteItemFromBucketDirect(const uint32_t& i,
                                    const uint32_t& fingerprint) {
    if (table->DeleteItemFromBucket(i, fingerprint)) {
      num_items--;
      return Ok;
    }
    return NotFound;
  }

  // AddToBucket adds an item ta a bucket at index i
  Status AddToBucket(const uint32_t& i, const uint32_t& item) {
    bool kickout = false;
    uint32_t old_fingerprint = 0;
    if (table->InsertItemToBucket(i, item, kickout, old_fingerprint)) {
      num_items++;
      return Ok;
    }
    return NotEnoughSpace;
  }

  // GetVictim returns an victim
  std::shared_ptr<Victim> GetVictim() {
    return std::make_shared<Victim>(victim);
  }

  Status DeleteVictim(const uint32_t& i, const uint32_t& fingerprint) {
    if (victim.used && victim.fingerprint == fingerprint && victim.index == i) {
      num_items--;
      victim.used = false;
      return Ok;
    }
    return NotFound;
  }

  string Info() {
    std::stringstream ss;
    ss << "CuckooFilter Status:\n"
       << "\t\t" << table->Info() << "\n"
       << "\t\tKeys stored: " << Size() << "\n"
       << "\t\tLoad factor: " << LoadFactor() << "\n"
       << "\t\tHashtable size: " << (table->SizeInBytes() >> 10) << " KB\n";
    if (Size() > 0) {
      ss << "\t\tbit/key:   " << BitsPerItem() << "\n";
    } else {
      ss << "\t\tbit/key:   N/A\n";
    }
    return ss.str();
  }
};

}  // namespace cuckoofilterbio1