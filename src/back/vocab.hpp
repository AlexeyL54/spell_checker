#pragma once

#include <QAtomicInt>
#include <QHash>
#include <QObject>
#include <QString>
#include <QVector>
#include <atomic>
#include <fstream>
#include <memory>
#include <unordered_map>
#include <vector>

using std::ifstream;
using std::shared_ptr;
using std::string;
using std::unordered_map;
using std::vector;

/**
 * @brief Класс для работы со словарём и проверки орфографии
 *
 * Предоставляет функциональность загрузки словаря из файла,
 * проверки наличия слов и поиска исправлений для ошибочных слов
 * с использованием индексации по триграммам для высокой производительности.
 */
class Vocabulary : public QObject {
  Q_OBJECT

public:
  /**
   * @brief Конструктор класса Vocabulary
   *
   * Инициализирует путь к файлу словаря и настраивает параметры.
   *
   * @param path Путь к файлу словаря в файловой системе
   * @param parent Родительский объект Qt (по умолчанию nullptr)
   */
  Vocabulary(const QString &path, QObject *parent = nullptr);

  /**
   * @brief Добавляет слово в хэш-таблицу словаря
   *
   * Преобразует строку из UTF-8 в QString, вычисляет хэш и добавляет слово
   * в соответствующий вектор по длине слова. При необходимости расширяет
   * хэш-таблицу.
   *
   * @param line Строка из файла в кодировке UTF-8
   */
  void addWordToHashTable(const std::string &line);

  /**
   * @brief Асинхронная загрузка словаря из файла
   *
   * Читает слова из файла в отдельном потоке, строит хэш-таблицу для быстрого
   * поиска и создаёт индекс триграмм для эффективного поиска исправлений. Во
   * время работы генерируются сигналы loadStarted(), loadProgress(),
   * loadFinished().
   */
  void loadVocabAsync();

  /**
   * @brief Проверяет, загружен ли словарь
   *
   * @return true если словарь успешно загружен, false в противном случае
   */
  bool isLoaded() const { return isLoaded_; }

  /**
   * @brief Создать хэш код для строки (алгоритм DJB2)
   *
   * @param str Ссылка на строку, для которой вычисляется хэш
   * @return size_t Хэш код строки
   */
  size_t createHashCode(const QString &str);

  /**
   * @brief Получить копию хэш-таблицы словаря
   *
   * @return QVector<QHash<size_t, QString>> Хэш-таблица, где индекс - длина
   * слова, а значение - отображение хэша на слово
   */
  QVector<QHash<size_t, QString>> getVocabHashTable();

  /**
   * @brief Проверить наличие строки в словаре
   *
   * @param str Ссылка на строку для проверки
   * @return true если строка есть в словаре, false в противном случае
   */
  bool isInVocab(const QString &str);

  /**
   * @brief Проверить орфографию слова и предложить исправления
   *
   * Использует индекс триграмм для быстрого поиска кандидатов, затем
   * вычисляет расстояние Левенштейна для отбора лучших вариантов.
   *
   * @param word Слово для проверки орфографии
   * @return QVector<QString> Вектор слов-исправлений, отсортированных по
   * релевантности
   */
  QVector<QString> checkWordSpelling(const QString &word);

signals:
  /**
   * @brief Сигнал о начале загрузки словаря
   */
  void loadStarted();

  /**
   * @brief Сигнал о прогрессе загрузки словаря
   * @param wordsLoaded Количество успешно загруженных слов
   */
  void loadProgress(int wordsLoaded);

  /**
   * @brief Сигнал о завершении загрузки словаря
   */
  void loadFinished();

  /**
   * @brief Сигнал об ошибке при загрузке словаря
   * @param error Текстовое описание ошибки
   */
  void loadError(const QString &error);

private:
  std::atomic<bool> isLoaded_{false}; ///< Флаг загрузки словаря
  QVector<QHash<size_t, QString>>
      vocab_hash_table;         ///< Хэш-таблица строк словаря
  QVector<QString> vocab_words; ///< Список всех слов словаря
  QHash<uint64_t, QVector<int>>
      trigram_index; ///< Индекс: триграмма -> индексы слов

  const QString vocab_path; ///< Путь к файлу словаря

  const uint32_t MAXLEVENSTEINDIST =
      3; ///< Максимальное допустимое расстояние Левенштейна
  const uint16_t MAX_CANDIDATES =
      100; ///< Максимальное количество кандидатов для проверки
  const uint8_t MIN_TRIGRAM_MATCHES =
      2; ///< Минимальное количество совпадающих триграмм для кандидата
  const uint16_t MAX_CORRECTIONS =
      5; ///< Максимальное количество возвращаемых исправлений

  /**
   * @brief Вычислить расстояние Левенштейна между двумя словами
   *
   * Использует оптимизированный алгоритм с ранним выходом и
   * ограничением диапазона вычислений. Если разница длин превышает
   * MAXLEVENSTEINDIST, функция возвращает MAXLEVENSTEINDIST + 1.
   *
   * @param word1 Первое слово для сравнения
   * @param word2 Второе слово для сравнения
   * @return uint32_t Расстояние Левенштейна (количество операций
   * редактирования)
   */
  uint32_t getLevensteinDistance(const QString &word1, const QString &word2);

  /**
   * @brief Вычислить максимальную длину строки в файле
   *
   * Просматривает весь файл и находит максимальную длину строки в символах.
   * После выполнения курсор файла возвращается в начало.
   *
   * @param file Ссылка на поток файла
   * @return size_t Максимальную длину строки в символах
   */
  size_t maxRowLength(ifstream &file);

  /**
   * @brief Построить индекс триграмм для всех слов словаря
   *
   * После загрузки слов создаёт индекс, сопоставляющий каждую триграмму
   * со списком индексов слов, содержащих эту триграмму.
   * Дубликаты триграмм для одного слова игнорируются.
   */
  void buildTrigramIndex();

  /**
   * @brief Извлечь триграммы из слова
   *
   * Разбивает слово на перекрывающиеся последовательности из 3 символов.
   * Для слов короче 3 символов создаёт специальные триграммы с дополнением '#'.
   *
   * @param word Слово для извлечения триграмм
   * @return QVector<uint64_t> Вектор хэшей триграмм
   */
  QVector<uint64_t> extractTrigrams(const QString &word);

  /**
   * @brief Вычислить хэш для триграммы
   *
   * Создаёт 64-битный хэш из 3 символов путём их объединения с битовыми
   * сдвигами. Каждый символ кодируется 21 битом, что позволяет избежать
   * коллизий.
   *
   * @param c1 Код первого символа
   * @param c2 Код второго символа
   * @param c3 Код третьего символа
   * @return uint64_t Хэш триграммы
   */
  uint64_t hashTrigram(uint32_t c1, uint32_t c2, uint32_t c3);

  /**
   * @brief Преобразует строку в вектор кодов символов
   *
   * @param word Исходная строка
   * @return QVector<uint32_t> Вектор Unicode кодов символов
   */
  QVector<uint32_t> stringToCharCodes(const QString &word);

  /**
   * @brief Создаёт триграммы для однобуквенного слова
   *
   * Для слова из одного символа создаёт триграмму вида #С#
   *
   * @param charCodes Вектор кодов символов (должен содержать 1 элемент)
   * @return QVector<uint64_t> Вектор с одной триграммой
   */
  QVector<uint64_t>
  extractTrigramsForSingleChar(const QVector<uint32_t> &charCodes);

  /**
   * @brief Создаёт триграммы для двухбуквенного слова
   *
   * Для слова из двух символов создаёт триграммы вида #СС и СС#
   *
   * @param charCodes Вектор кодов символов (должен содержать 2 элемента)
   * @return QVector<uint64_t> Вектор с двумя триграммами
   */
  QVector<uint64_t>
  extractTrigramsForDoubleChar(const QVector<uint32_t> &charCodes);

  /**
   * @brief Создаёт перекрывающиеся триграммы для длинного слова
   *
   * Для слов длиной 3 и более символов извлекает все перекрывающиеся триграммы
   *
   * @param charCodes Вектор кодов символов
   * @return QVector<uint64_t> Вектор внутренних триграмм
   */
  QVector<uint64_t>
  extractOverlappingTrigrams(const QVector<uint32_t> &charCodes);

  /**
   * @brief Добавляет маркерные триграммы для длинного слова
   *
   * Добавляет триграммы начала (#AB) и конца (BC#) для лучшего поиска
   *
   * @param charCodes Вектор кодов символов
   * @param trigrams Вектор для добавления маркерных триграмм
   */
  void addBoundaryTrigrams(const QVector<uint32_t> &charCodes,
                           QVector<uint64_t> &trigrams);

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
  scoreCandidatesByTrigrams(const QVector<uint64_t> &wordTrigrams);

  /**
   * @brief Фильтрует и сортирует кандидатов по количеству совпадений
   *
   * Отбирает кандидатов с количеством совпадений не менее MIN_TRIGRAM_MATCHES
   * и сортирует их по убыванию (лучшие кандидаты сначала)
   *
   * @param candidateScores Словарь оценок кандидатов
   * @return QVector<QPair<int, uint8_t>> Отсортированный список кандидатов
   */
  QVector<QPair<int, uint8_t>>
  filterAndSortCandidates(const QHash<int, uint8_t> &candidateScores);

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
  QVector<QPair<uint32_t, QString>> evaluateCandidatesWithLevenstein(
      const QVector<QPair<int, uint8_t>> &candidates, const QString &word);

  /**
   * @brief Сортирует результаты по расстоянию Левенштейна
   *
   * Сортирует кандидатов по возрастанию расстояния Левенштейна,
   * при равенстве - по длине слова
   *
   * @param results Список результатов для сортировки
   */
  void sortByLevensteinDistance(QVector<QPair<uint32_t, QString>> &results);

  /**
   * @brief Формирует финальный список исправлений
   *
   * Возвращает не более MAX_CORRECTIONS лучших исправлений
   *
   * @param results Отсортированный список результатов
   * @return QVector<QString> Список слов-исправлений
   */
  QVector<QString>
  buildCorrectionList(const QVector<QPair<uint32_t, QString>> &results);
};
