#include <bitset>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

using std::cout, std::vector, std::string, std::byte, std::ios, std::bitset;
std::ifstream inFile{};
std::ofstream outFile{};
bitset<8> extra{};
size_t nextExtra = 0u;

constexpr size_t CHUNK_SIZE = 1024 * 4u;

void add_bitset9_to_compressed(bitset<9> bits, vector<char>& compressed,
                               bool Finish = false) {
  /* bit scheme
   * add 9 bits to compressed
   * x x x x _ _ _      extras
   * v v v v v v v v v  bits to add
   * _ _ _ _ _ _ _ _    newByte
   * 1. copy extras to the new byte
   * x x x x _ _ _ _
   * 2. add some new bits
   * x x x x v v v v
   * add leftover new bits to extra
   *
   */
  // write combo of extra bits and new bits; put remaining into extra
  auto new_byte = bitset<8>{};
  size_t nextNew = 0u;
  for (size_t i = 0; i < nextExtra; i++) {
    new_byte[i] = extra[i];
  }

  for (size_t i = nextExtra; i < 8; i++) {
    new_byte[i] = bits[nextNew];
    nextNew += 1u;
  }
 
  compressed.push_back(new_byte.to_ulong());
  nextExtra = 0u;
  for (size_t i = nextNew; i < 9; i++) {
    extra[nextExtra] = bits[i];
    nextExtra += 1u;
  }
  cout << "size of extra: " << nextExtra << "\n";
}

void compress() {
  // Read inFile as chunks
  std::vector<char> compressed{};
  assert(sizeof(char) == 1u);
  std::string buffer;
  std::string lastBuf;
  buffer.resize(CHUNK_SIZE);
  inFile.seekg(0);
  inFile.clear();
  while (1) {
    inFile.read(buffer.data(), CHUNK_SIZE);
    // To write unmodified straight to file.
    // std::copy(lastBuf.begin(),lastBuf.end(),std::back_inserter(compressed));
    size_t numRead = inFile.gcount();
    for (size_t i = 0; i < numRead; i++) {
      // for each byte in buffer; check for repeats
      //
    }
    lastBuf = buffer;
    lastBuf.resize(numRead);
    if (numRead <= 0) {
      break;
    }
  }
  outFile.write(compressed.data(), compressed.size());
}

void decompress() {}

int main(int argc, char* argv[]) {
  if (argc < 2) {
    cout << "need more args; exiting\n";
    return 1;
  }
  for (int i = 0; i < argc; i++) {
    cout << "arg # " << i << "=" << argv[i] << "\n";
  }
  string firstArg = argv[1];
  if (firstArg == "-d" || firstArg == "d") {
    string inName = "compressed";
    inFile.open(inName, ios::in | ios::binary | ios::ate);
    outFile.open("decompressed", ios::out | ios::binary);
    cout << "decompressing file " << inName << " of size " << inFile.tellg()
         << "\n";
    decompress();
  } else {
    inFile.open(firstArg, ios::in | ios::binary | ios::ate);
    outFile.open("compressed", ios::out | ios::binary);
    cout << "compressing file " << firstArg << " of size " << inFile.tellg()
         << "\n";
    compress();
  }
  inFile.close();
  outFile.close();
  return 0;
}
