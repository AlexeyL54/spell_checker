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
#include <unordered_set>

using std::getline;
using std::ifstream;
using std::pair;
using std::string;
using std::unordered_set;
using utf8::Unistring;

Vocabulary::Vocabulary(const string &path) : vocab_path(path) {}

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

void Vocabulary::loadVocab() {
  string line;
  Unistring uline;
  size_t wline_hash;

  ifstream file(vocab_path);
  if (!file.is_open()) {
    file.close();
    return;
  }

  vocab_hash_table.clear();
  vocab_words.clear();
  trigram_index.clear();

  size_t dif_len_total = maxRowLength(file);
  vocab_hash_table.resize(dif_len_total + 1);

  while (getline(file, line)) {
    uline = Unistring(line);
    wline_hash = createHashCode(uline);

    vocab_hash_table[uline.length()][wline_hash] = uline;
    vocab_words.push_back(uline);
  }

  file.close();

  buildTrigramIndex();
}

size_t Vocabulary::createHashCode(const Unistring &str) {
  size_t hash = 5381;
  int length = str.length();

  for (int i = 0; i < length; i++) {
    hash = ((hash << 5) + hash) + utf8::unichar_to_int(str[i]);
  }

  return hash;
}

vector<unordered_map<size_t, Unistring>> Vocabulary::getVocabHashTable() {
  return vocab_hash_table;
}

bool Vocabulary::isInVocab(const Unistring &str) {
  size_t key = createHashCode(str);
  size_t index = str.length();

  if (index < vocab_hash_table.size()) {
    auto it = vocab_hash_table[index].find(key);
    if (it != vocab_hash_table[index].end()) {
      return it->second == str;
    }
  }

  return false;
}

void Vocabulary::buildTrigramIndex() {
  for (size_t idx = 0; idx < vocab_words.size(); ++idx) {
    const auto &word = vocab_words[idx];
    auto trigrams = extractTrigrams(word);

    // Удаляем дубликаты триграмм для одного слова
    unordered_set<uint64_t> unique_trigrams(trigrams.begin(), trigrams.end());

    for (uint64_t trigram : unique_trigrams) {
      trigram_index[trigram].push_back(idx);
    }
  }
}

uint64_t Vocabulary::hashTrigram(int c1, int c2, int c3) {
  // Используем 64-битный хэш для избежания коллизий
  // Комбинируем три 32-битных кода символов в один 64-битный хэш
  uint64_t hash = 0;
  hash = (hash << 21) | (static_cast<uint64_t>(c1) & 0x1FFFFF);
  hash = (hash << 21) | (static_cast<uint64_t>(c2) & 0x1FFFFF);
  hash = (hash << 21) | (static_cast<uint64_t>(c3) & 0x1FFFFF);
  return hash;
}

vector<uint64_t> Vocabulary::extractTrigrams(const Unistring &word) {
  vector<uint64_t> trigrams;
  size_t len = word.length();

  if (len == 0) {
    return trigrams;
  }

  // Получаем коды символов
  vector<int> char_codes;
  for (size_t i = 0; i < len; ++i) {
    char_codes.push_back(utf8::unichar_to_int(word[i]));
  }

  // Для односимвольных слов
  if (len == 1) {
    // Маркеры: #С#
    uint64_t trigram = hashTrigram('#', char_codes[0], '#');
    trigrams.push_back(trigram);
    return trigrams;
  }

  // Для двухсимвольных слов
  if (len == 2) {
    // Маркеры начала: #СС
    uint64_t trigram1 = hashTrigram('#', char_codes[0], char_codes[1]);
    trigrams.push_back(trigram1);

    // Маркеры конца: СС#
    uint64_t trigram2 = hashTrigram(char_codes[0], char_codes[1], '#');
    trigrams.push_back(trigram2);

    return trigrams;
  }

  // Для слов длиной 3 и более - извлекаем все перекрывающиеся триграммы
  for (size_t i = 0; i <= len - 3; ++i) {
    uint64_t trigram =
        hashTrigram(char_codes[i], char_codes[i + 1], char_codes[i + 2]);
    trigrams.push_back(trigram);
  }

  // Добавляем начальную триграмму с маркером начала
  uint64_t start_trigram = hashTrigram('#', char_codes[0], char_codes[1]);
  trigrams.push_back(start_trigram);

  // Добавляем конечную триграмму с маркером конца
  uint64_t end_trigram =
      hashTrigram(char_codes[len - 2], char_codes[len - 1], '#');
  trigrams.push_back(end_trigram);

  // Для слов длины 3 также добавляем триграммы с маркерами для лучшего поиска
  if (len == 3) {
    uint64_t full_start = hashTrigram('#', char_codes[0], char_codes[1]);
    uint64_t full_end = hashTrigram(char_codes[1], char_codes[2], '#');
    trigrams.push_back(full_start);
    trigrams.push_back(full_end);
  }

  return trigrams;
}

uint32_t Vocabulary::getLevensteinDistance(const Unistring &word1,
                                           const Unistring &word2) {
  size_t m = word1.length();
  size_t n = word2.length();

  int length_diff = static_cast<int>(m) - static_cast<int>(n);
  if (std::abs(length_diff) > static_cast<int>(MAXLEVENSTEINDIST)) {
    return MAXLEVENSTEINDIST + 1;
  }

  if (m > n) {
    return getLevensteinDistance(word2, word1);
  }

  std::vector<uint32_t> prev(n + 1, 0);
  std::vector<uint32_t> curr(n + 1, 0);

  for (size_t j = 0; j <= n; ++j) {
    prev[j] = j;
  }

  for (size_t i = 1; i <= m; ++i) {
    curr[0] = i;
    uint32_t min_in_row = curr[0];

    size_t j_start = (i > MAXLEVENSTEINDIST) ? i - MAXLEVENSTEINDIST : 1;
    size_t j_end = std::min(n, i + MAXLEVENSTEINDIST);

    for (size_t j = 1; j < j_start && j <= n; ++j) {
      curr[j] = MAXLEVENSTEINDIST + 1;
    }

    for (size_t j = j_start; j <= j_end; ++j) {
      uint32_t cost = (word1[i - 1] != word2[j - 1]) ? 1 : 0;

      curr[j] = std::min({curr[j - 1] + 1, prev[j] + 1, prev[j - 1] + cost});

      if (curr[j] < min_in_row) {
        min_in_row = curr[j];
      }
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

std::vector<Unistring> Vocabulary::checkWordSpelling(const Unistring &word) {
  std::vector<Unistring> corrections;

  if (isInVocab(word)) {
    return corrections;
  }

  if (word.length() == 0) {
    return corrections;
  }

  // Приводим слово к нижнему регистру для поиска
  Unistring lower_word = word.to_lower();
  auto word_trigrams = extractTrigrams(lower_word);

  if (word_trigrams.empty()) {
    return corrections;
  }

  // Собираем кандидатов на основе совпадения триграмм
  unordered_map<size_t, uint8_t> candidate_scores;

  for (uint64_t trigram : word_trigrams) {
    auto it = trigram_index.find(trigram);
    if (it != trigram_index.end()) {
      for (size_t idx : it->second) {
        candidate_scores[idx]++;
      }
    }
  }

  if (candidate_scores.empty()) {
    return corrections;
  }

  // Отбираем кандидатов с достаточным количеством совпадений
  std::vector<std::pair<size_t, uint8_t>> candidates;
  candidates.reserve(candidate_scores.size());

  for (const auto &[idx, score] : candidate_scores) {
    if (score >= MIN_TRIGRAM_MATCHES) {
      candidates.emplace_back(idx, score);
    }
  }

  // Сортируем по убыванию score (чем больше совпадений триграмм, тем лучше)
  std::sort(candidates.begin(), candidates.end(),
            [](const auto &a, const auto &b) { return a.second > b.second; });

  // Берём только топ кандидатов для вычисления расстояния Левенштейна
  size_t num_candidates =
      std::min(candidates.size(), static_cast<size_t>(MAX_CANDIDATES));

  // Вычисляем расстояние Левенштейна для топ-кандидатов
  std::vector<std::pair<uint32_t, Unistring>> distance_results;
  distance_results.reserve(num_candidates);

  for (size_t i = 0; i < num_candidates; ++i) {
    size_t idx = candidates[i].first;
    uint32_t dist = getLevensteinDistance(lower_word, vocab_words[idx]);

    if (dist <= MAXLEVENSTEINDIST) {
      distance_results.emplace_back(dist, vocab_words[idx]);
    }
  }

  // Сортируем по расстоянию Левенштейна
  std::sort(distance_results.begin(), distance_results.end(),
            [](const auto &a, const auto &b) {
              if (a.first != b.first) {
                return a.first < b.first;
              }
              return a.second.length() < b.second.length();
            });

  const uint16_t maxCorrections = 5;
  for (size_t i = 0; i < std::min(distance_results.size(),
                                  static_cast<size_t>(maxCorrections));
       ++i) {
    corrections.push_back(distance_results[i].second);
  }

  return corrections;
}
