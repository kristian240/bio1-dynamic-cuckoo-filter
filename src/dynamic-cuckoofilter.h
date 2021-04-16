#include <iostream>
#include <memory>
#include <queue>
#include <string>
#include <utility>
#include <vector>

#include "hash.h"
#include "table.h"

namespace cuckoofilterbio1 {

template <typename item_type = string, size_t bits_per_item = 7,
          template <size_t> class table_type = Table, typename hash_used = Hash>
class DynamicCuckooFilter {
  using CuckooFilter<item_type, bits_per_item, table_type, hash_used>
      TypedCuckooFilter;

  class DynamicCuckooFilterQueueComparator {
   public:
    bool operator()(const TypedCuckooFilter& lhs,
                    const TypedCuckooFilter& rhs) {
      return lhs.Size() < rhs.Size();
    }
  }

  std::priority_queue<class TypedCuckooFilter,
                      std::vector<class TypedCuckooFilter>,
                      DynamicCuckooFilterQueueComparator>
      dynamic_cuckoo_queue;

  class DynamicCuckooFilterNode {
    std::shared_ptr<class TypedCuckooFilter> cf;
    std::shared_ptr<DynamicCuckooFilterNode> next;

   public:
    DynamicCuckooFilterNode(std::shared_ptr<class TypedCuckooFilter> cf,
                            std::shared_ptr<DynamicCuckooFilterNode> next)
        : cf(cf), next(next) {}
  }

  const double capacity;
  std::shared_ptr<DynamicCuckooFilterNode> head_cf;
  std::shared_ptr<DynamicCuckooFilterNode> curr_cf;

  void AddToQueue(TypedCuckooFilter newCF) { dynamic_cuckoo_queue.push(newCF); }

 public:
  DynamicCuckooFilter(const size_t max_items)
      : head_cf(std::make_shared<DynamicCuckooFilterNode>(
            new TypedCuckooFilter(max_items), nullptr)),
        capacity(0.9) {
    AddToQueue(head_cf->cf.get());
    curr_cf = head_cf;
  }

  Status Add(const item_type& item) {
    if (curr_cf->cf.LoadFactor() > capacity) {
      if (curr_cf->next == nullptr) {
        curr_cf->next = std::make_shared<DynamicCuckooFilterNode>(
            new TypedCuckooFilter(max_items), nullptr);
        AddToQueue(curr_cf->next->cf.get());
      }

      curr_cf = curr_cf->next;
    }

    Status add_status = curr_cf->cf.Add(item);

    std::shared_ptr<DynamicCuckooFilterNode> tmp_curr_cf = curr_cf;

    std::shared_ptr<Victim> victim = tmp_curr_cf->cf.GetVictim();
    while (victim->used == true) {
      victim->used == false;

      if (tmp_curr_cf->next == nullptr) {
        tmp_curr_cf->next = std::make_shared<DynamicCuckooFilterNode>(
            new TypedCuckooFilter(max_items), nullptr);
        AddToQueue(tmp_curr_cf->next->cf.get());
      }

      std::shared_ptr<DynamicCuckooFilterNode> next_cf = tmp_curr_cf->next;

      next_cf->cf.AddImpl(victim->index, victim->fingerprint);

      tmp_curr_cf = tmp_curr_cf->next;
      victim = tmp_curr_cf->cf.GetVictim();
    }

    return add_status;
  }

  Status Contains(const item_type& item) {
    std::shared_ptr<DynamicCuckooFilterNode> tmp_curr_cf = head_cf;

    while (tmp_curr_cf != nullptr) {
      if (tmp_curr_cf->cf.Contains(item) == Status.Ok) {
        return Status.Ok;
      }

      tmp_curr_cf = tmp_curr_cf->next;
    }

    return Status.NotFound;
  }

  Status Delete(const item_type& item) {
    std::shared_ptr<DynamicCuckooFilterNode> tmp_curr_cf = head_cf;

    while (tmp_curr_cf != nullptr) {
      if (tmp_curr_cf->cf.Delete(item) == Status.Ok) {
        return Status.Ok;
      }

      tmp_curr_cf = tmp_curr_cf->next;
    }

    return Status.NotFound;
  }

  Status Delete(const item_type& item) {
    std::shared_ptr<DynamicCuckooFilterNode> tmp_curr_cf = head_cf;

    while (tmp_curr_cf != nullptr) {
      if (tmp_curr_cf->cf.Delete(item) == Status.Ok) {
        return Status.Ok;
      }

      tmp_curr_cf = tmp_curr_cf->next;
    }

    return Status.NotFound;
  }
};
}  // namespace cuckoofilterbio1

/*
Algorithm 4 Compact ( )
1: for k = 1 to s do
2: if CFk is not full then
3: add CFk to CFQ;
4: sort CFQ by ascending order;
5: for i = 2 to m do // m is the number of CFs in CFQ
6: curCF ← CF Q.element[i − 1];
7: for j = 1 to l do
8: if bucket curCF.B(j) is not empty then
9: for k = m to i do
10: CF Q.element[k].B(j) ← fingerprints of
curCF.B(j);
11: if curCF is empty then
12: remove curCF from DCF;
13: break;
14: return true.
*/