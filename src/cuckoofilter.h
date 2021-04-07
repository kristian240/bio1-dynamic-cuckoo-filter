

#include "table.h"
#include "hash.h"
#include <string>

namespace cuckoofilter{
const size_t max_num_kicks=500;


template<typename item_type=string, size_t bits_per_item=7,
        template <size_t> class table_type= Table,
        typename hash_used = Hash>
class CuckooFilter{
    shared_ptr<table_type<bits_per_item>> table;
    size_t num_items;

    hash_used hasher;
    static uint32_t mask=(1ULL<<bits_per_item)-1;


    //PAZI OVDJE RADI SAMO ZA STRING ZA SAD..
    size_t generateFingerprint(item_type item){
        return hasher(item) & mask;
    }

    size_t getindex1(item_type item){
        //BUCKETCOUNT MI NE PREPOZNAJE
        return hasher(item)%(table->BucketCount());
    }
    size_t getindex2(const size_t index1,cont uint32_t tag){
         return ((index1 ^ (hasher(tag))%(table->BucketCount()));
    }
    



};

}