#pragma once

#include <string>
#include <vector>

#include "vocab.hpp"

class Processor {

  std::pair<std::wstring, std::vector<std::wstring>>
  checkTextSpelling(const std::wstring &word);

  std::vector<std::wstring> checkWordSpelling(const std::wstring &word);

private:
  Vocabulary *vocab;
  int getLevensteinDistance(const std::wstring &word1,
                            const std::wstring &word2);
};
