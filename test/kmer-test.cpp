#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include <memory>

#include "../src/dynamic-cuckoofilter.h"
#include "generators.h"

using namespace cuckoofilterbio1;

std::string bases = "ACGT";
int k_options[4] = {50, 100, 200, 500};

uint64_t NowNanos() {
  return std::chrono::duration_cast<std::chrono::nanoseconds>(
             std::chrono::steady_clock::now().time_since_epoch())
      .count();
}

// positive_set contains items that should be in CF
// negative_set contains items that shouldn't be in CF
void testCuckooFilter(std::set<std::string> &positive_set,
                      std::set<std::string> &negative_set) {
  std::unique_ptr<CuckooFilter<uint32_t>> cf =
      std::make_unique<CuckooFilter<uint32_t>>(positive_set.size());
  std::cout << "CuckooFilter" << std::endl;

  uint64_t start_time = NowNanos();

  // insert all items from positive_set
  for (std::string item : positive_set) {
    cf->Add(item);
  }

  uint64_t total_add_time = NowNanos() - start_time;
  double avg_add_time = total_add_time / cf->Size();

  std::cout << "Added items: " << cf->Size() << " / " << positive_set.size()
            << " (" << (cf->Size() * 100.) / positive_set.size() << "%)"
            << std::endl;
  std::cout << "Total time to add " << cf->Size()
            << " items: " << total_add_time << "ns (avg. " << avg_add_time
            << "ns/item)" << std::endl;

  start_time = NowNanos();

  for (std::string item : positive_set) {
    if (cf->Contain(item) == NotFound) {
      break;
    }
  }

  uint64_t total_contain_time = NowNanos() - start_time;
  double avg_contain_time = total_contain_time / cf->Size();

  std::cout << "Total time to find " << cf->Size()
            << " items: " << total_contain_time << "ns (avg. "
            << avg_contain_time << "ns/item)" << std::endl;

  uint64_t used_bytes = cf->SizeInBytes();

  std::cout << "Total bytes used: " << used_bytes << " bytes" << std::endl;

  if (negative_set.size() == 0) {
    return;
  }

  size_t found_count = 0;

  for (std::string item : negative_set) {
    if (cf->Contain(item) == Ok) {
      found_count++;
    }
  }

  double false_positive_rate = (found_count * 1.) / negative_set.size() * 100;
  std::cout << "False positive rate: " << false_positive_rate << "%"
            << std::endl;
}

void testDynamicCuckooFilter(std::set<std::string> &positive_set,
                             std::set<std::string> &negative_set) {
  std::unique_ptr<DynamicCuckooFilter<uint32_t>> dcf =
      std::make_unique<DynamicCuckooFilter<uint32_t>>(positive_set.size() / 3);
  std::cout << "DynamicCuckooFilter" << std::endl;

  uint64_t start_time = NowNanos();

  // insert all items from positive_set
  for (std::string item : positive_set) {
    dcf->Add(item);
  }

  uint64_t total_add_time = NowNanos() - start_time;
  double avg_add_time = total_add_time / dcf->TotalSize();

  std::cout << "Added items: " << dcf->TotalSize() << " / "
            << positive_set.size() << " ("
            << (dcf->TotalSize() * 100.) / positive_set.size() << "%)"
            << std::endl;
  std::cout << "Total time to add " << dcf->TotalSize()
            << " items: " << total_add_time << "ns (avg. " << avg_add_time
            << "ns/item)" << std::endl;

  start_time = NowNanos();

  for (std::string item : positive_set) {
    if (dcf->Contains(item) == NotFound) {
      break;
    }
  }

  uint64_t total_contain_time = NowNanos() - start_time;
  double avg_contain_time = total_contain_time / dcf->TotalSize();

  std::cout << "Total time to find " << dcf->TotalSize()
            << " items: " << total_contain_time << "ns (avg. "
            << avg_contain_time << "ns/item)" << std::endl;

  uint64_t used_bytes = dcf->TotalSizeInBytes();

  std::cout << "Total bytes used: " << used_bytes << " bytes" << std::endl;

  if (negative_set.size() > 0) {
    size_t found_count = 0;

    for (std::string item : negative_set) {
      if (dcf->Contains(item) == Ok) {
        found_count++;
      }
    }

    double false_positive_rate = (found_count * 1.) / negative_set.size() * 100;
    std::cout << "False positive rate: " << false_positive_rate << "%"
              << std::endl;
  }

  int counter = 0;
  for (std::string item : positive_set) {
    if (counter++ % 2 == 0) {
      continue;
    }

    dcf->Delete(item);
  }

  std::cout << "Size of each CF before compact: ";
  for (size_t size : dcf->SizeOfEachCF()) {
    std::cout << size << " ";
  }
  std::cout << std::endl;

  start_time = NowNanos();
  dcf->Compact();
  uint64_t compact_time = NowNanos() - start_time;

  std::cout << "Size of each CF after compact: ";
  for (size_t size : dcf->SizeOfEachCF()) {
    std::cout << size << " ";
  }
  std::cout << std::endl;

  std::cout << "Time used for compact: " << compact_time << "ns" << std::endl;
}

void test1(size_t N) {
  std::cerr << "Running: TEST1 - (kmers from e.coli genome) - subset size: "
            << N << std::endl;

  std::cout << "TEST1 - (kmers from e.coli genome) - subset size: " << N
            << std::endl;

  std::set<std::string> positive_set;
  std::set<std::string> negative_set;

  std::ifstream ecoli1("ecoli1.txt");

  try {
    if (!ecoli1.is_open()) throw 1;

    // Ucitaj genom
    std::string genom;
    if (!getline(ecoli1, genom)) throw 2;
    size_t genom_len = genom.size();

    while (positive_set.size() < N) {
      int random_index = std::rand();
      int k = k_options[random_index % 4];
      int pos = random_index % genom_len;

      if ((size_t)(pos + k) > genom_len) continue;

      std::string kmer = genom.substr(pos, k);
      positive_set.insert(kmer);
    }
  } catch (int i) {
    if (i == 1) std::cout << "File failed to open!" << std::endl;
    if (i == 2) std::cout << "File does not contain any data" << std::endl;
  }

  testCuckooFilter(positive_set, negative_set);

  ecoli1.close();

  std::cout << std::endl;
}

void test2(size_t N) {
  std::cerr
      << "Running: TEST2 - (kmers with modification from e.coli genome) - " << N
      << std::endl;

  std::cout << "TEST2 - (kmers with modification from e.coli genome) - " << N
            << std::endl;

  std::set<std::string> positive_set;
  std::set<std::string> negative_set;

  std::ifstream ecoli1("ecoli1.txt");

  try {
    if (!ecoli1.is_open()) throw 1;

    // Ucitaj genom
    std::string genom;
    if (!getline(ecoli1, genom)) throw 2;
    size_t genom_len = genom.size();

    while (positive_set.size() < N || negative_set.size() < N) {
      int random_index = std::rand();
      int k = k_options[random_index % 4];
      int pos = random_index % genom_len;

      if ((size_t)(pos + k) > genom_len) continue;

      std::string kmer = genom.substr(pos, k);
      std::string kmer_wrong = kmer;

      for (int i = 0; i < 2; ++i) {
        random_index = std::rand() % k;

        char change_base = kmer_wrong.at(random_index);
        change_base = bases.at((bases.find(change_base) + rand() % 3 + 1) % 4);
        kmer_wrong.at(random_index) = change_base;
      }

      if (positive_set.size() < N) positive_set.insert(kmer);

      bool kmer_wrong_in_genom = genom.find(kmer_wrong) != std::string::npos;

      if (negative_set.size() < N && !kmer_wrong_in_genom)
        negative_set.insert(kmer_wrong);
    }

  } catch (int i) {
    if (i == 1) std::cout << "File failed to open!" << std::endl;
    if (i == 2) std::cout << "File does not contain any data" << std::endl;
  }

  ecoli1.close();

  testCuckooFilter(positive_set, negative_set);

  std::cout << std::endl;
}

void test3(size_t N) {
  std::cerr << "Running: TEST3 - (random kmers) - " << N << std::endl;
  std::cout << "TEST3 - (random kmers) - " << N << std::endl;

  std::set<std::string> positive_set;
  std::set<std::string> negative_set;

  std::ifstream ecoli1("ecoli1.txt");

  try {
    if (!ecoli1.is_open()) throw 1;

    // Ucitaj genom
    std::string genom;
    if (!getline(ecoli1, genom)) throw 2;

    while (positive_set.size() < N || negative_set.size() < N) {
      int k = 15;

      std::string kmer;
      kmer.reserve(k);

      for (int i = 0; i < k; ++i) {
        kmer += bases.at(std::rand() % 4);
      }

      if (genom.find(kmer) != std::string::npos) {
        if (positive_set.size() < N) positive_set.insert(kmer);
      } else {
        if (negative_set.size() < N) negative_set.insert(kmer);
      }
    }
  } catch (int i) {
    if (i == 1) std::cout << "File failed to open!" << std::endl;
    if (i == 2) std::cout << "File does not contain any data" << std::endl;
  }

  ecoli1.close();

  testCuckooFilter(positive_set, negative_set);

  std::cout << std::endl;
}

int main(int argc, const char *argv[]) {
  std::srand(std::time(nullptr));

  for (const size_t N : {50, 100, 500, 1000, 10000, 100000, 1000000}) test1(N);

  for (const size_t N : {50, 100, 500, 1000, 10000, 100000}) test2(N);

  for (const size_t N : {10, 20, 50}) test3(N);

  return 0;
}