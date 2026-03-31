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

const string test_vocab_path = "test_vocab.txt";
const vector<wstring> words = {
    L"привет",     L"мир",      L"программа", L"тестирование", L"словарь",
    L"русский",    L"язык",     L"проверка",  L"орфография",   L"компьютер",
    L"разработка", L"алгоритм", L"структура", L"данные",       L"функция",
    L"переменная", L"класс",    L"объект",    L"метод",        L"наследование",
};

/**
 * @brief Создать хэш код для строки
 * @param str ссылка на строку
 * @return хэш код
 */
int createHashCode(const wstring &str) {
  int code = 0;
  int length = str.length();

  for (int i = 0; i < length; i++) {
    code += pow(WCHAR_MAX, length - (i + 1)) * ((int)str[i]);
  }

  return code;
}

/**
 * @brief Создать тестовую хэш-таблицу
 * @return хэш-таблицу, гду i-ый элемент вектора - слоаварь, в котором значение
 * - строки с одинаковой длиной равной индексу словаря в векторе
 */
vector<unordered_map<int, wstring>> createTestHashTable() {
  vector<unordered_map<int, wstring>> test_hash_table(words.size());
  size_t index;
  int key;

  for (wstring str : words) {
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
  std::ofstream file(test_vocab_path, std::ios::binary);
  if (!file.is_open()) {
    cerr << "Unable to open file: " << test_vocab_path << endl;
    return;
  }

  // Записываем BOM для UTF-8 (опционально, помогает некоторым программам)
  // const unsigned char bom[] = {0xEF, 0xBB, 0xBF};
  // file.write(reinterpret_cast<const char*>(bom), sizeof(bom));

  for (const wstring &str : words) {
    // Конвертируем wstring в UTF-8 вручную
    std::string utf8_str;
    for (wchar_t ch : str) {
      if (ch < 0x80) {
        // ASCII символы
        utf8_str += static_cast<char>(ch);
      } else if (ch < 0x800) {
        // 2-байтовые UTF-8 символы
        utf8_str += static_cast<char>(0xC0 | (ch >> 6));
        utf8_str += static_cast<char>(0x80 | (ch & 0x3F));
      } else if (ch < 0x10000) {
        // 3-байтовые UTF-8 символы
        utf8_str += static_cast<char>(0xE0 | (ch >> 12));
        utf8_str += static_cast<char>(0x80 | ((ch >> 6) & 0x3F));
        utf8_str += static_cast<char>(0x80 | (ch & 0x3F));
      } else {
        // 4-байтовые UTF-8 символы (для суррогатных пар)
        utf8_str += static_cast<char>(0xF0 | (ch >> 18));
        utf8_str += static_cast<char>(0x80 | ((ch >> 12) & 0x3F));
        utf8_str += static_cast<char>(0x80 | ((ch >> 6) & 0x3F));
        utf8_str += static_cast<char>(0x80 | (ch & 0x3F));
      }
    }
    file << utf8_str << "\n";
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

  vector<unordered_map<int, wstring>> table = vocab.getVocabHashTable();
  vector<unordered_map<int, wstring>> test_table = createTestHashTable();

  EXPECT_EQ(table, test_table);

  remove(test_vocab_path);
}
