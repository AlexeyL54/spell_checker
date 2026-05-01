#pragma once

#include "unistring.hpp"
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <memory>
#include <string>
#include <sys/types.h>
#include <unordered_map>
#include <vector>

using std::ifstream;
using std::shared_ptr;
using std::string;
using std::unordered_map;
using std::vector;
using utf8::Unistring;

class Vocabulary {

public:
  /**
   * @brief Конструктор
   *
   * Инициализировать
   * поле пути к файлу словаря;
   * размер алфавита словаря и проверяемого текста
   */
  Vocabulary(const string &path);

  /**
   * @brief Загрузить словарь и представить его в виде хэш-таблицы
   */
  void loadVocab();

  /**
   * @brief Создать хэш код для строки
   * @param str ссылка на строку
   * @return хэш код
   */
  size_t createHashCode(const Unistring &str);

  /**
   * @brief Получить копию хэш-таблицы словаря
   * @return хэш-таблицу словаря
   */
  vector<unordered_map<size_t, Unistring>> getVocabHashTable();

  /**
   * @brief Проверить наличие строки в словаре
   * @param str ссылка на строку
   * @return true, если строка есть в словаре, иначе false
   */
  bool isInVocab(const Unistring &str);

  std::vector<Unistring> checkWordSpelling(const Unistring &word);

private:
  vector<unordered_map<size_t, Unistring>>
      vocab_hash_table;    // хэш-таблица строк словаря
  const string vocab_path; // путь к файлу словаря
  const uint32_t MAXLEVENSTEINDIST = 3;

  uint32_t getLevensteinDistance(const Unistring &word1,
                                 const Unistring &word2);
  /**
   * @brief Вычислить количество строк в файле
   * @param file ссылка на потокoк
   * @return количество строк в файле
   */
  size_t rowsTotal(ifstream &file);

  size_t maxRowLength(ifstream &file);
};
