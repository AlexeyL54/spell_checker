#include "vocab.hpp"
#include <cmath>
#include <cstddef>
#include <cstdio>
#include <cwchar>
#include <exception>
#include <fstream>
#include <math.h>
#include <string>
#include <vector>

using std::getline;
using std::ifstream;
using std::pow;
using std::vector;
using std::wstring;

/**
 * @brief Конструктор
 *
 * Инициализировать
 * поле пути к файлу словаря;
 * размер алфавита словаря и проверяемого текста
 */
Vocabulary::Vocabulary(const string &path)
    : vocab_path(path), alphabet_size(WCHAR_MAX) {}

/**
 * @brief Вычислить количество строк в файле
 * @param file ссылка на потокoк
 * @return количество строк в файле
 */
size_t rowsTotal(ifstream &file) {
  size_t rows = 0;
  string line;

  while (getline(file, line)) {
    rows++;
  }
  return rows;
}

/**
 * @brief Конвертировать string в wstring
 * @param str ссылка на конвертируемую строку
 * @return строку типа wstring
 */
std::wstring stringToWstring(const std::string &str) {
  size_t len = str.length();
  std::wstring wstr(len, L'\0');
  std::mbstowcs(&wstr[0], str.c_str(), len);
  return wstr;
}

/**
 * @brief Загрузить словарь и представить его в виде хэш-таблицы
 */
void Vocabulary::loadVocab() {
  string line;
  wstring wline;
  int wline_hash;

  ifstream file(vocab_path);
  if (!file.is_open()) {
    file.close();
    return;
  }

  size_t lines_total = Vocabulary::rowsTotal(file);
  Vocabulary::vocab_hash_table.resize(lines_total);

  while (getline(file, line)) {
    wline = stringToWstring(line);
    wline_hash = createHashCode(wline);
    Vocabulary::vocab_hash_table[wline.length()][wline_hash] = wline;
  }

  file.close();
}

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

/**
 * @brief Проверить наличие строки в словаре по хэш-коду
 * @param key хэш-код проверяемой строки
 * @return true, если строка есть в словаре, иначе false
 */
bool Vocabulary::isInVocab(int key) {
  bool found = false;

  try {
    auto str = Vocabulary::vocab_hash_table.at(key);

  } catch (const std::exception &e) {
    found = false;
  }
  return found;
}
