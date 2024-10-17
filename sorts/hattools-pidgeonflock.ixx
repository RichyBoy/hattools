export module hattools.sorts:pidgeonflock;

import <algorithm>;
import <concepts>;
import <execution>;
import <limits>;
import <thread>;
import <ranges>;
import <vector>;

// Extra note: If I put a execution policy into minmax_element or transform, gcc shits the bed, similar to gcc bug
// I raised for algorithm coming after vector (or vice-versa I forget which)

export namespace hattools::sorts::pidgeonflock {

  // This algorithm is more of a demonstration of "how to do it" rather than being computationally advantageous:
  //   
  //   1. Memory clock rates are considerably slower than the CPU/thread clock rate; the single thread is probably optimal already for flat-lining memory performance.
  //     a. With two memory channels in use it may be possible to write an allocator (or just over-reserve your vector) to force the
  //        temporary vectors in this algorithm to be allocated onto a different channel. This is probably a waste of effort/time.
  //   2. Beware memory! If you have 24 threads, you are going to get 24 seperate pidgeon hole vectors! If your vector has a range of 
  //      0 - 100,000, there would be 24 pidgeon hole vectors of 100K elements each, 2.4 million elements * sizeof(int). 
  //      a. It would be quite easy to "force the hand" of point 1a above - with a high thread count and large(ish) number of elements.
  //   3. I would guess that 4 or less threads is the only sane usage, even then probably only 2. It's all about memory clock rate and not CPU utility.  
  //      It's a demonstration of "how to".
  //
  // Pidgeon hole sorting is taking advantage of what has been overlooked for a fair few years - that resources are now plentiful and cheap. This algorithm is however
  // waving its fingers in the air at this and is using copious, silly amounts of memory. There is no advantage to be gained in once your memory
  // performance is saturated. However with threading it is adapatble to becoming a co-routine and could be adopted functionaly as well as 2 threads may well be viable.
  //
  // The algorithm takes the original span (the vector range you wish to sort) and slices it up into pieces. There is a piece for each thread available
  // but there is a minimum constrait of 2,000 elements per piece (I recall reading somewhere in a guidelines piece that the cost of starting up a thread
  // isn't returned until you consider 2,000 or more elements). E.g., you can have 24 threads but if you only have 100 elements - you will have just 1 thread.
  // 24 threads would be used the moment you exceed 48,000 elements. Both thread count and piece chunk can be overridden.
  //
  // class Pieces records the information for each slice of work, with its own pidgeon hole -ve/+ve vectors. The filler routine is the core part of the algorithm for 
  // generating the pidgeon hole vectors. The re-assembly of the span could easily be performed across many threads but I've opted for just two threads.
  //
  // What can't be done in parallel is rebuilding the original span as that requires write events - and the cost of locking for a single memory write is obviously
  // prohibitive and these writes are all back-to-back. What can be done in parallel is use a STL transform on each piece, to generate an additive vector, which can 
  // then be used to re-order the original span. This is quite expensive if you have 24 sparsely populated vectors to add together.. another reason why parallelism makes
  // little sense here. It would also seem that you can thread the finally assembly of the span, but you need to know the correct place value to know where to start
  // within your thread.. doable, if you change the STL minmax for our own method and record such details, but not worth the trouble.

  class Pieces {
  public:
    std::span<int> coverage{};
    int start{ 0 };
    int finish{ 0 };
    int min{ 0 };
    int max{ 0 };
    std::vector<int> pve{}, nve{};
  };

  void filler(Pieces& thePiece) {//, Locks& ll) {    
    // Perform pidegeon hole fill
    for (const auto& i : thePiece.coverage) {
      (i >= 0) ? thePiece.pve[i]++ : thePiece.nve[-i]++;      
    }
  }

  template <class T>
    requires std::ranges::contiguous_range<T>
  const void sort(T theSpan, unsigned int threads = 1, int chunk = 2000) {
    //int mmin{ std::numeric_limits<int>::max() };
    // int mmax{ std::numeric_limits<int>::min() };

    // Easy "nothing to do here" case..
    if (theSpan.begin() == theSpan.end()) {
      return;
    }

    // Determine how many threads to use
    unsigned int chunks{ static_cast<unsigned int>(static_cast<float>(theSpan.size()) / chunk) };
    if (chunks >= threads) {
      chunk = static_cast<int>(static_cast<float>(theSpan.size()) / threads);
    }
    else if (chunks > 1 && chunks < threads) {
      threads = chunks;
      chunk = static_cast<int>(static_cast<float>(theSpan.size()) / threads);
    }
    else {
      chunk = theSpan.size();
      threads = 1;
    }
    
    std::vector<Pieces> thePieces{};
    const auto [mins, maxs] = std::minmax_element(theSpan.begin(), theSpan.end());

    // Another "nothing to do here" case..
    if (*mins == *maxs) {
      return;
    }

    for (unsigned int i{ 0 }; i < threads; ++i) {
      Pieces newPiece{};
      newPiece.start = i * chunk;
      newPiece.finish = i * chunk + chunk - 1;

      // Last piece will take a few extra elements to account for rounding error in division, so get the finish and span right..
      if (i == threads - 1) {
        //newPiece.finish = static_cast<int>(theSpan.size()) - 1;
        newPiece.coverage = std::span{ theSpan.subspan(newPiece.start) };
        newPiece.finish = newPiece.start + newPiece.coverage.size();
      }
      else {
        newPiece.coverage = std::span{ theSpan.subspan(newPiece.start, newPiece.finish - newPiece.start + 1) };
      }

      // Allocate the pidgeon hole vectors.. each piece must be of identical size to stitch the reult back with std::transform later
      if (*maxs >= 0) {
        newPiece.pve.resize(*maxs + 1);
      }

      if (*mins <= -1) {
        newPiece.nve.resize(-*(mins)+1);
      }

      newPiece.min = *mins;
      newPiece.max = *maxs;
      thePieces.push_back(newPiece);
    }

    // Execute the pidgeon hole fills across the required number of threads
    {
      std::vector<std::jthread> th;
      for (unsigned int i{ 0 }; i < threads; ++i) {
        th.push_back(std::jthread{ filler, std::ref(thePieces[i]) }); 
      }
    }

    // Add all the pidgeon holes together and stick the results in the very first slice  
    for (int i = 1; i < static_cast<int>(threads); ++i) {
      std::transform(thePieces[0].nve.begin(), thePieces[0].nve.end(), thePieces[i].nve.begin(), thePieces[0].nve.begin(), std::plus<int>());
      std::transform(thePieces[0].pve.begin(), thePieces[0].pve.end(), thePieces[i].pve.begin(), thePieces[0].pve.begin(), std::plus<int>());
    }  

    // Finally reconstruct the data within the span
    auto boundary{ *maxs };
    auto place{ 0 };

    // Negative range first
    if (*maxs >= 0) {
      boundary = 0;
    }
    
    for (auto i{ static_cast<int>(thePieces[0].nve.size() - 1) }; i > boundary; --i) {
      for (auto j = 0; j < thePieces[0].nve[i]; j++) {
        theSpan[place++] = -i;
      }
    }

    // Now for the positive range
    if (*mins >= 0) {
      boundary = *mins;
    }

    for (auto i{ boundary }; static_cast<size_t>(i) < thePieces[0].pve.size(); ++i) {
      for (auto j{ 0 }; j < thePieces[0].pve[i]; j++) {
        theSpan[place++] = i;
      }
    }   
  }
}
