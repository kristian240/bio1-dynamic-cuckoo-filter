#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "hash.h"
#include "table.h"

namespace cuckoofilterbio1 {

template <typename item_type = string, size_t bits_per_item = 7,
          template <size_t> class table_type = Table, typename hash_used = Hash>
class DynamicCuckooFilter {
  using CuckooFilter<item_type, bits_per_item, table_type, hash_used> =
      TypedCuckooFilter;

  class DynamicCuckooFilterNode {
    std::shared_ptr<class TypedCuckooFilter> cf;
    std::shared_ptr<DynamicCuckooFilterNode> next;

   public:
    DynamicCuckooFilterNode(std::shared_ptr<class TypedCuckooFilter> cf,
                            std::shared_ptr<DynamicCuckooFilterNode> next)
        : cf(cf), next(next) {}
  }

  // U konstruktoru je namijesten da bude 0.9
  const double capacity;
  int counter_CF;
  std::shared_ptr<DynamicCuckooFilterNode> head_cf;
  std::shared_ptr<DynamicCuckooFilterNode> curr_cf;

 public:
  DynamicCuckooFilter(const size_t max_items)
      : head_cf(std::make_shared<DynamicCuckooFilterNode>(
            new TypedCuckooFilter(max_items), nullptr)),
        capacity(0.9),
        counter_CF(1) {
    curr_cf = head_cf;
  }
  virtual ~DynamicCuckooFilter() = default;

  Status Add(const item_type& item) {
    if (curr_cf->cf.LoadFactor() > capacity) {
      if (curr_cf->next == nullptr) {
        curr_cf->next = std::make_shared<DynamicCuckooFilterNode>(
            new TypedCuckooFilter(max_items), nullptr);
        ++counter_CF;
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
        ++counter_CF;
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

  /*  void Compact() {
      vector<TypedCuckooFilter&> CFvector;
      std::shared_ptr<DynamicCuckooFilterNode> tmp_curr_cf = head_cf;
      // For petlja ne bi smjela doci do nullptr
      for (int i = 0; i < counter_CF; ++i) {
        if (tmp_curr_cf->cf.LoadFactor() < capacity) {
          // Get ide zato sto zelimo sami cf
          CFvector.push_back(tmp_curr_cf->cf.get());
        }
        // TODO SORT ZA VECTOR
        tmp_curr_cf = tmp_curr_cf->next;
      }
      size_t sizeCFQ = CFvector.size();
      for (int i = 0; i < sizeCFQ; ++i) {
        TypedCuckooFilter& tmp_cf = CFvector.at(i);
        size_t buckets = tmp_cf.GetBucketCount();
        for (size_t j = 0; j < buckets; ++j) {
          vector<uint32_t> bucket = tmp_cf.getBucketFromTable(j);
          if (!bucket.empty()){
            for (size_t k=sizeCFQ-1;k>i;--k){
              TypedCuckooFilter& tmp_cf1 = CFvector.at(k);
               vector<uint32_t> bucket1 = tmp_cf1.getBucketFromTable(j);
            }
          }
          if (tmp_cf.Size()==0){
            break;
          }
        }
      }

      // TODO Napravi da svi empty se izbrisu
    }*/
};
}  // namespace cuckoofilterbio1
