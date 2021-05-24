#include <algorithm>
#include <iostream>
#include <memory>
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
  /* class DynamicCuckooFilterComparator {
    public:
     bool operator()(std::shared_ptr<TypedCuckooFilter> lhs,
                     std::shared_ptr<TypedCuckooFilter> rhs) {
       return lhs->Size() < rhs->Size();
     }
   }; */

  static auto DynamicCuckooFilterComparator =
      [](std::shared_ptr<TypedCuckooFilter> lhs,
         std::shared_ptr<TypedCuckooFilter> rhs) {
        return lhs->Size() < rhs->Size();
      };

  class DynamicCuckooFilterNode {
    std::shared_ptr<TypedCuckooFilter> cf;
    std::shared_ptr<DynamicCuckooFilterNode> next;

   public:
    DynamicCuckooFilterNode(std::shared_ptr<TypedCuckooFilter> cf,
                            std::shared_ptr<DynamicCuckooFilterNode> next)
        : cf(cf), next(next) {}
    ~DynamicCuckooFilterNode() {
      std::cout << "Obrisao se jedan cf!" << std::endl;
    }
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

  // Ne moze se nikad desiti da mi vrati notEnoughSpace
  Status Add(const item_type& item) {
    if (curr_cf_node->cf->LoadFactor() > capacity) {
      if (curr_cf_node->next == nullptr) {
        curr_cf_node->next = std::make_shared<DynamicCuckooFilterNode>(
            std::make_shared<TypedCuckooFilter>(max_items), nullptr);
        ++counter_CF;
      }
      curr_cf_node = curr_cf_node->next;
    }
    // Dodao ili ne dodao trebamo viditi jel ima victima
    Status add_status = curr_cf_node->cf->Add(item);

    std::shared_ptr<DynamicCuckooFilterNode> tmp_curr_cf_node = curr_cf_node;
    std::shared_ptr<Victim> victim = tmp_curr_cf_node->cf->GetVictim();
    if (victim->used == false && add_status == Ok) {
      return Ok;
    }

    while (victim->used == true) {
      victim->used == false;

      if (tmp_curr_cf_node->next == nullptr) {
        tmp_curr_cf_node->next = std::make_shared<DynamicCuckooFilterNode>(
            std::make_shared<TypedCuckooFilter>(max_items), nullptr);
        ++counter_CF;
      }
      tmp_curr_cf_node = tmp_curr_cf_node->next;
      add_status = tmp_curr_cf_node->cf->Add(victim->fingerprint);
      victim = tmp_curr_cf_node->cf->GetVictim();
    }
    // Mora biti OK
    return add_status;
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
    std::vector<std::shared_ptr<TypedCuckooFilter>> dynamic_cuckoo_queue;

    // napravi CFQ
    while (tmp_curr_cf_node != nullptr) {
      if (tmp_curr_cf_node->cf->LoadFactor() < capacity)
        dynamic_cuckoo_queue.push_back(tmp_curr_cf_node->cf);

      tmp_curr_cf_node = tmp_curr_cf_node->next;
    }

    std::sort(dynamic_cuckoo_queue.begin(), dynamic_cuckoo_queue.end(),
              DynamicCuckooFilterComparator);

    // za svaki CF u CFQ
    for (uint32_t i = 1; i < dynamic_cuckoo_queue.size(); i++) {
      std::shared_ptr<TypedCuckooFilter> tmp_cf = dynamic_cuckoo_queue[i - 1];

      // za svaki Bucket u CF
      size_t bucket_count = tmp_cf->GetBucketCount();
      for (uint32_t j = 0; j < bucket_count; j++) {
        std::vector<uint32_t> bucket_at_j_from_tmp_cf =
            tmp_cf->GetBucketFromTable(j);

        // za svaki CF koji je iza trenutnog
        for (uint32_t k = dynamic_cuckoo_queue.size() - 1;
             bucket_at_j_from_tmp_cf.size() > 0 && k > i; k--) {
          // prebaci fingerprint iz trenutnog u neki od prethodnih
          // Ako u bucketu drugome nema vise mjesta, nece biti vise mjesta ni za
          // drugoga
          while (bucket_at_j_from_tmp_cf.size() > 0 &&
                 Ok == dynamic_cuckoo_queue[k]->AddToBucket(
                           i, bucket_at_j_from_tmp_cf[0])) {
            // izbrisi prebaceni fingerprint
            tmp_cf->DeleteItemFromBucketDirect(j, bucket_at_j_from_tmp_cf[0]);
            bucket_at_j_from_tmp_cf.erase(bucket_at_j_from_tmp_cf.begin());
          }
        }

        // ako je CF sada prazan, ukloni ga
        if (tmp_cf->Size() == 0) {
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
    // Restart curr_cf_node
    curr_cf_node = head_cf_node;
    while (curr_cf_node->cf->LoadFactor() > capacity) {
      curr_cf_node = curr_cf_node->next;
    }

    return Ok;
  }
};
}  // namespace cuckoofilterbio1
