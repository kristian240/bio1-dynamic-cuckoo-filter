#include <iostream>
#include <memory>
#include <queue>
#include <string>
#include <utility>
#include <vector>

#include "cuckoofilter.h"
//#include "hash.h"
//#include "table.h"

namespace cuckoofilterbio1 {

template <typename uintx = uint8_t, typename item_type = std::string,
          class table_type = Table<uintx>, typename hash_used = Hash>

class DynamicCuckooFilter {
  using TypedCuckooFilter =
      CuckooFilter<uintx, item_type, table_type, hash_used>;
  class DynamicCuckooFilterQueueComparator {
   public:
    bool operator()(const TypedCuckooFilter& lhs,
                    const TypedCuckooFilter& rhs) {
      return lhs.Size() < rhs.Size();
    }
  };

  class DynamicCuckooFilterNode {
    std::shared_ptr<TypedCuckooFilter> cf;
    std::shared_ptr<DynamicCuckooFilterNode> next;

   public:
    DynamicCuckooFilterNode(std::shared_ptr<TypedCuckooFilter> cf,
                            std::shared_ptr<DynamicCuckooFilterNode> next)
        : cf(cf), next(next) {}
  };

  // U konstruktoru je namjesten da bude 0.9
  const double capacity;
  int counter_CF;
  const size_t max_items;
  std::shared_ptr<DynamicCuckooFilterNode> head_cf_node;
  std::shared_ptr<DynamicCuckooFilterNode> curr_cf_node;

 public:
  DynamicCuckooFilter(const size_t max_items, double capacity = 0.9)
      : max_items(max_items),
        head_cf_node(std::make_shared<DynamicCuckooFilterNode>(
            std::make_shared<TypedCuckooFilter>(max_items), nullptr)),
        capacity(capacity),
        counter_CF(1) {
    curr_cf_node = head_cf_node;
  }
  virtual ~DynamicCuckooFilter() = default;

  Status Add(const item_type& item) {
    if (curr_cf_node->cf->LoadFactor() > capacity) {
      if (curr_cf_node->next == nullptr) {
        curr_cf_node->next = std::make_shared<DynamicCuckooFilterNode>(
            std::make_shared<TypedCuckooFilter>(max_items), nullptr);
        ++counter_CF;
      }

      Status add_status = curr_cf_node->cf.Add(item);

      std::shared_ptr<DynamicCuckooFilterNode> tmp_curr_cf_node = curr_cf_node;

      std::shared_ptr<Victim> victim = tmp_curr_cf_node->cf.GetVictim();
      while (victim->used == true) {
        victim->used == false;

        if (tmp_curr_cf_node->next == nullptr) {
          tmp_curr_cf_node->next = std::make_shared<DynamicCuckooFilterNode>(
              new TypedCuckooFilter(max_items), nullptr);
          ++counter_CF;
        }

        std::shared_ptr<DynamicCuckooFilterNode> next_cf_node =
            tmp_curr_cf_node->next;
      }

      return add_status;
    }
  }

  Status Contains(const item_type& item) {
    std::shared_ptr<DynamicCuckooFilterNode> tmp_curr_cf_node = head_cf_node;

    while (tmp_curr_cf_node != nullptr) {
      if (tmp_curr_cf_node->cf->Contains(item) == Ok) {
        return Ok;
      }

      tmp_curr_cf_node = tmp_curr_cf_node->next;
    }

    return NotFound;
  }

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

  Status Compact(const item_type& item) {
    std::shared_ptr<DynamicCuckooFilterNode> tmp_curr_cf_node = head_cf_node;
    std::priority_queue<TypedCuckooFilter, std::vector<TypedCuckooFilter>,
                        DynamicCuckooFilterQueueComparator>
        dynamic_cuckoo_queue;

    // napravi CFQ
    while (tmp_curr_cf_node != nullptr) {
      if (tmp_curr_cf_node.get()->cf.LoadFactor() < capacity)
        dynamic_cuckoo_queue.push(tmp_curr_cf_node.get()->cf);

      tmp_curr_cf_node = tmp_curr_cf_node.get()->next;
    }

    // za svaki CF u CFQ
    for (uint32_t i = 1; i < dynamic_cuckoo_queue.size(); i++) {
      std::shared_ptr<TypedCuckooFilter> curr_cf_node =
          dynamic_cuckoo_queue.c[i];

      // za svaki Bucket u CF
      size_t bucket_count = curr_cf_node->GetBucketCount();
      for (uint32_t j = 1; j < bucket_count; j++) {
        std::vector<uint32_t> bucket_at_j_from_curr_cf =
            curr_cf_node->GetBucketFromTable(j);

        // za svaki CF koji je iza trenutnog
        for (uint32_t k = dynamic_cuckoo_queue.size() - 1;
             bucket_at_j_from_curr_cf.size() > 0 && k > i; k--) {
          // prebaci fingerprint iz trenutnog u neki od prethodnih
          while (bucket_at_j_from_curr_cf.size() > 0 &&
                 Ok == dynamic_cuckoo_queue.c[k].AddToBucket(
                           i, bucket_at_j_from_curr_cf[0]))
            // izbrisi prebaceni fingerprint
            bucket_at_j_from_curr_cf.erase(bucket_at_j_from_curr_cf.begin());
        }

        // ako je CF sada prazan, ukloni ga
        if (curr_cf_node->Size() == 0) {
          tmp_curr_cf_node = head_cf_node;

          while (tmp_curr_cf_node->next != nullptr) {
            if (tmp_curr_cf_node->next->cf.Size() == 0) {
              tmp_curr_cf_node->next = tmp_curr_cf_node->next->next;

              break;
            }

            tmp_curr_cf_node = tmp_curr_cf_node->next;
          }

          break;
        }
      }
    }

    return Ok;
  }
};
}  // namespace cuckoofilterbio1
