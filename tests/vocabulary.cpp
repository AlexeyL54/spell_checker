#include "../src/back/unistring.hpp"
#include "../src/back/vocab.hpp"
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
size_t createHashCode(const Unistring &str) {
  size_t hash = 5381;
  int length = str.length();

  for (int i = 0; i < length; i++) {
    hash = ((hash << 5) + hash) + utf8::unichar_to_int(str[i]);
  }

  return hash;
}

/**
 * @brief Найти максимальную длину слов вектора words
 * @return максимальную длину слов вектора words
 */
size_t maxWordLen() {
  size_t maxLen = 0;

  for (const Unistring &word : words) {
    if (word.length() > maxLen)
      maxLen = word.length();
  }
  return maxLen;
}

/**
 * @brief Создать тестовую хэш-таблицу
 * @return хэш-таблицу, гду i-ый элемент вектора - слоаварь, в котором значение
 * - строки с одинаковой длиной равной индексу словаря в векторе
 */
vector<unordered_map<size_t, Unistring>> createTestHashTable() {
  vector<unordered_map<size_t, Unistring>> test_hash_table(maxWordLen() + 1);
  size_t index;
  size_t key;

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

  vector<unordered_map<size_t, Unistring>> table = vocab.getVocabHashTable();
  vector<unordered_map<size_t, Unistring>> test_table = createTestHashTable();
  EXPECT_EQ(table, test_table);

  remove(test_vocab_path);
}

TEST(VOCABULARY, isInVocab) {
  prepareTestVocab();

  Vocabulary vocab = Vocabulary(test_vocab_path);
  vocab.loadVocab();

  EXPECT_EQ(vocab.isInVocab("класс"), true);
  EXPECT_EQ(vocab.isInVocab("клас"), false);
  EXPECT_EQ(vocab.isInVocab("наследование"), true);
  EXPECT_EQ(vocab.isInVocab("привет"), true);
}

TEST(VOCABULARY, checkWordSpelling) {
  prepareTestVocab();

  Vocabulary vocab = Vocabulary(test_vocab_path);
  vocab.loadVocab();

  auto corrections1 = vocab.checkWordSpelling("прграмма");
  ASSERT_FALSE(corrections1.empty());
  EXPECT_EQ(corrections1[0], "программа");

  auto corrections2 = vocab.checkWordSpelling("приверка");
  ASSERT_FALSE(corrections2.empty());
  EXPECT_EQ(corrections2[0], "проверка");

  auto corrections3 = vocab.checkWordSpelling("приветы");
  ASSERT_FALSE(corrections3.empty());
  EXPECT_EQ(corrections3[0], "привет");

  auto corrections4 = vocab.checkWordSpelling("класс");
  EXPECT_TRUE(corrections4.empty());
}
