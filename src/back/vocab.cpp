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

/**
 * @brief Конструктор класса Vocabulary
 *
 * Инициализирует путь к файлу словаря и настраивает параметры.
 *
 * @param path Путь к файлу словаря в файловой системе
 * @param parent Родительский объект Qt (по умолчанию nullptr)
 */
Vocabulary::Vocabulary(const QString &path, QObject *parent)
    : QObject(parent), vocab_path(path) {}

/**
 * @brief Вычислить максимальную длину строки в файле
 *
 * Просматривает весь файл и находит максимальную длину строки в символах.
 * После выполнения курсор файла возвращается в начало.
 *
 * @param file Ссылка на поток файла
 * @return size_t Максимальную длину строки в символах
 */
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

/**
 * @brief Добавляет слово в хэш-таблицу словаря
 *
 * Преобразует строку из UTF-8 в QString, вычисляет хэш и добавляет слово
 * в соответствующий вектор по длине слова. При необходимости расширяет
 * хэш-таблицу.
 *
 * @param line Строка из файла в кодировке UTF-8
 */
void Vocabulary::addWordToHashTable(const std::string &line) {
  QString qline = QString::fromUtf8(line.c_str());
  size_t hash = createHashCode(qline);
  size_t len = qline.length();

  // Расширяем хэш-таблицу если необходимо
  if (len >= (size_t)vocab_hash_table.size()) {
    vocab_hash_table.resize(len + 1);
  }

  // Добавляем слово в хэш-таблицу
  vocab_hash_table[len][hash] = qline;
  vocab_words.append(qline);
}

/**
 * @brief Асинхронная загрузка словаря из файла
 *
 * Читает слова из файла в отдельном потоке, строит хэш-таблицу для быстрого
 * поиска и создаёт индекс триграмм для эффективного поиска исправлений. Во
 * время работы генерируются сигналы loadStarted(), loadProgress(),
 * loadFinished().
 */
void Vocabulary::loadVocabAsync() {
  QThread *thread = QThread::create([this]() {
    emit loadStarted();

    QElapsedTimer timer;
    timer.start();

    string line;

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

      addWordToHashTable(line);
      wordCount++;
    }

    file.close();
    buildTrigramIndex();

    double elapsed = timer.elapsed() / 1000.0;

    isLoaded_ = true;
    emit loadProgress(wordCount);
    emit loadFinished();
  });

  connect(thread, &QThread::finished, thread, &QThread::deleteLater);
  thread->start();
}

/**
 * @brief Создать хэш код для строки (алгоритм DJB2)
 *
 * @param str Ссылка на строку, для которой вычисляется хэш
 * @return size_t Хэш код строки
 */
size_t Vocabulary::createHashCode(const QString &str) {
  size_t hash = 5381;
  int length = str.length();

  for (int i = 0; i < length; i++) {
    hash = ((hash << 5) + hash) + str[i].unicode();
  }

  return hash;
}

/**
 * @brief Получить копию хэш-таблицы словаря
 *
 * @return QVector<QHash<size_t, QString>> Хэш-таблица, где индекс - длина
 * слова, а значение - отображение хэша на слово
 */
QVector<QHash<size_t, QString>> Vocabulary::getVocabHashTable() {
  return vocab_hash_table;
}

/**
 * @brief Проверить наличие строки в словаре
 *
 * @param str Ссылка на строку для проверки
 * @return true если строка есть в словаре, false в противном случае
 */
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

/**
 * @brief Построить индекс триграмм для всех слов словаря
 *
 * После загрузки слов создаёт индекс, сопоставляющий каждую триграмму
 * со списком индексов слов, содержащих эту триграмму.
 * Дубликаты триграмм для одного слова игнорируются.
 */
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

/**
 * @brief Вычислить хэш для триграммы
 *
 * Создаёт 64-битный хэш из 3 символов путём их объединения с битовыми сдвигами.
 * Каждый символ кодируется 21 битом, что позволяет избежать коллизий.
 *
 * @param c1 Код первого символа
 * @param c2 Код второго символа
 * @param c3 Код третьего символа
 * @return uint64_t Хэш триграммы
 */
uint64_t Vocabulary::hashTrigram(uint32_t c1, uint32_t c2, uint32_t c3) {
  uint64_t hash = 0;
  hash = (hash << 21) | (static_cast<uint64_t>(c1) & 0x1FFFFF);
  hash = (hash << 21) | (static_cast<uint64_t>(c2) & 0x1FFFFF);
  hash = (hash << 21) | (static_cast<uint64_t>(c3) & 0x1FFFFF);
  return hash;
}

/**
 * @brief Преобразует строку в вектор кодов символов
 *
 * @param word Исходная строка
 * @return QVector<uint32_t> Вектор Unicode кодов символов
 */
QVector<uint32_t> Vocabulary::stringToCharCodes(const QString &word) {
  QVector<uint32_t> charCodes;
  charCodes.reserve(word.length());
  for (int i = 0; i < word.length(); ++i) {
    charCodes.append(word[i].unicode());
  }
  return charCodes;
}

/**
 * @brief Создаёт триграммы для однобуквенного слова
 *
 * Для слова из одного символа создаёт триграмму вида #С#
 *
 * @param charCodes Вектор кодов символов (должен содержать 1 элемент)
 * @return QVector<uint64_t> Вектор с одной триграммой
 */
QVector<uint64_t>
Vocabulary::extractTrigramsForSingleChar(const QVector<uint32_t> &charCodes) {
  QVector<uint64_t> trigrams;
  // Маркеры: #С#
  uint64_t trigram = hashTrigram('#', charCodes[0], '#');
  trigrams.append(trigram);
  return trigrams;
}

/**
 * @brief Создаёт триграммы для двухбуквенного слова
 *
 * Для слова из двух символов создаёт триграммы вида #СС и СС#
 *
 * @param charCodes Вектор кодов символов (должен содержать 2 элемента)
 * @return QVector<uint64_t> Вектор с двумя триграммами
 */
QVector<uint64_t>
Vocabulary::extractTrigramsForDoubleChar(const QVector<uint32_t> &charCodes) {
  QVector<uint64_t> trigrams;
  // Маркеры начала: #СС
  uint64_t trigram1 = hashTrigram('#', charCodes[0], charCodes[1]);
  trigrams.append(trigram1);

  // Маркеры конца: СС#
  uint64_t trigram2 = hashTrigram(charCodes[0], charCodes[1], '#');
  trigrams.append(trigram2);
  return trigrams;
}

/**
 * @brief Создаёт перекрывающиеся триграммы для длинного слова
 *
 * Для слов длиной 3 и более символов извлекает все перекрывающиеся триграммы
 *
 * @param charCodes Вектор кодов символов
 * @return QVector<uint64_t> Вектор внутренних триграмм
 */
QVector<uint64_t>
Vocabulary::extractOverlappingTrigrams(const QVector<uint32_t> &charCodes) {
  QVector<uint64_t> trigrams;
  int len = charCodes.size();
  trigrams.reserve(len - 2);

  for (int i = 0; i <= len - 3; ++i) {
    uint64_t trigram =
        hashTrigram(charCodes[i], charCodes[i + 1], charCodes[i + 2]);
    trigrams.append(trigram);
  }
  return trigrams;
}

/**
 * @brief Добавляет маркерные триграммы для длинного слова
 *
 * Добавляет триграммы начала (#AB) и конца (BC#) для лучшего поиска
 *
 * @param charCodes Вектор кодов символов
 * @param trigrams Вектор для добавления маркерных триграмм
 */
void Vocabulary::addBoundaryTrigrams(const QVector<uint32_t> &charCodes,
                                     QVector<uint64_t> &trigrams) {
  int len = charCodes.size();

  // Добавляем начальную триграмму с маркером начала
  uint64_t startTrigram = hashTrigram('#', charCodes[0], charCodes[1]);
  trigrams.append(startTrigram);

  // Добавляем конечную триграмму с маркером конца
  uint64_t endTrigram =
      hashTrigram(charCodes[len - 2], charCodes[len - 1], '#');
  trigrams.append(endTrigram);

  // Для слов длины 3 также добавляем дополнительные триграммы с маркерами
  if (len == 3) {
    uint64_t fullStart = hashTrigram('#', charCodes[0], charCodes[1]);
    uint64_t fullEnd = hashTrigram(charCodes[1], charCodes[2], '#');
    trigrams.append(fullStart);
    trigrams.append(fullEnd);
  }
}

/**
 * @brief Извлечь триграммы из слова
 *
 * Разбивает слово на перекрывающиеся последовательности из 3 символов.
 * Для слов короче 3 символов создаёт специальные триграммы с дополнением '#'.
 *
 * @param word Слово для извлечения триграмм
 * @return QVector<uint64_t> Вектор хэшей триграмм
 */
QVector<uint64_t> Vocabulary::extractTrigrams(const QString &word) {
  int len = word.length();

  if (len == 0) {
    return QVector<uint64_t>();
  }

  QVector<uint32_t> charCodes = stringToCharCodes(word);

  if (len == 1) {
    return extractTrigramsForSingleChar(charCodes);
  }

  if (len == 2) {
    return extractTrigramsForDoubleChar(charCodes);
  }

  // Для слов длиной 3 и более
  QVector<uint64_t> trigrams = extractOverlappingTrigrams(charCodes);
  addBoundaryTrigrams(charCodes, trigrams);

  return trigrams;
}

/**
 * @brief Вычислить расстояние Левенштейна между двумя словами
 *
 * Использует оптимизированный алгоритм с ранним выходом и
 * ограничением диапазона вычислений. Если разница длин превышает
 * MAXLEVENSTEINDIST, функция возвращает MAXLEVENSTEINDIST + 1.
 *
 * @param word1 Первое слово для сравнения
 * @param word2 Второе слово для сравнения
 * @return uint32_t Расстояние Левенштейна (количество операций редактирования)
 */
uint32_t Vocabulary::getLevensteinDistance(const QString &word1,
                                           const QString &word2) {
  int m = word1.length();
  int n = word2.length();

  int lengthDiff = m - n;
  if (std::abs(lengthDiff) > static_cast<int>(MAXLEVENSTEINDIST)) {
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
    uint32_t minInRow = curr[0];

    int jStart =
        (i > static_cast<int>(MAXLEVENSTEINDIST)) ? i - MAXLEVENSTEINDIST : 1;
    int jEnd = std::min(n, i + static_cast<int>(MAXLEVENSTEINDIST));

    // Заполняем левую запретную зону
    for (int j = 1; j < jStart && j <= n; ++j) {
      curr[j] = MAXLEVENSTEINDIST + 1;
    }

    // Вычисляем для допустимого диапазона
    for (int j = jStart; j <= jEnd; ++j) {
      uint32_t cost = (word1[i - 1] != word2[j - 1]) ? 1 : 0;
      curr[j] = std::min({curr[j - 1] + 1, prev[j] + 1, prev[j - 1] + cost});

      if (curr[j] < minInRow) {
        minInRow = curr[j];
      }
    }

    // Заполняем правую запретную зону
    for (int j = jEnd + 1; j <= n; ++j) {
      curr[j] = MAXLEVENSTEINDIST + 1;
    }

    // Ранний выход, если минимальное расстояние уже превышает лимит
    if (minInRow > MAXLEVENSTEINDIST) {
      return minInRow;
    }

    std::swap(prev, curr);
  }

  return prev[n];
}

/**
 * @brief Вычисляет оценки кандидатов на основе совпадений триграмм
 *
 * Для каждой триграммы искомого слова находит соответствующие слова в индексе
 * и увеличивает их счётчик.
 *
 * @param wordTrigrams Триграммы искомого слова
 * @return QHash<int, uint8_t> Словарь: индекс слова -> количество совпавших
 * триграмм
 */
QHash<int, uint8_t>
Vocabulary::scoreCandidatesByTrigrams(const QVector<uint64_t> &wordTrigrams) {
  QHash<int, uint8_t> candidateScores;

  for (uint64_t trigram : wordTrigrams) {
    auto it = trigram_index.find(trigram);
    if (it != trigram_index.end()) {
      for (int idx : it.value()) {
        candidateScores[idx]++;
      }
    }
  }

  return candidateScores;
}

/**
 * @brief Фильтрует и сортирует кандидатов по количеству совпадений
 *
 * Отбирает кандидатов с количеством совпадений не менее MIN_TRIGRAM_MATCHES
 * и сортирует их по убыванию (лучшие кандидаты сначала)
 *
 * @param candidateScores Словарь оценок кандидатов
 * @return QVector<QPair<int, uint8_t>> Отсортированный список кандидатов
 */
QVector<QPair<int, uint8_t>> Vocabulary::filterAndSortCandidates(
    const QHash<int, uint8_t> &candidateScores) {

  QVector<QPair<int, uint8_t>> candidates;
  candidates.reserve(candidateScores.size());

  // Отбираем кандидатов с достаточным количеством совпадений
  for (auto it = candidateScores.begin(); it != candidateScores.end(); ++it) {
    if (it.value() >= MIN_TRIGRAM_MATCHES) {
      candidates.append(qMakePair(it.key(), it.value()));
    }
  }

  // Сортируем по убыванию score (чем больше совпадений, тем лучше)
  std::sort(candidates.begin(), candidates.end(),
            [](const QPair<int, uint8_t> &a, const QPair<int, uint8_t> &b) {
              return a.second > b.second;
            });

  return candidates;
}

/**
 * @brief Вычисляет расстояние Левенштейна для лучших кандидатов
 *
 * Берёт top-MAX_CANDIDATES кандидатов и вычисляет для каждого
 * расстояние Левенштейна до исходного слова
 *
 * @param candidates Список кандидатов (индекс, оценка)
 * @param word Исходное слово для сравнения
 * @return QVector<QPair<uint32_t, QString>> Список (расстояние, слово)
 */
QVector<QPair<uint32_t, QString>> Vocabulary::evaluateCandidatesWithLevenstein(
    const QVector<QPair<int, uint8_t>> &candidates, const QString &word) {

  size_t numCandidates = std::min<size_t>(candidates.size(), MAX_CANDIDATES);

  QVector<QPair<uint32_t, QString>> results;
  results.reserve(numCandidates);

  for (size_t i = 0; i < numCandidates; ++i) {
    int idx = candidates[i].first;
    uint32_t dist = getLevensteinDistance(word, vocab_words[idx]);

    if (dist <= MAXLEVENSTEINDIST) {
      results.append(qMakePair(dist, vocab_words[idx]));
    }
  }

  return results;
}

/**
 * @brief Сортирует результаты по расстоянию Левенштейна
 *
 * Сортирует кандидатов по возрастанию расстояния Левенштейна,
 * при равенстве - по длине слова
 *
 * @param results Список результатов для сортировки
 */
void Vocabulary::sortByLevensteinDistance(
    QVector<QPair<uint32_t, QString>> &results) {
  std::sort(
      results.begin(), results.end(),
      [](const QPair<uint32_t, QString> &a, const QPair<uint32_t, QString> &b) {
        if (a.first != b.first) {
          return a.first < b.first;
        }
        return a.second.length() < b.second.length();
      });
}

/**
 * @brief Формирует финальный список исправлений
 *
 * Возвращает не более MAX_CORRECTIONS лучших исправлений
 *
 * @param results Отсортированный список результатов
 * @return QVector<QString> Список слов-исправлений
 */
QVector<QString> Vocabulary::buildCorrectionList(
    const QVector<QPair<uint32_t, QString>> &results) {

  QVector<QString> corrections;
  size_t numCorrections = std::min<size_t>(results.size(), MAX_CORRECTIONS);
  corrections.reserve(numCorrections);

  for (size_t i = 0; i < numCorrections; ++i) {
    corrections.append(results[i].second);
  }

  return corrections;
}

/**
 * @brief Проверить орфографию слова и предложить исправления
 *
 * Использует индекс триграмм для быстрого поиска кандидатов, затем
 * вычисляет расстояние Левенштейна для отбора лучших вариантов.
 *
 * Алгоритм работы:
 * - Проверка на пустое слово или наличие в словаре
 * - Извлечение триграмм из слова
 * - Оценка кандидатов по совпадениям триграмм
 * - Фильтрация и сортировка кандидатов
 * - Вычисление расстояния Левенштейна для лучших кандидатов
 * - Сортировка результатов
 * - Формирование финального списка исправлений
 *
 * @param word Слово для проверки орфографии
 * @return QVector<QString> Вектор слов-исправлений, отсортированных по
 * релевантности
 */
QVector<QString> Vocabulary::checkWordSpelling(const QString &word) {
  if (word.length() == 0 || isInVocab(word)) {
    return QVector<QString>();
  }

  QString lowerWord = word.toLower();
  QVector<uint64_t> wordTrigrams = extractTrigrams(lowerWord);

  if (wordTrigrams.isEmpty()) {
    return QVector<QString>();
  }

  QHash<int, uint8_t> candidateScores = scoreCandidatesByTrigrams(wordTrigrams);

  if (candidateScores.isEmpty()) {
    return QVector<QString>();
  }

  QVector<QPair<int, uint8_t>> candidates =
      filterAndSortCandidates(candidateScores);

  QVector<QPair<uint32_t, QString>> results =
      evaluateCandidatesWithLevenstein(candidates, lowerWord);

  if (results.isEmpty()) {
    return QVector<QString>();
  }

  sortByLevensteinDistance(results);

  return buildCorrectionList(results);
}
