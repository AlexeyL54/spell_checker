#include "../src/back/vocab.hpp"
#include "qhashfunctions.h"

#include <chrono>
#include <cstddef>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <iostream>
#include <math.h>
#include <string>

#include <QApplication>
#include <QCoreApplication>
#include <QString>
#include <QtTest/QSignalSpy>

using std::cerr;
using std::endl;
using std::string;
using std::wofstream;
using std::filesystem::remove;

const QString TEST_VOCAB_PATH = "test_vocab.txt";
const QString REAL_DICT_PATH = "../../vocab/russian-words/russian.txt";
const vector<QString> words = {
    "привет",     "мир",      "программа", "тестирование", "словарь",
    "русский",    "язык",     "проверка",  "орфография",   "компьютер",
    "разработка", "алгоритм", "структура", "данные",       "функция",
    "переменная", "класс",    "объект",    "метод",        "наследование",
};

static QApplication *testApp = nullptr;

int main(int argc, char **argv) {
  // Создаём QApplication для тестов
  QApplication app(argc, argv);
  testApp = &app;

  ::testing::InitGoogleTest(&argc, argv);
  int result = RUN_ALL_TESTS();

  return result;
}

/**
 * @brief Найти максимальную длину слов вектора words
 * @return максимальную длину слов вектора words
 */
size_t maxWordLen() {
  size_t maxLen = 0;

  for (const QString &word : words) {
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
QVector<QHash<size_t, QString>> createTestHashTable(Vocabulary &vocab) {
  QVector<QHash<size_t, QString>> test_hash_table(maxWordLen() + 1);
  size_t index;
  size_t key;

  for (QString str : words) {
    index = str.length();
    key = vocab.createHashCode(str);
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
  const std::string path = TEST_VOCAB_PATH.toUtf8().toStdString();
  std::ofstream file(path, std::ios::out);
  if (!file.is_open()) {
    cerr << "Unable to open file: " << path << endl;
    return;
  }

  for (const QString &word : words) {
    file << word.toUtf8().toStdString() << "\n";
  }

  file.close();
}

// =========================================================================================

/**
 * @brief Тест создания хэш-таблицы из файла словаря
 */
TEST(VOCABULARY, loadVocabAsync) {
  prepareTestVocab();

  Vocabulary vocab = Vocabulary(TEST_VOCAB_PATH);

  QSignalSpy finishedSpy(&vocab, &Vocabulary::loadFinished);
  QSignalSpy errorSpy(&vocab, &Vocabulary::loadError);
  vocab.loadVocabAsync();

  bool finished = finishedSpy.wait(5000);

  ASSERT_TRUE(finished) << "loadFinished not received";
  ASSERT_TRUE(errorSpy.isEmpty())
      << "Error: "
      << (errorSpy.isEmpty() ? ""
                             : errorSpy.at(0).at(0).toString().toStdString());

  QVector<QHash<size_t, QString>> table = vocab.getVocabHashTable();
  QVector<QHash<size_t, QString>> test_table = createTestHashTable(vocab);
  EXPECT_EQ(table, test_table);

  remove(TEST_VOCAB_PATH.toUtf8().toStdString());
}

TEST(VOCABULARY, isInVocab) {
  prepareTestVocab();

  Vocabulary vocab = Vocabulary(TEST_VOCAB_PATH);

  QSignalSpy finishedSpy(&vocab, &Vocabulary::loadFinished);
  QSignalSpy errorSpy(&vocab, &Vocabulary::loadError);
  vocab.loadVocabAsync();

  bool finished = finishedSpy.wait(60000);

  ASSERT_TRUE(finished) << "loadFinished not received";
  ASSERT_TRUE(errorSpy.isEmpty())
      << "Error: "
      << (errorSpy.isEmpty() ? ""
                             : errorSpy.at(0).at(0).toString().toStdString());

  EXPECT_EQ(vocab.isInVocab("класс"), true);
  EXPECT_EQ(vocab.isInVocab("клас"), false);
  EXPECT_EQ(vocab.isInVocab("наследование"), true);
  EXPECT_EQ(vocab.isInVocab("привет"), true);
}

TEST(VOCABULARY, checkWordSpelling) {
  prepareTestVocab();

  Vocabulary vocab = Vocabulary(TEST_VOCAB_PATH);
  vocab.loadVocab();

  QVector<QString> corrections1 = vocab.checkWordSpelling("прграмма");
  ASSERT_FALSE(corrections1.empty());
  EXPECT_EQ(corrections1[0], "программа");

  QVector<QString> corrections2 = vocab.checkWordSpelling("приверка");
  ASSERT_FALSE(corrections2.empty());
  EXPECT_EQ(corrections2[0], "проверка");

  QVector<QString> corrections3 = vocab.checkWordSpelling("приветы");
  ASSERT_FALSE(corrections3.empty());
  EXPECT_EQ(corrections3[0], "привет");

  QVector<QString> corrections4 = vocab.checkWordSpelling("класс");
  EXPECT_TRUE(corrections4.empty());
}

TEST(VOCABULARY, realPerformanceBenchmark) {
  const std::string path = REAL_DICT_PATH.toUtf8().toStdString();
  std::ifstream check(path);
  if (!check.is_open()) {
    std::cout << "Dictionary not found at: " << path << std::endl;
    std::cout << "Skipping performance test" << std::endl;
    return;
  }
  check.close();

  // Подсчитываем количество строк в словаре
  size_t word_count = 0;
  std::string line;
  std::ifstream count_file(path);
  while (std::getline(count_file, line)) {
    word_count++;
  }
  count_file.close();

  std::cout << "\n=== Performance Test ===" << std::endl;
  std::cout << "Dictionary size: " << word_count << " words" << std::endl;

  std::chrono::time_point start = std::chrono::high_resolution_clock::now();

  Vocabulary vocab = Vocabulary(REAL_DICT_PATH);
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
