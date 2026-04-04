#include "unistring.hpp"
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <strings.h>

using std::string;

using utf8::Unistring;

Unistring::Unistring() {};

Unistring::Unistring(const string s) : value(s) {};

Unistring::Unistring(const char *s) : value(s) {};

string Unistring::to_string() const { return value; }

Unistring &Unistring::operator=(const Unistring &right) {
  if (this == &right) {
    return *this;
  }
  value = right.value;
  return *this;
}

Unistring &Unistring::operator=(const string &right) {
  value = right;
  return *this;
}

Unistring &Unistring::operator=(const char *right) {
  value = right;
  return *this;
}

Unistring Unistring::operator[](size_t index) const {
  string symbol;
  int8_t bytes_to_decode_symbol;
  unsigned char first_byte = static_cast<unsigned char>(value[0]);

  if (first_byte == 0xD0 or first_byte == 0xD1) {
    bytes_to_decode_symbol = 2;
    index *= bytes_to_decode_symbol;
    symbol.push_back(value[index]);
    symbol.push_back(value[index + 1]);
  } else {
    symbol = value[index];
  }
  return Unistring(symbol);
}

Unistring Unistring::operator[](int index) const {
  string symbol;
  int8_t bytes_to_decode_symbol;
  unsigned char first_byte = static_cast<unsigned char>(value[0]);

  if (first_byte == 0xD0 or first_byte == 0xD1) {
    bytes_to_decode_symbol = 2;
    index *= bytes_to_decode_symbol;
    symbol.push_back(value[index]);
    symbol.push_back(value[index + 1]);
  } else {
    symbol = value[index];
  }
  return Unistring(symbol);
}

size_t Unistring::length() const {
  size_t len = 0;
  int8_t bytes_to_decode_symbol;
  unsigned char first_byte_of_symbol;

  for (size_t i = 0; i < value.length();) {
    first_byte_of_symbol = static_cast<unsigned char>(value[i]);

    if (first_byte_of_symbol == 0xD0 or first_byte_of_symbol == 0xD1) {
      bytes_to_decode_symbol = 2;
    } else {
      bytes_to_decode_symbol = 1;
    }
    len++;
    i += bytes_to_decode_symbol;
  }
  return len;
}

Unistring Unistring::to_lower() {
  std::string lower_str = value;
  size_t len = value.length();

  for (size_t i = 0; i < len;) {
    unsigned char c1 = static_cast<unsigned char>(value[i]);

    // Если это начало 2-байтового символа UTF-8 (110xxxxx)
    if ((c1 & 0xE0) == 0xC0) {

      // Защита от некорректной UTF-8
      if (i + 1 >= len)
        break;

      unsigned char c2 = static_cast<unsigned char>(value[i + 1]);

      // Специальные случаи (Ё, І, Є)
      if (c1 == 0xD0) {
        if (c2 == 0x81) { // Ё -> ё
          lower_str[i] = 0xD1;
          lower_str[i + 1] = 0x91;
        } else if (c2 == 0x86) { // І -> і
          lower_str[i] = 0xD1;
          lower_str[i + 1] = 0x96;
        } else if (c2 == 0x88) { // Є -> є
          lower_str[i] = 0xD1;
          lower_str[i + 1] = 0x94;
        }
        // А-П (D0 90-9F) -> а-п (D0 B0-BF)
        else if (c2 >= 0x90 && c2 <= 0x9F) {
          lower_str[i] = 0xD0;
          lower_str[i + 1] = c2 + 0x20;
        }
        // Р-Я (D0 A0-AF) -> р-я (D1 80-8F)
        else if (c2 >= 0xA0 && c2 <= 0xAF) {
          lower_str[i] = 0xD1;
          lower_str[i + 1] = c2 - 0x20;
        }
      }

      i += 2;
    } else {
      // TODO: Однобайтовый символ (ASCII) или другая длина UTF-8
      i++;
    }
  }

  return Unistring(lower_str);
}

bool utf8::operator==(const Unistring &s1, const Unistring &s2) {
  return s1.to_string() == s2.to_string();
}

bool utf8::operator==(const Unistring &s1, const string &s2) {
  return s1.to_string() == s2;
}

bool utf8::operator==(const Unistring &s1, const char *s2) {
  return s1.to_string() == s2;
}

/*
 * @brief Конвертировать символ строки Unistring в int
 * @param ch символ
 * @return код типа int, -1 в случае если ch является подстрокой
 */
int utf8::unichar_to_int(const Unistring &ch) {
  if (ch.length() > 1) {
    return -1;
  }

  unsigned char first_byte = ch.to_string()[0];
  unsigned char second_byte = ch.to_string()[1];
  int code = (first_byte << 8) | second_byte;

  return code;
}
