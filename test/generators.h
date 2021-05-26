#include <iostream>
#include <set>

std::string generateKMer(size_t k) {
  static const char bases[] = "ACGT";

  std::string k_mer;

  k_mer.reserve(k);

  for (int i = 0; i < k; ++i) k_mer += bases[rand() % (sizeof(bases) - 1)];

  return k_mer;
}

std::set<std::string> generateKMerSet(size_t k, size_t kMerCount) {
  std::set<std::string> set;

  for (int i = 0; i < kMerCount; i++) {
    set.insert(generateKMer(k));
  }

  return set;
}
