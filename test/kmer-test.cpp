#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>

#include "generators.h"
/*
std::string str((std::istreambuf_iterator<char>(ecoli)),
                    std::istreambuf_iterator<char>());
    str.erase(std::remove(str.begin(), str.end(), '\n'), str.end());
    // std::cout << str << std::endl;
    ecoli1 << str; */

std::string bases = "ACGT";
int k_options[4] = {50, 100, 200, 500};

void test1(int N) {
  std::cout << "TEST1" << std::endl;

  std::set<std::string> positive_set;

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

      if (pos + k > genom_len) continue;

      std::string kmer = genom.substr(pos, k);
      positive_set.insert(kmer);
    }
  } catch (int i) {
    if (i == 1) std::cout << "Datoteka se nije otvorila " << std::endl;
    if (i == 2) std::cout << "Nisi u string ubacio genom" << std::endl;
  }

  ecoli1.close();
}

void test2(int N) {
  std::cout << "TEST2" << std::endl;

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

      if (pos + k > genom_len) continue;

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
    if (i == 1) std::cout << "Datoteka se nije otvorila " << std::endl;
    if (i == 2) std::cout << "Nisi u string ubacio genom" << std::endl;
  }

  ecoli1.close();
}

void test3(int N) {
  std::cout << "TEST3" << std::endl;

  std::set<std::string> positive_set;
  std::set<std::string> negative_set;

  std::ifstream ecoli1("ecoli1.txt");

  try {
    if (!ecoli1.is_open()) throw 1;

    // Ucitaj genom
    std::string genom;
    if (!getline(ecoli1, genom)) throw 2;

    std::string kmer;

    while (positive_set.size() < N || negative_set.size() < N) {
      int random_variable = std::rand();

      // k = k_options[random_variable % 4];
      int k = 15;
      kmer.clear();
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
    if (i == 1) std::cout << "Datoteka se nije otvorila " << std::endl;
    if (i == 2) std::cout << "Nisi u string ubacio genom" << std::endl;
  }

  ecoli1.close();
}

int main(int argc, const char* argv[]) {
  std::srand(std::time(nullptr));
  test1(100);
  test2(100);
  test3(10);

  return 0;
}