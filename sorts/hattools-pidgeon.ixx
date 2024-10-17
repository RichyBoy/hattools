export module hattools.sorts:pidgeon;

import <algorithm>;
import <vector>;

export namespace hattools::sorts::pidgeon {
  
  // This is a 'legacy' implementation and is also the fastest - modern C++ is pretty and easy to read but is also
  // a bit slower. This code can be built stand-alone by removing the module which is the only 'fancy' bit of C++ in this source.
  // The algorithm doesn't use iterators (although it could do) and it needs the template type as it must contruct a temporary
  // vector of the data type for the pidgeon hole sort. Being legacy C++ it isn't attempting to restrict the type of the sort
  // but this will only work with integer types, the entire range of INT_MIN to INT_MAX will take 4 GB of memory plus your initial
  // vector, you can sort long if you so wish but you probably don't want to go beyond 35 bits unless > 32 GB of memory.

  template <class T>
  const void sort(T& data) {

    // Easy "nothing to do here" case..
    if (data.begin() == data.end()) {
      return;
    }

    auto max{ 0 };
    auto min{ 0 };    

    // Determine the largest and smallest value
    for (const auto& i : data) {
      if (i > max) {
        max = i;
      }
      else if (i < min) {
        min = i;
      }
    }

    // Another "nothing to do here" case..
    if (min == max) {
      return;
    }    

    // Create two vectors to hold the positive and negative range of numbers
    T pve(max + 1);
    T nve(-min + 1);

    // Execute a pidgeon hole fill of the vectors
    for (const auto& i : data) {
      (i >= 0) ? pve[i]++ : nve[-i]++;      
    }

    // Shrink the two pidgeon holes back to the original iterator range, adjusting for sign in the negative range.

    // One other optimisation - the entire range maybe negative, or entirely positive. If that is the case there
    // is no need to search the whole vector.. we start at min and end at max (if positive only)
    // or start at max and end at min (if negative only).      
    auto boundary{ max };
    auto place{ 0 };

    // Negative range first
    if (max >= 0) {
      boundary = 0;
    }

    for (auto i{ static_cast<int>(nve.size() - 1) }; i > boundary; --i) {
      for (auto j = 0; j < nve[i]; j++) {        
        data[place++] = -i;
      }
    }

    // Now for the positive range
    if (min >= 0) {
      boundary = min;
    }

    for (auto i{ boundary }; static_cast<size_t>(i) < pve.size(); ++i) {
      for (auto j{ 0 }; j < pve[i]; j++) {        
        data[place++] = i;
      }
    }    
  }
}