export module hattools.sorts:radical;

import <fstream>;
import <iostream>;
import <format>;
import <memory>;
import <ranges>;
import <vector>;
import <algorithm>;

import hattools.utils;

using namespace std;

export namespace hattools::sorts::radical {

// I had a notion of splitting out radixs and linking them to other radixs and then pidgeon hole sorting them.
// I then thought this was a terrible idea but maybe I should just 'counting sort' each radix.
// It doesn't really work - well it does for small amounts of data and I've never really debugged it properly:
//  a. this is never going to be fast
//  b. far to much re-searching of previously searched space
//  c. the broken bit is the recursive re-assembly - sooner or later with enough numbers it goes haywire
//  d. threading is difficult because of write dependency
//  e. nice to have an idea and try it out, but dead-end so abandoned.

  class Bucket {
  public:
    int nums[10][10]{ 0 };
  };

  struct output {
    int value = 0;
    int block = 0;
    int digit = 0;
  };

  vector<Bucket> bs{};

  vector<int> assMan{};
  // Sample to assemble the output
  output assemble(int bck = 0, int dgt = 9) {
    output oo{};

    for (int currentBucket = bck; currentBucket < bs.size(); ++currentBucket) {
      for (int digit = dgt; digit >= 0; --digit) {
        for (int nextDigit = 9; nextDigit >= 0; --nextDigit) {
          while (bs[currentBucket].nums[digit][nextDigit] > 0) {
            // Found something to do..
            bs[currentBucket].nums[digit][nextDigit]--;
            if (bck != bs.size() - 1) {
              int mult = 1;
              for (int zz = currentBucket; zz < bs.size() - 1; ++zz) {
                mult *= 10;
              }

              output nextOutput = assemble(currentBucket + 1, nextDigit);
              oo.value += nextOutput.value;
              oo.value += digit * mult;
              oo.block = bck;
              oo.digit = digit;
              return oo;
            }
            else {
              oo.value = digit;
              oo.block = bck;
              oo.digit = digit;
              return oo;
            }
          }

        }

      }
    }

    oo.block = -1; // Let caller know we're done
    return oo;
  }

  void mash(int value)
  {
    int digit = value / 1 % 10;
    int olderDigit = 0;
    for (int i = 0; value != 0; ++i) {
      if (i == 0) {
        bs[bs.size() - i - 1].nums[digit][digit]++;
        olderDigit = digit;
      }
      else {
        bs[bs.size() - i - 1].nums[digit][olderDigit]++;
        olderDigit = digit;
      }
      
      value /= 10;
      digit = value / 1 % 10;
    }
    
  }
  

  void sort() {
    for (int i = 0; i < 10; ++i)
    {
      bs.push_back(Bucket());
    }

    vector<int> arr{};
    hattools::utils::generate(1,1'000'000,1'000'000, arr);
    vector<int> stlsort{ arr };

    hattools::utils::Timer t;

    for (auto& i : arr) {
      mash(i);
    }

    /*
    // Lets pretend a number is 321

    bs[1].nums[3][2]++;
    bs[2].nums[2][1]++;
    bs[3].nums[1][1]++;

    
    
    // And another, 20
    bs[2].nums[2][0]++;
    bs[3].nums[0][0]++;
    
    // And another, 1
    bs[3].nums[1][1]++;
    
    // And another, 327
    bs[1].nums[3][2]++;
    bs[2].nums[2][7]++;
    bs[3].nums[7][7]++;
    
    // And another, 4500
    bs[0].nums[4][5]++;
    bs[1].nums[5][0]++;
    bs[2].nums[0][0]++;
    bs[3].nums[0][0]++;
    */
    

    

    //cout << format("num: {}", assemble());
    //cout << format("num: {}", assemble());
    // vector<int> a = bigAss();
    output res{};
    res.block = 0;
    res.digit = 9;
    do {
      res = assemble(res.block);
      if (res.block != -1) {
        assMan.push_back(res.value);
      }
    } while (res.block != -1);

    cout << format("num: {} Time: {}\n", assMan.size(), t.elapsed());
    
    t.reset();
    sort(stlsort.begin(), stlsort.end());
    cout << format("num: {} Time: {}\n", stlsort.size(), t.elapsed());

    reverse(assMan.begin(), assMan.end());
    if (assMan == stlsort) {
      cout << "Suucess\n";
    }

    ofstream of("ass.txt");
    for (int i : assMan) {
      of << i << endl;
    }
    of.close();

    ofstream of2("stl.txt");
    for (int i : stlsort) {
      of2 << i << endl;
    }

    //for (int i = 0; i < assMan.size(); ++i) {
//      cout << format("num: {}\n", assMan[i]);
  //  }

  }

}
