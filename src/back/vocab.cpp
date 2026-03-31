#include "vocab.hpp"
#include <cmath>
#include <cstddef>
#include <cstdio>
#include <cwchar>
#include <fstream>
#include <math.h>
#include <string>

using std::getline;
using std::ifstream;
using std::pow;
using std::wstring;

// TODO: Vocabulary::vocab_hash_table.resize(кол-во длин строк в файле)
// TODO: индекс Vocabulary::vocab_hash_table = длина строки - 1
// TODO: переписать с wstring на string или свой тип для utf8

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
size_t Vocabulary::rowsTotal(ifstream &file) {
  size_t rows = 0;
  string line;

  while (getline(file, line)) {
    rows++;
  }

  file.clear();
  file.seekg(0, std::ios::beg);

  return rows;
}

/**
 * @brief Конвертировать string в wstring
 * @param str ссылка на конвертируемую строку
 * @return строку типа wstring
 */
std::wstring Vocabulary::stringToWstring(const std::string &str) {
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
 * @brief Получить копию хэш-таблицы словаря
 * @return хэш-таблицу словаря
 */
vector<unordered_map<int, wstring>> Vocabulary::getVocabHashTable() {
  return Vocabulary::vocab_hash_table;
}

/**
 * @brief Проверить наличие строки в словаре по хэш-коду
 * @param key хэш-код проверяемой строки
 * @return true, если строка есть в словаре, иначе false
 */
bool Vocabulary::isInVocab(const wstring &str) {
  int key = createHashCode(str);
  size_t index = str.length();

  if (index < Vocabulary::vocab_hash_table.size()) {

    if (Vocabulary::vocab_hash_table[index].find(key) !=
        Vocabulary::vocab_hash_table[index].end()) {
      return true;
    }
  }

  return false;
}
