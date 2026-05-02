#include "vocab.hpp"
#include "unistring.hpp"
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cwchar>
#include <fstream>
#include <iostream>
#include <math.h>
#include <string>

using std::getline;
using std::ifstream;
using std::wstring;
using utf8::Unistring;

/**
 * @brief Конструктор
 *
 * Инициализировать
 * поле пути к файлу словаря;
 * размер алфавита словаря и проверяемого текста
 */
Vocabulary::Vocabulary(const string &path) : vocab_path(path) {}

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
 * @brief Вычислить максимальную длину строк
 * @param file ссылка на потокoк
 * @return максимальную длину строк
 */
size_t Vocabulary::maxRowLength(ifstream &file) {
  size_t maxLen = 0;
  std::string line;
  Unistring uline;

  while (getline(file, line)) {
    if ((uline = Unistring(line)).length() > maxLen)
      maxLen = uline.length();
  }

  file.clear();
  file.seekg(0, std::ios::beg);

  return maxLen;
}

/**
 * @brief Загрузить словарь и представить его в виде хэш-таблицы
 */
void Vocabulary::loadVocab() {
  string line;
  Unistring uline;
  size_t wline_hash;

  ifstream file(vocab_path);
  if (!file.is_open()) {
    file.close();
    return;
  }

  size_t dif_len_total = Vocabulary::maxRowLength(file);
  Vocabulary::vocab_hash_table.resize(dif_len_total + 1);

  while (getline(file, line)) {
    uline = line;
    wline_hash = createHashCode(uline);
    Vocabulary::vocab_hash_table[uline.length()][wline_hash] = uline;
  }

  file.close();
}

/**
 * @brief Создать хэш код для строки (DJB2)
 * @param str ссылка на строку
 * @return хэш код
 */
size_t Vocabulary::createHashCode(const Unistring &str) {
  // Простая, но эффективная хэш-функция
  size_t hash = 5381;
  int length = str.length();

  for (int i = 0; i < length; i++) {
    hash = ((hash << 5) + hash) + utf8::unichar_to_int(str[i]); // hash * 33 + c
  }

  return hash;
}

/**
 * @brief Получить копию хэш-таблицы словаря
 * @return хэш-таблицу словаря
 */
vector<unordered_map<size_t, Unistring>> Vocabulary::getVocabHashTable() {
  return Vocabulary::vocab_hash_table;
}

/**
 * @brief Проверить наличие строки в словаре по хэш-коду
 * @param key хэш-код проверяемой строки
 * @return true, если строка есть в словаре, иначе false
 */
bool Vocabulary::isInVocab(const Unistring &str) {
  size_t key = createHashCode(str);
  size_t index = str.length();

  if (index < Vocabulary::vocab_hash_table.size()) {
    auto it = Vocabulary::vocab_hash_table[index].find(key);
    if (it != Vocabulary::vocab_hash_table[index].end()) {
      // Verify it's actually the same string (handle hash collisions)
      return it->second == str;
    }
  }

  return false;
}

/**
 * @brief Проверить слово на орфографию
 * @param word слово для проверки
 * @return вектор ближайших слов к проверяемому по редакционному расстоянию
 */
std::vector<Unistring> Vocabulary::checkWordSpelling(const Unistring &word) {
  const uint16_t maxCorrections = 5;
  std::vector<Unistring> corrections;

  if (isInVocab(word))
    return corrections;

  // Перебираем слова с длиной от max(1, len-2) до len+2
  size_t wordLen = word.length();
  size_t startLen = (wordLen > 2) ? wordLen - 2 : 1;
  size_t endLen = std::min(wordLen + 2, vocab_hash_table.size() - 1);

  // Store pairs of (distance, word)
  std::vector<std::pair<uint32_t, Unistring>> candidates;

  for (size_t currentLen = startLen; currentLen <= endLen; currentLen++) {
    for (auto &p : vocab_hash_table[currentLen]) {
      uint32_t dist = getLevensteinDistance(word, p.second);
      if (dist <= MAXLEVENSTEINDIST) {
        candidates.push_back({dist, p.second});
      }
    }
  }

  // Сортируем по расстоянию Левенштейна
  std::sort(candidates.begin(), candidates.end(),
            [](const std::pair<uint32_t, Unistring> &a,
               const std::pair<uint32_t, Unistring> &b) {
              return a.first < b.first;
            });

  // Extract just the words in order
  for (const auto &candidate : candidates) {
    corrections.push_back(candidate.second);
    if (corrections.size() >= maxCorrections) {
      break;
    }
  }

  return corrections;
}

/**
 * @brief Рассчитать расстояние Левенштейна
 * @param word1 первое слово
 * @param word2 второе слово
 * @return расстояние Левенштейна
 */
uint32_t Vocabulary::getLevensteinDistance(const Unistring &word1,
                                           const Unistring &word2) {
  const size_t m = word1.length();
  const size_t n = word2.length();

  // Быстрая проверка по длине
  const int length_diff = static_cast<int>(m) - static_cast<int>(n);
  if (std::abs(length_diff) > MAXLEVENSTEINDIST) {
    return MAXLEVENSTEINDIST + 1;
  }

  // Оптимизируем порядок (меньшая строка - первая)
  if (m > n) {
    return getLevensteinDistance(word2, word1);
  }

  // Используем только два ряда
  std::vector<uint32_t> prev(n + 1), curr(n + 1);
  for (size_t j = 0; j <= n; ++j)
    prev[j] = j;

  for (size_t i = 1; i <= m; ++i) {
    curr[0] = i;
    uint32_t min_in_row = curr[0];

    // Оптимизация: вычисляем только необходимый диапазон
    size_t j_start = (i > MAXLEVENSTEINDIST) ? i - MAXLEVENSTEINDIST : 1;
    size_t j_end = std::min(n, i + MAXLEVENSTEINDIST);

    // Заполняем границы большими значениями
    for (size_t j = 1; j < j_start && j <= n; ++j) {
      curr[j] = MAXLEVENSTEINDIST + 1;
    }

    for (size_t j = j_start; j <= j_end; ++j) {
      const uint32_t cost = (word1[i - 1] != word2[j - 1]) ? 1 : 0;
      curr[j] = std::min({curr[j - 1] + 1, prev[j] + 1, prev[j - 1] + cost});
      if (curr[j] < min_in_row)
        min_in_row = curr[j];
    }

    for (size_t j = j_end + 1; j <= n; ++j) {
      curr[j] = MAXLEVENSTEINDIST + 1;
    }

    if (min_in_row > MAXLEVENSTEINDIST) {
      return min_in_row;
    }

    std::swap(prev, curr);
  }

  return prev[n];
}
