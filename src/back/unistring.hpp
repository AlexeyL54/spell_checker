#pragma once

#include <cstddef>
#include <string>
#include <vector>

using std::string;
using std::vector;

// TODO: поддержка 3 и 4 байтовых символов

namespace utf8 {

class Unistring {
private:
  string value;

public:
  /**
   * @brief Конструктор
   */
  Unistring();

  /**
   * @brief Конструктор
   * @param str строка
   */
  Unistring(const string str);

  /**
   * @brief Конструктор
   * @param str строка
   */
  Unistring(const char *str);

  /**
   * @brief Перевести в std::string
   */
  string to_string() const;

  /**
   * @brief Перевести строку в нижний регистр
   */
  Unistring to_lower();

  /**
   * @brief Определить длину строки
   */
  size_t length() const;

  /**
   * @brief Разбить строку на вектор подстрок по разделителю
   * @param splitter разделитель
   */
  vector<Unistring> split(const Unistring &splitter); // TODO:

  /**
   * @brief Найти индекс подстроки в строке
   * @param substr подстрока
   * @param str строка
   * @return индекс подстроки в строке, если подстрока найдена в строке, иначе
   * SIZE_MAX
   */
  size_t find(const Unistring &substr, const Unistring &str); // TODO:

  Unistring &operator=(const Unistring &right);

  Unistring &operator=(const string &right);

  Unistring &operator=(const char *right);

  Unistring operator[](size_t) const;

  Unistring operator[](int) const;

private:
};

bool operator==(const Unistring &, const Unistring &);
bool operator==(const Unistring &, const string &);
bool operator==(const Unistring &, const char *);

/*
 * @brief Конвертировать символ строки Unistring в int
 * @param ch символ
 * @return код типа int
 */
int unichar_to_int(const Unistring &ch);

} // namespace utf8
