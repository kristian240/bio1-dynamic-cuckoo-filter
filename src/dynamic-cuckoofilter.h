#include <algorithm>
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "cuckoofilter.h"

namespace cuckoofilterbio1 {

// DynamicCuckooFilter is a dynamic data structure that holds onto one or
// multiple CF instances. Once the current CF instance is filled, new one is
// added.
template <typename uintx = uint8_t, typename item_type = std::string,
          class table_type = Table<uintx>, typename hash_used = Hash>
class DynamicCuckooFilter {
  using TypedCuckooFilter =
      CuckooFilter<uintx, item_type, table_type, hash_used>;

  // DynamicCuckooFilterNode is a hepler class that is used to create a linked
  // list of CFs
  class DynamicCuckooFilterNode {
   public:
    std::shared_ptr<TypedCuckooFilter> cf;
    std::shared_ptr<DynamicCuckooFilterNode> next;

    DynamicCuckooFilterNode(std::shared_ptr<TypedCuckooFilter> cf,
                            std::shared_ptr<DynamicCuckooFilterNode> next)
        : cf(cf), next(next) {}
    virtual ~DynamicCuckooFilterNode() = default;
  };

  // Load factor threshold is used to determine if CF is full or not. Default
  // value is 0.9
  const double load_factor_threshold;
  // Counter that count number of CFs
  int counter_CF;
  // Max items that each CF can hold
  const size_t max_items;
  // Initial CF pointer
  std::shared_ptr<DynamicCuckooFilterNode> head_cf_node;
  // Current CF pointer (first CF that is not full)
  std::shared_ptr<DynamicCuckooFilterNode> curr_cf_node;

 public:
  // constructor will create inital CF and will set currCF to point at it
  DynamicCuckooFilter(const size_t max_items,
                      double load_factor_threshold = 0.9)
      : max_items(max_items),
        head_cf_node(std::make_shared<DynamicCuckooFilterNode>(
            std::make_shared<TypedCuckooFilter>(max_items), nullptr)),
        load_factor_threshold(load_factor_threshold),
        counter_CF(1) {
    curr_cf_node = head_cf_node;
  }

  // destructor
  virtual ~DynamicCuckooFilter() = default;

  // Add method will first check if there is room in the currCF for an item to
  // be added. If load factor threshold if passed, new CF will be created.
  // Next, item will be added to the currCF. After adding, we need to check if
  // there is a victim. If there is, add it and free up the victim; this can be
  // repeated until there is new victim occurring.
  // Note: This method call should always add an item to a DCF
  Status Add(const item_type& item) {
    while (curr_cf_node->cf->LoadFactor() >= load_factor_threshold) {
      if (curr_cf_node->next == nullptr) {
        curr_cf_node->next = std::make_shared<DynamicCuckooFilterNode>(
            std::make_shared<TypedCuckooFilter>(max_items), nullptr);
        ++counter_CF;
      }
      curr_cf_node = curr_cf_node->next;
    }

    Status add_status = curr_cf_node->cf->Add(item);

    std::shared_ptr<DynamicCuckooFilterNode> tmp_curr_cf_node = curr_cf_node;

    // Check if victim exists
    std::shared_ptr<Victim> victim = tmp_curr_cf_node->cf->GetVictim();

    if (victim->used == false && add_status == Ok) {
      return Ok;
    }

    while (victim->used == true) {
      // Remove victim
      tmp_curr_cf_node->cf->DeleteVictim(victim->index, victim->fingerprint);

      if (tmp_curr_cf_node->next == nullptr) {
        tmp_curr_cf_node->next = std::make_shared<DynamicCuckooFilterNode>(
            std::make_shared<TypedCuckooFilter>(max_items), nullptr);
        ++counter_CF;
      }

      tmp_curr_cf_node = tmp_curr_cf_node->next;
      add_status =
          tmp_curr_cf_node->cf->AddImpl(victim->index, victim->fingerprint);

      // Check if victim still exists
      victim = tmp_curr_cf_node->cf->GetVictim();
    }

    // Will be Status.Ok
    return add_status;
  }

  // Contains will iterate over all CF in the DCF and check if any CF contains
  // provided item. If true return Ok, NotFound otherwise
  Status Contains(const item_type& item) {
    std::shared_ptr<DynamicCuckooFilterNode> tmp_curr_cf_node = head_cf_node;

    while (tmp_curr_cf_node != nullptr) {
      if (tmp_curr_cf_node->cf->Contain(item) == Ok) {
        return Ok;
      }

      tmp_curr_cf_node = tmp_curr_cf_node->next;
    }

    return NotFound;
  }

  // Delete will iterate over all CF in the DCF and delete an item if any CF
  // contains it. If item deleted successfuly return Ok, NotFound otherwise
  Status Delete(const item_type& item) {
    std::shared_ptr<DynamicCuckooFilterNode> tmp_curr_cf_node = head_cf_node;

    while (tmp_curr_cf_node != nullptr) {
      if (tmp_curr_cf_node->cf->Delete(item) == Ok) {
        return Ok;
      }

      tmp_curr_cf_node = tmp_curr_cf_node->next;
    }

    return NotFound;
  }

  // Compact will "compress" the DQF so each not filled CF is filled as much as
  // possible.
  // Algorithm:
  // for k=1 to counter_CF do
  //  if CF[k] is not full then
  //    add CF[k] to CFQ; sort C F Q by ascending order;
  //  for i=2 to m do //m is the number of CFs in CFQ
  //    curCF ← CFQ.element[i − 1];
  //  for j = 1 to l do
  //    if bucket curCF.B(j) is not empty then
  //      for k = m to i do
  //        CF Q.element[k].B(j) ← fingerprints of
  //        curCF.B(j);
  //      if curCF is empty then
  //        remove curCF from DCF;
  //        break;
  // return true.
  Status Compact() {
    std::shared_ptr<DynamicCuckooFilterNode> tmp_curr_cf_node = head_cf_node;
    std::vector<std::shared_ptr<TypedCuckooFilter>> dynamic_cuckoo_queue;

    // create sorted CFQ
    while (tmp_curr_cf_node != nullptr) {
      if (tmp_curr_cf_node->cf->LoadFactor() < load_factor_threshold)
        dynamic_cuckoo_queue.push_back(tmp_curr_cf_node->cf);

      tmp_curr_cf_node = tmp_curr_cf_node->next;
    }

    std::sort(dynamic_cuckoo_queue.begin(), dynamic_cuckoo_queue.end(),
              [](std::shared_ptr<TypedCuckooFilter> lhs,
                 std::shared_ptr<TypedCuckooFilter> rhs) {
                return lhs->Size() < rhs->Size();
              });

    // for each CF in CFQ
    for (uint32_t i = 0; i < dynamic_cuckoo_queue.size(); i++) {
      std::shared_ptr<TypedCuckooFilter> tmp_cf = dynamic_cuckoo_queue[i];

      // for each Bucket in CF
      size_t bucket_count = tmp_cf->GetBucketCount();
      for (uint32_t j = 0; j < bucket_count; j++) {
        std::vector<uint32_t> bucket_at_j_from_tmp_cf =
            tmp_cf->GetBucketFromTable(j);

        // for each CF after i in reversed CFQ
        for (uint32_t k = dynamic_cuckoo_queue.size() - 1;
             bucket_at_j_from_tmp_cf.size() > 0 && k > i; k--) {
          // move fingerprint from one bucket to another until:
          // - there are no more items to be moved
          // - CF in which we are moving fingerpints is filled
          // - no more space in a bucket
          while (bucket_at_j_from_tmp_cf.size() > 0 &&
                 dynamic_cuckoo_queue[k]->LoadFactor() <
                     load_factor_threshold &&
                 Ok == dynamic_cuckoo_queue[k]->AddToBucket(
                           i, bucket_at_j_from_tmp_cf[0])) {
            // remove moved fingerprint
            tmp_cf->DeleteItemFromBucketDirect(j, bucket_at_j_from_tmp_cf[0]);

            bucket_at_j_from_tmp_cf.erase(bucket_at_j_from_tmp_cf.begin());
          }
        }

        // if the CF is empty, remove it from DCF
        if (tmp_cf->Size() == 0) {
          tmp_curr_cf_node = head_cf_node;

          while (tmp_curr_cf_node->next != nullptr) {
            if (tmp_curr_cf_node->next->cf->Size() == 0) {
              tmp_curr_cf_node->next = tmp_curr_cf_node->next->next;
              break;
            }

            tmp_curr_cf_node = tmp_curr_cf_node->next;
          }

          break;
        }
      }
    }

    // Restart curr_cf_node to first not filled CF in DCF
    curr_cf_node = head_cf_node;
    while (curr_cf_node->cf->LoadFactor() >= load_factor_threshold) {
      curr_cf_node = curr_cf_node->next;
    }

    return Ok;
  }
};
}  // namespace cuckoofilterbio1
