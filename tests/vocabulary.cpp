#include "../src/back/unistring.hpp"
#include "../src/back/vocab.hpp"
#include <cstddef>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <math.h>
#include <string>

#include <chrono>
#include <iostream>

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
/*size_t createHashCode(const Unistring &str) {
  size_t hash = 5381;
  int length = str.length();

  for (int i = 0; i < length; i++) {
    hash = ((hash << 5) + hash) + utf8::unichar_to_int(str[i]);
  }

  return hash;
}*/

size_t createHashCode(const Unistring &str) {
  size_t hash = 5381;
  const string &bytes = str.to_string();
  for (unsigned char c : bytes) {
    hash = ((hash << 5) + hash) + c;
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

TEST(VOCABULARY, realPerformanceBenchmark) {
  // Замените на путь к вашему реальному словарю
  const string real_dict_path = "../../vocab/russian-words/russian.txt";

  // Проверяем, существует ли файл
  std::ifstream check(real_dict_path);
  if (!check.is_open()) {
    std::cout << "Dictionary not found at: " << real_dict_path << std::endl;
    std::cout << "Skipping performance test" << std::endl;
    return;
  }
  check.close();

  // Подсчитываем количество строк в словаре
  size_t word_count = 0;
  std::string line;
  std::ifstream count_file(real_dict_path);
  while (std::getline(count_file, line)) {
    word_count++;
  }
  count_file.close();

  std::cout << "\n=== Performance Test ===" << std::endl;
  std::cout << "Dictionary size: " << word_count << " words" << std::endl;

  auto start = std::chrono::high_resolution_clock::now();

  Vocabulary vocab(real_dict_path);
  vocab.loadVocab();

  std::chrono::time_point end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<long, std::ratio<1, 1000>> duration =
      std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

  std::cout << "Load time: " << duration.count() << " ms ("
            << (duration.count() / 1000.0) << " seconds)" << std::endl;
  std::cout << "Speed: " << (word_count * 1000.0 / duration.count())
            << " words/second" << std::endl;
  std::cout << "========================\n" << std::endl;
}
