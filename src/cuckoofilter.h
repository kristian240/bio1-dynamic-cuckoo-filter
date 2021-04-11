
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
  // Mjesto za pohranu
  shared_ptr<table_type<bits_per_item>> table;

  // Broj spremljenih
  size_t num_items;

  class Victim {
   public:
    size_t index;
    size_t fingerprint;
    bool used;
  }

  // Koristi se kao rezervna pohrana
  Victim victim;

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

 public:
  explicit CuckooFilter(const size_t max_items)
      : num_items(0), victim_(), hasher_() {
    size_t assoc = 4;
    // zaokruzi na sljedecu potenciju broja 2
    size_t num_buckets = pow(2, ceil(log(max_num_keys / assoc)) / log(2));

    // victim se ne koristi
    victim_.used = false;

    table = new TableType<bits_per_item>(num_buckets);
  }

  ~CuckooFilter() { delete table; }

  Status Add(const item_type& item) {
    if (victim.used) return NotEnoughSpace;

    size_t index = GetIndex1(item);
    uint32_t fingerprint = GenerateFingerprint(item);

    return AddImpl(index, fingerprint);
  }

  // Ako je usao u AddImpl, ubacit se,
  // ako se ne ubaci, sprema se kao victim
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

    // Koristim victima.. tablica je puna
    victim.used = true;
    victim.index = current_index victim.fingerprint = current_fingerprint;
    num_items++;

    return Ok;
  }

  Status Contains(const item_type& item) {
    bool found = false;
    size_t fingerprint = GenerateFingerprint(item);
    size_t index1 = GetIndex1(item);
    size_t index2 = GetIndex2(index1, fingerprint);

      found = (victim.used && victim.fingerprint==fingerprint && (index1==victim.index || index2=victim.index);
      if (found || table-> FindFingerprintInBuckets(index1,index2,fingerprint)) return Ok;
      else return NotFound;
  }

  Status Delete(const item_type& item) {
    size_t fingerprint = GenerateFingerprint(item);
    size_t index1 = GetIndex1(item);
    size_t index2 = GetIndex2(index1, fingerprint);

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

}  // namespace cuckoofilter