#pragma once

#include <cstddef>
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
using std::wstring;

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
  int createHashCode(const wstring &str);

  /**
   * @brief Проверить наличие строки в словаре по хэш-коду
   * @param key хэш-код проверяемой строки
   * @return true, если строка есть в словаре, иначе false
   */
  bool isInVocab(int key);

private:
  vector<unordered_map<int, wstring>>
      vocab_hash_table;    // хэш-таблица строк словаря
  const string vocab_path; // путь к файлу словаря
  const int alphabet_size; // размер алфавита проверяемого текста и словаря

  /**
   * @brief Вычислить количество строк в файле
   * @param file ссылка на потокoк
   * @return количество строк в файле
   */
  size_t rowsTotal(ifstream &file);

  /**
   * @brief Конвертировать string в wstring
   * @param str ссылка на конвертируемую строку
   * @return строку типа wstring
   */
  wstring stringToWstring(const string &str);
};
