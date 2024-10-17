export module hattools.sorts:pidgeon23;

import <algorithm>;
import <concepts>;
import <ranges>;
import <span>;
import <vector>;

export namespace hattools::sorts::pidgeon23 {

  // There is little difference between this implementation and the hattools-pidgeon implementation.. except for using C++23.
  // Ok, I don't think there is anything specific to C++23 (over C++20) but in truth, most compilers are years behind the C++20 spec.
  // In short, you need to enable C++23 to stand any chance at all of C++20 goodness actually working.
  // The algorithm takes a span, from a vector, and makes sure it is contiguous via a compile-time contract.
  // However the type isn't templated and we still need to create a temporary vector so I've coded this to int. Easy enough 
  // to promote something smaller to int and you probably didn't want to sort any integer type that is bigger anyway..
  
  template <class T>
    requires std::ranges::contiguous_range<T>
  const void sort(T theSpan) {

    // Easy "nothing to do here" case..
    if (theSpan.begin() == theSpan.end()) {
      return;
    }

    const auto [mins, maxs] = std::minmax_element(theSpan.begin(), theSpan.end());

    // Another "nothing to do here" case..
    if (mins == maxs) {
      return;
    }

    // Create two vectors to hold the positive and negative range of numbers.
    // Assuming int here because there is no (sane) way of determining the span underlying type
    std::vector<int> pve{};
    std::vector<int> nve{};

    // The sorted range may be entirely negative.. or positive, so allocate appropriately
    if (*maxs >= 0) {
      pve.resize(*maxs + 1);
    }

    // The positive range is dealing with values == 0 so offset the negative range
    if (*mins <= -1) {
      nve.resize(-(*mins) + 1);
    }

    // Execute a pidgeon hole fill of the vectors    
    for (const auto& i : theSpan) {
      (i >= 0) ? pve[i]++ : nve[-i]++;
    }  

    // Shrink the two pidgeon holes back to the original iterator range, adjusting for sign in the negative range.

    // One other optimisation - the entire range maybe negative, or entirely positive. If that is the case there
    // is no need to search the whole vector.. we start at min and end at max (if positive only)
    // or start at max and end at min (if negative only).      
    auto boundary{ *maxs };
    auto place{ 0 };

    // Negative range first
    if (*maxs >= 0) {
      boundary = 0;
    }
    
    // Compiler won't allow an auto, nor will it allow size_t from nve.size() ...
    for (int i : std::views::reverse(std::ranges::iota_view(boundary, static_cast<int>(nve.size())))) {    
      for (auto const& j : std::ranges::iota_view(0, nve[i])) {    
        theSpan[place++] = -i;
      }
    }

    // Now for the positive range
    if (*mins >= 0) {
      boundary = *mins;
    }

    for (auto i : std::ranges::iota_view(boundary, static_cast<int>(pve.size()))) {    
      for (auto const& j : std::ranges::iota_view(0, pve[i])) {
        theSpan[place++] = i;
      }
    }
  }
}
