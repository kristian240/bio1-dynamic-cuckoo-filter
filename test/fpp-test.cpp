#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <memory>
#include <set>

#include "../src/dynamic-cuckoofilter.h"

using namespace cuckoofilterbio1;
void testCuckooFilter8(std::set<std::string> &positive_set,
                       std::set<std::string> &negative_set) {
  std::unique_ptr<CuckooFilter<uint8_t>> cf =
      std::make_unique<CuckooFilter<uint8_t>>(positive_set.size());
  std::cout << "CuckooFilter<uint8_t> ";
  for (std::string item : positive_set) {
    cf->Add(item);
  }
  size_t found_count = 0;

  for (std::string item : negative_set) {
    if (cf->Contain(item) == Ok) {
      found_count++;
    }
  }
  double false_positive_rate = (found_count * 1.) / negative_set.size() * 100;
  std::cout << "False positive rate: (" << found_count << "/"
            << negative_set.size() << ") " << false_positive_rate << "%"
            << std::endl;
}

void testCuckooFilter16(std::set<std::string> &positive_set,
                        std::set<std::string> &negative_set) {
  std::unique_ptr<CuckooFilter<uint16_t>> cf =
      std::make_unique<CuckooFilter<uint16_t>>(positive_set.size());
  std::cout << "CuckooFilter<uint16_t> ";
  for (std::string item : positive_set) {
    cf->Add(item);
  }
  size_t found_count = 0;

  for (std::string item : negative_set) {
    if (cf->Contain(item) == Ok) {
      found_count++;
    }
  }
  double false_positive_rate = (found_count * 1.) / negative_set.size() * 100;
  std::cout << "False positive rate: (" << found_count << "/"
            << negative_set.size() << ") " << false_positive_rate << "%"
            << std::endl;
}

void testCuckooFilter32(std::set<std::string> &positive_set,
                        std::set<std::string> &negative_set) {
  std::unique_ptr<CuckooFilter<uint32_t>> cf =
      std::make_unique<CuckooFilter<uint32_t>>(positive_set.size());
  std::cout << "CuckooFilter<uint32_t> ";
  for (std::string item : positive_set) {
    cf->Add(item);
  }
  size_t found_count = 0;

  for (std::string item : negative_set) {
    if (cf->Contain(item) == Ok) {
      found_count++;
    }
  }

  double false_positive_rate = (found_count * 1.) / negative_set.size() * 100;
  std::cout << "False positive rate: (" << found_count << "/"
            << negative_set.size() << ") " << false_positive_rate << "%"
            << std::endl;
}
void testDynamicCuckooFilter8(std::set<std::string> &positive_set,
                              std::set<std::string> &negative_set) {
  std::unique_ptr<DynamicCuckooFilter<uint8_t>> dcf =
      std::make_unique<DynamicCuckooFilter<uint8_t>>(positive_set.size() / 4);
  std::cout << "DynamicCuckooFilter<uint8_t> ";
  for (std::string item : positive_set) {
    dcf->Add(item);
  }
  size_t found_count = 0;

  for (std::string item : negative_set) {
    if (dcf->Contains(item) == Ok) {
      found_count++;
    }
  }

  double false_positive_rate = (found_count * 1.) / negative_set.size() * 100;
  std::cout << "False positive rate: (" << found_count << "/"
            << negative_set.size() << ") " << false_positive_rate << "%"
            << std::endl;
}
void testDynamicCuckooFilter16(std::set<std::string> &positive_set,
                               std::set<std::string> &negative_set) {
  std::unique_ptr<DynamicCuckooFilter<uint16_t>> dcf =
      std::make_unique<DynamicCuckooFilter<uint16_t>>(positive_set.size() / 4);
  std::cout << "DynamicCuckooFilter<uint16_t> ";
  for (std::string item : positive_set) {
    dcf->Add(item);
  }
  size_t found_count = 0;

  for (std::string item : negative_set) {
    if (dcf->Contains(item) == Ok) {
      found_count++;
    }
  }

  double false_positive_rate = (found_count * 1.) / negative_set.size() * 100;
  std::cout << "False positive rate: (" << found_count << "/"
            << negative_set.size() << ") " << false_positive_rate << "%"
            << std::endl;
}

void testDynamicCuckooFilter32(std::set<std::string> &positive_set,
                               std::set<std::string> &negative_set) {
  std::unique_ptr<DynamicCuckooFilter<uint32_t>> dcf =
      std::make_unique<DynamicCuckooFilter<uint32_t>>(positive_set.size() / 4);
  std::cout << "DynamicCuckooFilter<uint32_t> ";
  for (std::string item : positive_set) {
    dcf->Add(item);
  }
  size_t found_count = 0;

  for (std::string item : negative_set) {
    if (dcf->Contains(item) == Ok) {
      found_count++;
    }
  }

  double false_positive_rate = (found_count * 1.) / negative_set.size() * 100;
  std::cout << "False positive rate: (" << found_count << "/"
            << negative_set.size() << ") " << false_positive_rate << "%"
            << std::endl;
}

void test4(size_t N) {
  std::cerr << "Running: TEST4 - (random) - " << N << std::endl;
  std::cout << "TEST4 - (random) - " << N << std::endl;

  std::set<std::string> positive_set;
  std::set<std::string> negative_set;
  std::string bases = "ABCDEFGHIJKLMNOPRSTUVZ";
  int k = 30;

  while (positive_set.size() < N) {
    std::string kmer;
    kmer.reserve(k);

    for (int i = 0; i < k; ++i) {
      kmer += bases.at(std::rand() % 22);
    }
    positive_set.insert(kmer);
  }
  while (negative_set.size() < N) {
    std::string kmer;
    kmer.reserve(k);

    for (int i = 0; i < k; ++i) {
      kmer += bases.at(std::rand() % 22);
    }
    if (positive_set.find(kmer) == positive_set.end())
      negative_set.insert(kmer);
  }
  testCuckooFilter8(positive_set, negative_set);
  testCuckooFilter16(positive_set, negative_set);
  testCuckooFilter32(positive_set, negative_set);
  testDynamicCuckooFilter8(positive_set, negative_set);
  testDynamicCuckooFilter16(positive_set, negative_set);
  testDynamicCuckooFilter32(positive_set, negative_set);
}

int main(int argc, const char *argv[]) {
  std::srand(987654321);

  for (const size_t N : {50, 100, 500, 1000, 10000, 100000, 1000000}) test4(N);

  return 0;
}