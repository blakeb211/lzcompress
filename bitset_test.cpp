#include <algorithm>
#include <bitset>
#include <cassert>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <queue>
#include <unordered_map>
#include <vector>
using namespace std;

/*  COMPRESS   progname filename_to_compress
 *  DECOMPRESS progname d
 ****************************************************************
 *  COMPRESION METHOD:
 *  Dynamically build map of substrings in previous window.
 *  Write non compressed bytes as 9bits, first bit is a flag.
 *  If match found in dictionary, add two bytes as an index into the dictionary.
 *
 *  DECOMPRESSION METHOD:
 *  Build same dictionary as the compressor and replace the indices with the
 *substring.
 *
 *  LOOKBACK WINDOW CALCULATION:
 *  Note: Using term 'substrings' but really mean byte sequences.
 *  Substrings in N bytes: (1/2) * N * (N+1) - N - (N-1) - (N-2)
 *  Solution is N == 364 for length 3 or larger.
 *  Typically some will be duplicates.
 */

queue<int> extra = {};
vector<char> compressed = {};
constexpr bool DEBUGMODE = false;

void push_9bits_to_compressed(bitset<9> newBits) {
  auto writeByte = bitset<8>{};
  size_t writeCnt = 0u;
  size_t numNewAdded = 0u;
  size_t extraSz = extra.size();
  assert(extraSz <= 8);
  // copy extra to writeByte and clear it
  for (int i = 0; i < extraSz; i++) {
    writeByte[i] = extra.front();
    extra.pop();
    writeCnt += 1u;
  }
  extraSz = extra.size();
  assert(writeCnt <= 8);
  // fill rest of writeByte with newBits
  auto numAlreadyWritten = writeCnt;
  for (int i = numAlreadyWritten; i < 8; i++) {
    writeByte[i] = newBits[numNewAdded];
    writeCnt += 1u;
    numNewAdded += 1u;
  }
  assert(writeCnt <= 8);
  if (writeCnt == 8) {
    // cout << "writing " << writeByte << " to compressed\n";
    compressed.push_back(writeByte.to_ulong());
    writeCnt = 0;
  }
  // put rest of newBits in extra
  size_t newBitsSz = newBits.size();
  for (int i = numNewAdded; i < newBitsSz; i++) {
    extra.push(newBits[i]);
    numNewAdded += 1u;
  }
  assert(numNewAdded == newBitsSz);
  // write any full bytes present in extra
  while (extra.size() >= 8) {
    bool a1 = extra.front();
    writeByte[0] = a1;
    extra.pop();
    bool a2 = extra.front();
    writeByte[1] = a2;
    extra.pop();
    bool a3 = extra.front();
    writeByte[2] = a3;
    extra.pop();
    bool a4 = extra.front();
    writeByte[3] = a4;
    extra.pop();
    bool a5 = extra.front();
    writeByte[4] = a5;
    extra.pop();
    bool a6 = extra.front();
    writeByte[5] = a6;
    extra.pop();
    bool a7 = extra.front();
    writeByte[6] = a7;
    extra.pop();
    bool a8 = extra.front();
    writeByte[7] = a8;
    extra.pop();
    // cout << "writing " << writeByte << " to compressed\n";
    compressed.push_back(writeByte.to_ulong());
  }
}

void push_17bits_to_compressed(bitset<17> newBits) {
  auto writeByte = bitset<8>{};
  size_t writeCnt = 0u;
  size_t numNewAdded = 0u;
  size_t extraSz = extra.size();
  assert(extraSz <= 8);
  // copy extra to writeByte and clear it
  for (int i = 0; i < extraSz; i++) {
    writeByte[i] = extra.front();
    extra.pop();
    writeCnt += 1u;
  }
  extraSz = extra.size();
  assert(writeCnt <= 8);
  // fill rest of writeByte with newBits
  auto numAlreadyWritten = writeCnt;
  for (int i = numAlreadyWritten; i < 8; i++) {
    writeByte[i] = newBits[numNewAdded];
    writeCnt += 1u;
    numNewAdded += 1u;
  }
  assert(writeCnt <= 8);
  if (writeCnt == 8) {
    // cout << "writing " << writeByte << " to compressed\n";
    compressed.push_back(writeByte.to_ulong());
    writeCnt = 0;
  }
  // put rest of newBits in extra
  size_t newBitsSz = newBits.size();
  for (int i = numNewAdded; i < newBitsSz; i++) {
    extra.push(newBits[i]);
    numNewAdded += 1u;
  }
  assert(numNewAdded == newBitsSz);
  // write any full bytes present in extra
  while (extra.size() >= 8) {
    bool a1 = extra.front();
    writeByte[0] = a1;
    extra.pop();
    bool a2 = extra.front();
    writeByte[1] = a2;
    extra.pop();
    bool a3 = extra.front();
    writeByte[2] = a3;
    extra.pop();
    bool a4 = extra.front();
    writeByte[3] = a4;
    extra.pop();
    bool a5 = extra.front();
    writeByte[4] = a5;
    extra.pop();
    bool a6 = extra.front();
    writeByte[5] = a6;
    extra.pop();
    bool a7 = extra.front();
    writeByte[6] = a7;
    extra.pop();
    bool a8 = extra.front();
    writeByte[7] = a8;
    extra.pop();
    // cout << "writing " << writeByte << " to compressed\n";
    compressed.push_back(writeByte.to_ulong());
  }
}

void print_info() {
  cout << "compressed size: " << compressed.size() << "\n";
  cout << "size of extra: " << extra.size() << "\n";
  for (size_t i = 0; i < extra.size(); i++) {
    cout << extra.front();
    extra.pop();
  }
  cout << "\n";
}

bitset<24> getBits(string& buf, int num, bool resetCounters = false) {
  // get some bits from the buffer as if streaming
#if DEBUGMODE
  cout << "getbits call\n";
#endif
  // This will always be either 1, 8, 16
  // 0xc2 = '0b11000010'
  assert(num == 1 || num == 8 || num == 16);
  bitset<24> ret;

  static size_t byte{0u};
  static size_t bit{0u};
  if (resetCounters) {
    byte = 0u;
    bit = 0u;
    return bitset<24>();
  }
  bitset<8> data(buf[byte]);
  while (num > 0) {
    ret = ret >> 1;
    ret[ret.size() - 1] = data[bit];
    bit++;
    num--;
    if (bit == 8u) {
      byte++;
      data = buf[byte];  // waste if we are on the last needed bit
      bit = 0u;
    }
    assert(byte <= buf.size());
  }
  return ret;
}

void decompress() {
  // Show that we can read a hybrid 9bit and 8bit data format.
  // Then can implement the lempel-ziv algo.
  /*
  _ _ _ _ _ _ _ f _ _ _ _ _ _ f _

  if flag == 0; next flag is + 9
  if flag == 1; next flag is + 9
  9bits where 8 is data
  9bits where next 16 is count and 8 are length

  test_file    0x61 0x62
  compressed   0xc2 0x88

  */
  auto inFile = ifstream("compressed", ios::in | ios::binary | ios::ate);
  auto outFile = ofstream("decompressed", ios::out | ios::binary);
  auto szIn = inFile.tellg();
  size_t totBits = szIn * 8u;
  inFile.seekg(0);
  string buffer;
  buffer.resize(szIn);
  inFile.read((char*)buffer.data(), szIn);
  inFile.close();
  cout << "(" << "sizein:" << szIn << ")";
  //
  size_t flagPos = 0u;
  bitset<64> bitbuffer{};
  size_t nextBuf = 0u;
  size_t bitCnt = 0u;

  // Dictionary
  std::unordered_map<string, size_t> dic;
  std::unordered_map<string, uint16_t> sub2id;
  std::queue<uint16_t> ids;
  const size_t MAXKEYLEN{5};
  const size_t MAXDICSZ{65535};
   
  while (bitCnt < totBits) {
    auto flagBit = getBits(buffer, 1)[0];
    bitCnt += 1;
    if (flagBit == 0) {
      char byte = (getBits(buffer,8) >> 16).to_ulong();
      outFile.write(&byte,1);
      bitCnt += 8;
    }
    if (flagBit == 1) {
      uint16_t num = (getBits(buffer,16) >> 8).to_ulong();
      outFile.write(reinterpret_cast<char*>(&num),2);
      bitCnt += 16;
    }
    // write 3 function for this; top 9 to bitset, top17 to bitset
    /* 
    for (int i = 8 - 1; i >= 0; i--) {
      //   _  _  _  _  _  _  _  _
      //  23 22 21 20 19 18 17 16
      //   7  6  5  4  3  2  1  0
      writeByte[i] = inByte[16 + i];
    }
    */
    //outFile.write(&topByte, 1);
  }
}

bool test1_push_9bits() {
  // push test data as 9 bits to compressed and check it; then reset extra and
  // compressed;
  uint8_t input[8] = {0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0x0a};
  for (int i = 0; i < 8; i++) {
    bitset<9> write = input[i];
    write <<= 1;  // shift once left because index 0 is the flag bit; always 0
                  // for this test
    cout << write << "\n";
    push_9bits_to_compressed(write);
  }
  cout << "size compressed and extra: " << compressed.size() << " "
       << extra.size() << "\n";
  string buffer(compressed.data(), compressed.size());
  cout << "size of buffer: " << buffer.size() << "\n";

  int matches = 0;
  int i = 0;
  while (i < buffer.size() - 1) {
    auto unused = getBits(buffer, 1);  // throw away flag bit
    bitset<24> gotten = getBits(buffer, 8);
    cout << "gotten:" << gotten << "\n";
    bitset<8> expected(input[i]);
    cout << "expected:" << expected << "\n";
    if (bitset<8>((gotten >> 16).to_ulong()) == expected) {
      matches++;
    } else {
      cout << "DOES NOT MATCH\n";
    }
    i++;
  }
  cout << "Match count " << matches << "\n";
  compressed.clear();
  return matches == 8 && extra.size() == 0 && compressed.size() == 0;
}

bool test2_push_17bits() {
  // Push 17 bits to compressed (8 times) and make sure can read
  // the number correctly.
  for (size_t i = 0; i < 8; i++) {
    // 16 15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00
    //  _  _  _  _  _  _  _  _  _  _  _  _  _  _  _  _  f
    //  Use i*7 as the 2 byte payload of these 17 bit hunks.
    //  Flag bit on far right is 0. It is not used in this test.
    push_17bits_to_compressed(bitset<17>(uint16_t(i * 7u)) << 1);
  }
  cout << "test2 compressed.size() " << compressed.size() << "\n";
  cout << "extra size extra.size() " << extra.size() << "\n";
  string buffer(compressed.data(), compressed.size());
  int i = 0;
  int matches = 0;
  while (i < 8) {
    auto unused = getBits(buffer, 1);
    bitset<24> bits = getBits(buffer, 16);
    bitset<16> top16 = bits.to_ulong() >> 8;
    bitset<16> expected = i * 7u;
    matches += (expected == top16);
    cout << "top16    : " << top16 << "\n";
    cout << "expected : " << expected << "\n";
    i++;
  }
  cout << "Match count " << matches << "\n";
  compressed.clear();
  return matches == 8 && extra.size() == 0 && compressed.size() == 0;
}

int main(int argc, char** argv) {
  if (argc > 1 && std::string(argv[1]).compare("t") == 0) {
    /*
     *  TESTING BLOCK
     */
    cout << "testing\n";
    if (test1_push_9bits() == true) {
      cout << "test1_push_9bits_PASSED\n";
    } else {
      cout << "test1_push_9bits_FAILED\n";
    }
    string _fakeBuf;
    getBits(_fakeBuf, 1, true);  // reset static vars in getBits
    if (test2_push_17bits() == true) {
      cout << "test2_push_17bits_PASSED\n";
    } else {
      cout << "test2_push_17bits_failed\n";
    }
    getBits(_fakeBuf, 1, true);  // reset static vars in getBits
    return 0;
  }
  if (argc > 1 && std::string(argv[1]).compare("d") == 0) {
    cout << "decompressing\n";
    decompress();
    return 0;
  }
  if (argc < 2) {
    cout << "First arg is a 'd' for decompress, a 't' for testing, or a "
            "filename for compression\n";
    exit(1);
  }
  // COMPRESSING
  std::unordered_map<string, size_t> dic;
  std::unordered_map<string, uint16_t> sub2id;
  std::queue<uint16_t> ids;
  auto inFile = ifstream(argv[1], ios::in | ios::binary | ios::ate);
  size_t inSz = inFile.tellg();
  inFile.seekg(0);
  string buffer;
  buffer.resize(inSz);
  inFile.read((char*)buffer.data(), buffer.size());
  inFile.close();
  //    COMPRESSION
  //    if flag == 0, it's followed by 8 bits
  //    if flag == 1, it's followed by 16 bits
  const size_t MAXKEYLEN{5};
  const size_t MAXDICSZ{65535};
  for (uint16_t i = 0; i < MAXDICSZ; i++) {
    ids.push(i);
  }
  for (int i = 0; i < inSz; i += 1) {
  INNER_START:
    for (int ii = MAXKEYLEN; ii > 2 && i + MAXKEYLEN < inSz; ii--) {
      string sub = buffer.substr(i, ii);
      if (dic.count(sub) > 0) {
        /*
         * MATCH FOUND
         */
        // increment dic
        dic[sub] = dic[sub] + 1;
        uint16_t subid = sub2id[sub];
        bitset<17> toWrite(subid);
        toWrite <<= 1;
        toWrite[0] = 1;
        // push 17 bits, always starting with a 1
        push_17bits_to_compressed(toWrite);
        // increment i and break;
        i += ii;
        goto INNER_START;
      } else {
        /*
         * NO MATCH
         */
        assert(sub2id.size() == dic.size());
        // Add entry to dic and sub2id
        dic[sub] = 1;
        sub2id[sub] = ids.front();
        ids.pop();
        if (ids.size() == 0) {
          // cull dic and sub2id, returning ids to ids
          // Calculate threshold first.
          size_t threshold = 0;
          size_t to_remove = 0;
          while (to_remove < 8000) {
            threshold += 1u;
            to_remove = std::count_if(
                dic.begin(), dic.end(),
                [&](const auto& pair) { return pair.second < threshold; });
          }
          std::queue<string> dicKeyToRemove;
          for (const auto pair : dic) {
            if (pair.second <= threshold) {
              ids.push(sub2id[pair.first]);
              sub2id.erase(pair.first);
              dicKeyToRemove.push(pair.first);
            }
          }
          //cout << "cull-"<< "(" << threshold << "," << dicKeyToRemove.size() << ")";
          size_t numToRemove = dicKeyToRemove.size();
          while (numToRemove > 0) {
            dic.erase(dicKeyToRemove.front());
            dicKeyToRemove.pop();
            numToRemove--;
          }
        }  // End Cull
      }
    }
    // If reach here, we checked substrings 3 through MAXKEYLEN
    // and no matches were found. Entries were added to dic.
    // Write the two 9bit vals and increment i
    push_9bits_to_compressed(bitset<9>(buffer[i]) << 1);
  }
  auto outFile = ofstream("compressed", ios::out | ios::binary);
  outFile.write(compressed.data(), compressed.size());
  outFile.close();
  return 0;
}
