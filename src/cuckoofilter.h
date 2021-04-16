#include <memory>
#include <string>
#include <vector>

#include "hash.h"
#include "table.h"

namespace cuckoofilterbio1 {
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

  class Victim {
   public:
    uint32_t index;
    uint32_t fingerprint;
    bool used;
  };
  // U konstruktoru napravi victim.false;
  Victim victim;

  hash_used hasher;
  static const uint32_t item_mask = (1ULL << bits_per_item) - 1;

  // PAZI OVDJE RADI SAMO ZA STRING ZA SAD..
  uint32_t GenerateFingerprint(item_type& item) {
    return hasher(item) & item_mask;
  }

  uint32_t GetIndex1(item_type& item) {
    // BUCKETCOUNT MI NE PREPOZNAJE
    return hasher(item) % (table->BucketCount());
  }

  uint32_t GetIndex2(const uint32_t& index1, const uint32_t& fingerprint) {
    return (index1 ^ hasher(fingerprint)) % (table->BucketCount());
  }

  Status Add(const item_type& item) {
    if (victim.used) return NotEnoughSpace;

    uint32_t index = GetIndex1(item);
    uint32_t fingerprint = GenerateFingerprint(item);

    return AddImpl(index, fingerprint);
  }

  // Ako je usao u AddImpl, ubacit se item sigurno, ako nista na victimu ce
  // netko biti
  Status AddImpl(const uint32_t index, const uint32_t fingerprint) {
    uint32_t current_index = index;
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
    // Koristim victima.. tablica je puna
    victim.used = true;
    victim.index = current_index;
    victim.fingerprint = current_fingerprint;
    num_items++;

    return Ok;
  }

  Status Contain(const item_type& item) {
    bool found = false;
    uint32_t fingerprint = GenerateFingerprint(item);
    uint32_t index1 = GetIndex1(item);
    uint32_t index2 = GetIndex2(index1, fingerprint);

    found = (victim.used && victim.fingerprint == fingerprint &&
             (index1 == victim.index || index2 = victim.index));
    if (found || table->FindFingerprintInBuckets(index1, index2, fingerprint))
      return Ok;
    else
      return NotFound;
  }

  Status Delete(const item_type& item) {
    uint32_t fingerprint = GenerateFingerprint(item);
    uint32_t index1 = GetIndex1(item);
    uint32_t index2 = GetIndex2(index1, fingerprint);

    if (table->deleteItemFromBucket(index1, fingerprint)) {
      num_items--;
      if (victim.used) {
        victim.used = false;
        AddImpl(victim.index, victim.fingerprint);
        return Ok;
      }
    } else if (table->deleteItemFromBucket(index2, fingerprint)) {
      num_items--;
      if (victim.used) {
        victim.used = false;
        AddImpl(victim.index, victim.fingerprint);
        return Ok;
      }
    } else if (victim.used && fingerprint == victim.fingerprint &&
               (index1 == victim.index || index2 == victim.index)) {
      num_items--;
      victim.used = false;
      return Ok;
    }
    return NotFound;
  }

  size_t Size() const { return num_items; }

  size_t SizeInBytes() const { return table->SizeInBytes(); }

  double LoadFactor() const { return 1.0 * Size() / table->SizeTable(); }

  double BitsPerItem() const { return 8.0 * table->SizeInBytes() / Size(); }

  size_t GetBucketCount() const { return table->BucketCount(); }

  vector<uint32_t> getBucketFromTable(const uint32_t i) {
    return table->getBucket(i);
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