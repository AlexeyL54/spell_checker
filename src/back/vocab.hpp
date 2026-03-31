#pragma once

#include <memory>
#include <string>
#include <sys/types.h>
#include <unordered_map>
#include <vector>

using std::shared_ptr;
using std::string;
using std::unordered_map;
using std::vector;
using std::wstring;

class Vocabulary {

public:
  Vocabulary(const string &path);

  void loadVocab();

  wstring readFromFile();

  int createHashCode(const wstring &str);

  bool isInVocab(int key);

private:
  vector<unordered_map<int, wstring>> vocab_hash_table;
  const string vocab_path;
  const int alphabet_size;
};
