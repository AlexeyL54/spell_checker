#include "vocab.hpp"
#include <cmath>
#include <math.h>
#include <string>
#include <vector>

using std::pow;
using std::vector;
using std::wstring;

Vocabulary::Vocabulary(const string &path)
    : vocab_path(path), alphabet_size(WCHAR_MAX) {}

/**
 * @brief Создать хэш код для строки
 * @param str ссылка на строку
 * @return хэш код
 */
int Vocabulary::createHashCode(const wstring &str) {
  int code = 0;
  int length = str.length();

  for (int i = 0; i < length; i++) {
    code += pow(alphabet_size, length - (i + 1)) * ((int)str[i]);
  }

  return code;
}
