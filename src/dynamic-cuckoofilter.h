#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "cuckoofilter.h"
//#include "hash.h"
//#include "table.h"

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

  class DynamicCuckooFilterNode {
    std::shared_ptr<class TypedCuckooFilter> cf;
    std::shared_ptr<DynamicCuckooFilterNode> next;

   public:
    DynamicCuckooFilterNode(std::shared_ptr<class TypedCuckooFilter> cf,
                            std::shared_ptr<DynamicCuckooFilterNode> next)
        : cf(cf), next(next) {}
  };

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

    Status Compact(const item_type& item) {
      std::shared_ptr<DynamicCuckooFilterNode> tmp_curr_cf = head_cf;
      std::priority_queue<class TypedCuckooFilter,
                          std::vector<class TypedCuckooFilter>,
                          DynamicCuckooFilterQueueComparator>
          dynamic_cuckoo_queue;

      // napravi CFQ
      while (tmp_curr_cf != nullptr) {
        if (tmp_curr_cf.get()->cf.LoadFactor() < capacity)
          dynamic_cuckoo_queue.push(tmp_curr_cf.get()->cf);

        tmp_curr_cf = tmp_curr_cf.get()->next;
      }

      // za svaki CF u CFQ
      for (uint32_t i = 1; i < dynamic_cuckoo_queue.size(); i++) {
        std::shared_ptr<class TypedCuckooFilter> curr_cf =
            dynamic_cuckoo_queue.c[i];

        // za svaki Bucket u CF
        size_t bucket_count = curr_cf->GetBucketCount();
        for (uint32_t j = 1; j < bucket_count; j++) {
          std::vector<uint32_t> bucket_at_j_from_curr_cf =
              curr_cf->GetBucketFromTable(j);

          // za svaki CF koji je iza trenutnog
          for (uint32_t k = dynamic_cuckoo_queue.size() - 1;
               bucket_at_j_from_curr_cf.size() > 0 && k > i; k--) {
            // prebaci fingerprint iz trenutnog u neki od prethodnih
            while (bucket_at_j_from_curr_cf.size() > 0 &&
                   Status.Ok == dynamic_cuckoo_queue.c[k].AddToBucket(
                                    i, bucket_at_j_from_curr_cf[0]))
              // izbrisi prebaceni fingerprint
              bucket_at_j_from_curr_cf.erase(bucket_at_j_from_curr_cf.begin());
          }

          // ako je CF sada prazan, ukloni ga
          if (curr_cf->Size() == 0) {
            tmp_curr_cf = head_cf;

            while (tmp_curr_cf->next != nullptr) {
              if (tmp_curr_cf->next->cf.Size() == 0) {
                tmp_curr_cf->next = tmp_curr_cf->next->next;

                break;
              }

              tmp_curr_cf = tmp_curr_cf->next;
            }

            break;
          }
        }
      }

      return Status.Ok;
    }
  };
};
}  // namespace cuckoofilterbio1
