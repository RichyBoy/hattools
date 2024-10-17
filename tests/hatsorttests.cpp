import <algorithm>;
import <execution>;
import <format>;
import <fstream>;
import <iostream>;
import <limits>;
import <ranges>;
import <thread>;
import <vector>;

import hattools.sorts;
import hattools.utils;

using namespace hattools::sorts;
using namespace std;

void doSorts(ofstream& out, vector<int>& data, auto& fillRate) {
  constexpr int chunk = 2000; // Thread fragment size
  vector<int> pidgeon23Data{ data };
  vector<int> pidgeonData{ data };
  vector<int> pidgeonFlockData{ data };
  vector<int> stlsortData{ data };
  vector<int> stlsortParData{ data };
  vector<int> stlsortParData2{ data };
  hattools::utils::Timer t;

  // Pidgeon sort  
  t.reset();
  pidgeon::sort(pidgeonData);      
  out << format(",{:.7f}", t.elapsed());

  // Pidgeon23 sort
  t.reset();
  pidgeon23::sort(span<int>{ pidgeon23Data });
  out << format(",{:.7f}", t.elapsed());

  // Pidgeon Flock sort - running with 4 threads
  t.reset();
  pidgeonflock::sort(std::span<int>{ pidgeonFlockData }, 4, chunk);
  //pidgeonflock::sort(std::span<int>{ pidgeonFlockData }, std::thread::hardware_concurrency(), chunk);
  out << format(",{:.7f}", t.elapsed());

  // STL non-parallel sort
  t.reset();
  sort(stlsortData.begin(), stlsortData.end());
  out << format(",{:.7f}", t.elapsed());

  // STL parallel sort
  t.reset();
  sort(execution::par, stlsortParData.begin(), stlsortParData.end());
  out << format(",{:.7f}", t.elapsed());

  // STL parallel unseq sort (in theory, vectorised)
  t.reset();
  sort(execution::par_unseq, stlsortParData2.begin(), stlsortParData2.end());
  out << format(",{:.7f}\n", t.elapsed());

  t.reset();
  if (pidgeonData == stlsortData &&
    pidgeon23Data == stlsortData &&
    stlsortParData == stlsortData &&
    stlsortParData2 == stlsortData &&
    pidgeonFlockData == stlsortData) {
    out  << format("OK,{:.7f}\n", t.elapsed());
  }
  else {
    out << format("NOK,{:.7f}\n\n", t.elapsed());
  }
}

// At the denser end of the spectrum, between 0.5% and 25% fill densisty
// At the sparse end of the spectrum, between 0.05% and 2.5% fill density
void sparse(ofstream& out) {
  std::vector<int> arr{};

  for (auto i = 1'000'000; i <= 10'000'000; i += 1'000'000) {
    for (auto j = 10'000; j <= 500'000; j += 10'000) {
      arr.clear();
      hattools::utils::generate(-i, i, j, arr);      
      auto fillRate = static_cast<float>(j) / (i + i);
      out << format("{},{},{},{:.2f},", -i, i, j, fillRate);
      doSorts(out, arr, fillRate);
    }
  }
}

// At the denser end of the spectrum, between 100% and 1,000% densisty
// At the sparse end of the spectrum, between 0.5% and 100% fill density
void dense(ofstream& out) {
  std::vector<int> arr{};

  for (auto i = 1'000'000; i <= 10'000'000; i += 1'000'000) {
    for (auto j = 1'000'000; j <= 10'000'000; j += 1'000'000) {
      arr.clear();
      hattools::utils::generate(-i, i, j, arr);      
      auto fillRate = static_cast<float>(j) / (i + i);
      out << format("{},{},{},{:.2f},", -i, i, j, fillRate);
      doSorts(out, arr, fillRate);
    }
  }
}

// At the denser end of the spectrum, between 100% and 500% densisty
// At the sparse end of the spectrum, between 100% and 10% fill density
void equal(ofstream& out) {
  std::vector<int> arr{};

  for (auto i = 1'000'000; i <= 50'000'000; i += 1'000'000) {
    for (auto j = 1'000'000; j <= 5'000'000; j += 1'000'000) {
      arr.clear();
      hattools::utils::generate(-i, i, j, arr);      
      auto fillRate = static_cast<float>(j) / (i + i);
      out << format("{},{},{},{:.2f},", -i, i, j, fillRate);
      doSorts(out, arr, fillRate);
    }
  }
}

int main()
{
  vector<string> files = { "sparse.csv", "equal.csv", "dense.csv" };

  for(unsigned int i = 0; i < files.size(); ++i) {
    ofstream p1{};
    p1.open(files[i]);
    p1 << "lowerBound,upperBound,elementCount,fill,elapsedPidgeon,elapsedPidgeon23,elapsedFlock,elapsedSTL,elapsedSTLPar,elapsedSTLParSeq\n";
    // Lazy, I know...
    if (i == 0) {
      sparse(p1);
    } else if (i == 1) {
      equal(p1);
    } else if (i == 2) {
      dense(p1);
    }
   
    p1.close();
  }

  return 0;
  }
