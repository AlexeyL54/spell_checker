#include "vocab.hpp"
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <unordered_set>

#include <QDebug>
#include <QElapsedTimer>
#include <QTextStream>
#include <QThread>

using std::getline;
using std::ifstream;
using std::pair;
using std::string;
using std::unordered_set;

Vocabulary::Vocabulary(const QString &path, QObject *parent)
    : QObject(parent), vocab_path(path) {}

size_t Vocabulary::maxRowLength(ifstream &file) {
  size_t maxLen = 0;
  std::string line;

  while (getline(file, line)) {
    QString qline = QString::fromUtf8(line.c_str());
    if ((size_t)qline.length() > maxLen)
      maxLen = qline.length();
  }

  file.clear();
  file.seekg(0, std::ios::beg);

  return maxLen;
}

void Vocabulary::loadVocabAsync() {
  QThread *thread = QThread::create([this]() {
    emit loadStarted();

    QElapsedTimer timer;
    timer.start();

    string line;
    QString qline;
    size_t wline_hash;

    ifstream file(vocab_path.toStdString());
    if (!file.is_open()) {
      emit loadError(QString("Не удалось открыть файл словаря"));
      return;
    }

    vocab_hash_table.clear();
    vocab_words.clear();
    trigram_index.clear();

    vocab_hash_table.reserve(50);

    file.seekg(0, std::ios::end);
    size_t file_size = file.tellg();
    file.seekg(0, std::ios::beg);

    size_t estimated_lines = file_size / 8;
    vocab_words.reserve(estimated_lines);

    int wordCount = 0;

    while (getline(file, line)) {
      if (line.empty())
        continue;

      qline = QString::fromUtf8(line.c_str());
      wline_hash = createHashCode(qline);

      size_t len = qline.length();
      if (len >= (size_t)vocab_hash_table.size()) {
        vocab_hash_table.resize(len + 1);
      }

      vocab_hash_table[len][wline_hash] = qline;
      vocab_words.append(qline);

      wordCount++;
    }

    file.close();
    buildTrigramIndex();

    double elapsed = timer.elapsed() / 1000.0;
    qDebug() << "Dictionary loaded:" << wordCount << "words in" << elapsed
             << "seconds";

    isLoaded_ = true;
    emit loadProgress(wordCount);
    emit loadFinished();
  });

  connect(thread, &QThread::finished, thread, &QThread::deleteLater);
  thread->start();
}

size_t Vocabulary::createHashCode(const QString &str) {
  size_t hash = 5381;
  int length = str.length();

  for (int i = 0; i < length; i++) {
    hash = ((hash << 5) + hash) + str[i].unicode();
  }

  return hash;
}

QVector<QHash<size_t, QString>> Vocabulary::getVocabHashTable() {
  return vocab_hash_table;
}

bool Vocabulary::isInVocab(const QString &str) {
  size_t key = createHashCode(str);
  size_t index = str.length();

  if (index < (size_t)vocab_hash_table.size()) {
    auto it = vocab_hash_table[index].find(key);
    if (it != vocab_hash_table[index].end()) {
      return it.value() == str;
    }
  }

  return false;
}

void Vocabulary::buildTrigramIndex() {
  for (int idx = 0; idx < vocab_words.size(); ++idx) {
    const QString &word = vocab_words[idx];
    QVector<uint64_t> trigrams = extractTrigrams(word);

    // Удаляем дубликаты триграмм для одного слова
    unordered_set<uint64_t> unique_trigrams;
    for (uint64_t trigram : trigrams) {
      unique_trigrams.insert(trigram);
    }

    for (uint64_t trigram : unique_trigrams) {
      trigram_index[trigram].append(idx);
    }
  }
}

uint64_t Vocabulary::hashTrigram(uint32_t c1, uint32_t c2, uint32_t c3) {
  uint64_t hash = 0;
  hash = (hash << 21) | (static_cast<uint64_t>(c1) & 0x1FFFFF);
  hash = (hash << 21) | (static_cast<uint64_t>(c2) & 0x1FFFFF);
  hash = (hash << 21) | (static_cast<uint64_t>(c3) & 0x1FFFFF);
  return hash;
}

QVector<uint64_t> Vocabulary::extractTrigrams(const QString &word) {
  QVector<uint64_t> trigrams;
  int len = word.length();

  if (len == 0) {
    return trigrams;
  }

  QVector<uint32_t> char_codes;
  for (int i = 0; i < len; ++i) {
    char_codes.append(word[i].unicode());
  }

  // Для односимвольных слов
  if (len == 1) {
    // Маркеры: #С#
    uint64_t trigram = hashTrigram('#', char_codes[0], '#');
    trigrams.append(trigram);
    return trigrams;
  }

  // Для двухсимвольных слов
  if (len == 2) {
    // Маркеры начала: #СС
    uint64_t trigram1 = hashTrigram('#', char_codes[0], char_codes[1]);
    trigrams.append(trigram1);

    // Маркеры конца: СС#
    uint64_t trigram2 = hashTrigram(char_codes[0], char_codes[1], '#');
    trigrams.append(trigram2);

    return trigrams;
  }

  // Для слов длиной 3 и более - извлекаем все перекрывающиеся триграммы
  for (int i = 0; i <= len - 3; ++i) {
    uint64_t trigram =
        hashTrigram(char_codes[i], char_codes[i + 1], char_codes[i + 2]);
    trigrams.append(trigram);
  }

  // Добавляем начальную триграмму с маркером начала
  uint64_t start_trigram = hashTrigram('#', char_codes[0], char_codes[1]);
  trigrams.append(start_trigram);

  // Добавляем конечную триграмму с маркером конца
  uint64_t end_trigram =
      hashTrigram(char_codes[len - 2], char_codes[len - 1], '#');
  trigrams.append(end_trigram);

  // Для слов длины 3 также добавляем триграммы с маркерами для лучшего поиска
  if (len == 3) {
    uint64_t full_start = hashTrigram('#', char_codes[0], char_codes[1]);
    uint64_t full_end = hashTrigram(char_codes[1], char_codes[2], '#');
    trigrams.append(full_start);
    trigrams.append(full_end);
  }

  return trigrams;
}

uint32_t Vocabulary::getLevensteinDistance(const QString &word1,
                                           const QString &word2) {
  int m = word1.length();
  int n = word2.length();

  int length_diff = m - n;
  if (std::abs(length_diff) > static_cast<int>(MAXLEVENSTEINDIST)) {
    return MAXLEVENSTEINDIST + 1;
  }

  if (m > n) {
    return getLevensteinDistance(word2, word1);
  }

  std::vector<uint32_t> prev(n + 1, 0);
  std::vector<uint32_t> curr(n + 1, 0);

  for (int j = 0; j <= n; ++j) {
    prev[j] = j;
  }

  for (int i = 1; i <= m; ++i) {
    curr[0] = i;
    uint32_t min_in_row = curr[0];

    int j_start = (i > (int)MAXLEVENSTEINDIST) ? i - MAXLEVENSTEINDIST : 1;
    int j_end = std::min(n, i + (int)MAXLEVENSTEINDIST);

    for (int j = 1; j < j_start && j <= n; ++j) {
      curr[j] = MAXLEVENSTEINDIST + 1;
    }

    for (int j = j_start; j <= j_end; ++j) {
      uint32_t cost = (word1[i - 1] != word2[j - 1]) ? 1 : 0;

      curr[j] = std::min({curr[j - 1] + 1, prev[j] + 1, prev[j - 1] + cost});

      if (curr[j] < min_in_row) {
        min_in_row = curr[j];
      }
    }

    for (int j = j_end + 1; j <= n; ++j) {
      curr[j] = MAXLEVENSTEINDIST + 1;
    }

    if (min_in_row > MAXLEVENSTEINDIST) {
      return min_in_row;
    }

    std::swap(prev, curr);
  }

  return prev[n];
}

QVector<QString> Vocabulary::checkWordSpelling(const QString &word) {
  QVector<QString> corrections;
  QHash<int, uint8_t> candidate_scores;

  if (isInVocab(word) or word.length() == 0) {
    return corrections;
  }

  QString lower_word = word.toLower();
  QVector<uint64_t> word_trigrams = extractTrigrams(lower_word);

  if (word_trigrams.isEmpty()) {
    return corrections;
  }

  for (uint64_t trigram : word_trigrams) {
    auto it = trigram_index.find(trigram);
    if (it != trigram_index.end()) {
      for (int idx : it.value()) {
        candidate_scores[idx]++;
      }
    }
  }

  if (candidate_scores.isEmpty()) {
    return corrections;
  }

  // Отбираем кандидатов с достаточным количеством совпадений
  QVector<QPair<int, uint8_t>> candidates;
  candidates.reserve(candidate_scores.size());

  for (auto it = candidate_scores.begin(); it != candidate_scores.end(); ++it) {
    if (it.value() >= MIN_TRIGRAM_MATCHES) {
      candidates.append(qMakePair(it.key(), it.value()));
    }
  }

  // Сортируем по убыванию score (чем больше совпадений триграмм, тем лучше)
  std::sort(candidates.begin(), candidates.end(),
            [](const QPair<int, uint8_t> &a, const QPair<int, uint8_t> &b) {
              return a.second > b.second;
            });

  // Берём только топ кандидатов для вычисления расстояния Левенштейна
  size_t num_candidates = std::min<size_t>(candidates.size(), MAX_CANDIDATES);

  // Вычисляем расстояние Левенштейна для топ-кандидатов
  QVector<QPair<uint32_t, QString>> distance_results;
  distance_results.reserve(num_candidates);

  for (size_t i = 0; i < num_candidates; ++i) {
    int idx = candidates[i].first;
    uint32_t dist = getLevensteinDistance(lower_word, vocab_words[idx]);

    if (dist <= MAXLEVENSTEINDIST) {
      distance_results.append(qMakePair(dist, vocab_words[idx]));
    }
  }

  // Сортируем по расстоянию Левенштейна
  std::sort(
      distance_results.begin(), distance_results.end(),
      [](const QPair<uint32_t, QString> &a, const QPair<uint32_t, QString> &b) {
        if (a.first != b.first) {
          return a.first < b.first;
        }
        return a.second.length() < b.second.length();
      });

  const uint16_t maxCorrections = 5;
  for (size_t i = 0;
       i < std::min<size_t>(distance_results.size(), maxCorrections); ++i) {
    corrections.append(distance_results[i].second);
  }

  return corrections;
}
