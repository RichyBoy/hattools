export module hattools.utils:generator;

import <random>;
import <algorithm>;
import <vector>;

export namespace hattools::utils {
  const void generate(int lowerBound, int upperBound, int quantity, std::vector<int>& randomNums) {
    std::random_device dev;
    std::mt19937 rng(dev());

    int low{ 0 }, high{ 0 };
    
    if (lowerBound >= 0) {
      low = lowerBound;
      high = upperBound;
    }
    else {
      if (upperBound >= 0) {        
        high = upperBound + -lowerBound;
      }
      else {
        high = -(lowerBound - upperBound);
      }
    }

    std::uniform_int_distribution<std::mt19937::result_type> theRange(low, high);
    randomNums.clear();
    randomNums.resize(quantity);

    for (int i{ 0 }; i < quantity; ++i) {
      int x = static_cast<int>(theRange(rng));
      
      if (lowerBound < 0) {
        x += lowerBound;
      }
      
      randomNums[i] = x;
    }        
  }
}
