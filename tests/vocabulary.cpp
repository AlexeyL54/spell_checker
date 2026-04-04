#include "../src/back/unistring.hpp"
#include "../src/back/vocab.hpp"
#include <climits>
#include <cstddef>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <math.h>
#include <string>

using std::cerr;
using std::endl;
using std::string;
using std::wofstream;
using std::filesystem::remove;
using utf8::Unistring;

const string test_vocab_path = "test_vocab.txt";
const vector<Unistring> words = {
    "привет",     "мир",      "программа", "тестирование", "словарь",
    "русский",    "язык",     "проверка",  "орфография",   "компьютер",
    "разработка", "алгоритм", "структура", "данные",       "функция",
    "переменная", "класс",    "объект",    "метод",        "наследование",
};

/**
 * @brief Создать хэш код для строки
 * @param str ссылка на строку
 * @return хэш код
 */
int createHashCode(const Unistring &str) {
  int code = 0;
  int length = str.length();

  for (int i = 0; i < length; i++) {
    code += pow(INT_MAX, length - (i + 1)) * utf8::unichar_to_int(str[i]);
  }

  return code;
}

/**
 * @brief Создать тестовую хэш-таблицу
 * @return хэш-таблицу, гду i-ый элемент вектора - слоаварь, в котором значение
 * - строки с одинаковой длиной равной индексу словаря в векторе
 */
vector<unordered_map<int, Unistring>> createTestHashTable() {
  vector<unordered_map<int, Unistring>> test_hash_table(words.size());
  size_t index;
  int key;

  for (Unistring str : words) {
    index = str.length();
    key = createHashCode(str);
    test_hash_table[index][key] = str;
  }

  return test_hash_table;
}

/**
 * @brief Подготовить тестовый словарь
 * @return хэш-таблицу, гду i-ый элемент вектора - слоаварь, в котором значение
 * - строки с одинаковой длиной равной индексу словаря в векторе
 */
void prepareTestVocab() {
  // Используем обычный ofstream в бинарном режиме
  std::ofstream file(test_vocab_path, std::ios::out);
  if (!file.is_open()) {
    cerr << "Unable to open file: " << test_vocab_path << endl;
    return;
  }

  for (const Unistring &word : words) {
    file << word.to_string() << "\n";
  }

  file.close();
}

// =========================================================================================

/**
 * @brief Тест создания хэш-таблицы из файла словаря
 */
TEST(VOCABULARY, loadVocab) {
  prepareTestVocab();

  Vocabulary vocab = Vocabulary(test_vocab_path);
  vocab.loadVocab();

  vector<unordered_map<int, Unistring>> table = vocab.getVocabHashTable();
  vector<unordered_map<int, Unistring>> test_table = createTestHashTable();
  EXPECT_EQ(table, test_table);

  remove(test_vocab_path);
}
