#pragma once

#include "qobject.h"
#include "qtmetamacros.h"
#include "unistring.hpp"
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <memory>
#include <string>
#include <sys/types.h>
#include <unordered_map>
#include <vector>

#include <QObject>
#include <QString>
#include <atomic>

using std::ifstream;
using std::shared_ptr;
using std::string;
using std::unordered_map;
using std::vector;
using utf8::Unistring;

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
   * @brief Конструктор
   *
   * Инициализирует путь к файлу словаря и настраивает параметры
   * @param path Путь к файлу словаря
   */
  Vocabulary(const string &path, QObject *parent = nullptr);

  /**
   * @brief Загрузить словарь из файла
   *
   * Читает слова из файла, строит хэш-таблицу для быстрого поиска
   * и создаёт индекс триграмм для эффективного поиска исправлений
   */
  void loadVocab();

  void loadVocabAsync();

  bool isLoaded() const { return isLoaded_; }

  /**
   * @brief Создать хэш код для строки (алгоритм DJB2)
   * @param str Ссылка на строку
   * @return Хэш код строки
   */
  size_t createHashCode(const Unistring &str);

  /**
   * @brief Получить копию хэш-таблицы словаря
   * @return Хэш-таблица, где индекс - длина слова, а значение - маппинг хэша на
   * слово
   */
  vector<unordered_map<size_t, Unistring>> getVocabHashTable();

  /**
   * @brief Проверить наличие строки в словаре
   * @param str Ссылка на строку для проверки
   * @return true, если строка есть в словаре, иначе false
   */
  bool isInVocab(const Unistring &str);

  /**
   * @brief Проверить орфографию слова и предложить исправления
   *
   * Использует индекс триграмм для быстрого поиска кандидатов, затем
   * вычисляет расстояние Левенштейна для отбора лучших вариантов.
   * @param word Слово для проверки
   * @return Вектор слов-исправлений, отсортированных по релевантности
   */
  std::vector<Unistring> checkWordSpelling(const Unistring &word);

signals:
  void loadStarted();
  void loadProgress(int wordsLoaded); // Только количество загруженных слов
  void loadFinished();
  void loadError(const QString &error);

private:
  std::atomic<bool> isLoaded_{false};

  vector<unordered_map<size_t, Unistring>>
      vocab_hash_table; ///< Хэш-таблица строк словаря (индекс - длина слова)
  ///
  vector<Unistring> vocab_words; ///< Плоский список всех слов словаря
  unordered_map<uint64_t, vector<size_t>>

      trigram_index; ///< Индекс триграмм: триграмма -> список индексов слов

  const string vocab_path; ///< Путь к файлу словаря

  const uint32_t MAXLEVENSTEINDIST =
      3; ///< Максимальное допустимое расстояние Левенштейна

  const uint16_t MAX_CANDIDATES =
      100; ///< Максимальное количество кандидатов для проверки

  const uint8_t MIN_TRIGRAM_MATCHES =
      2; ///< Минимальное количество совпадающих триграмм для кандидата

  /**
   * @brief Вычислить расстояние Левенштейна между двумя словами
   *
   * Использует оптимизированный алгоритм с ранним выходом и
   * ограничением диапазона вычислений.
   * @param word1 Первое слово
   * @param word2 Второе слово
   * @return Расстояние Левенштейна
   */
  uint32_t getLevensteinDistance(const Unistring &word1,
                                 const Unistring &word2);

  /**
   * @brief Вычислить максимальную длину строки в файле
   * @param file Ссылка на поток файла
   * @return Максимальную длину строки в символах
   */
  size_t maxRowLength(ifstream &file);

  /**
   * @brief Построить индекс триграмм для всех слов словаря
   *
   * После загрузки слов создаёт индекс, сопоставляющий каждую триграмму
   * со списком индексов слов, содержащих эту триграмму.
   */
  void buildTrigramIndex();

  /**
   * @brief Извлечь триграммы из слова
   *
   * Разбивает слово на перекрывающиеся последовательности из 3 символов.
   * Для слов короче 3 символов создаёт специальные триграммы с дополнением.
   * @param word Слово для извлечения триграмм
   * @return Вектор хэшей триграмм
   */
  vector<uint64_t> extractTrigrams(const Unistring &word);

  /**
   * @brief Вычислить хэш для триграммы
   *
   * Создаёт 64-битный хэш из 3 символов путём их объединения.
   * @param c1 Код первого символа
   * @param c2 Код второго символа
   * @param c3 Код третьего символа
   * @return Хэш триграммы
   */
  uint64_t hashTrigram(int c1, int c2, int c3);
};
