#include "unistring.hpp"
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <strings.h>

using std::string;

using utf8::Unistring;

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

Unistring Unistring::operator[](size_t index) {
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

size_t Unistring::length() {
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
  string lower_str = value;

  unsigned char c1 = static_cast<unsigned char>(value[0]);
  unsigned char c2;

  if (c1 == 0xD0) {

    for (size_t i = 0; i < value.length(); i += 2) {

      c1 = static_cast<unsigned char>(value[i]);
      c2 = static_cast<unsigned char>(value[i + 1]);

      if (c2 >= 0x90 && c2 <= 0x9F) {
        lower_str[i] = 0xD0;
        lower_str[i + 1] = c2 + 0x20;
      } else if (c2 >= 0xA0 && c2 <= 0xAF) {
        lower_str[i] = 0xD1;
        lower_str[i + 1] = c2 - 0x20;
      } else if (c2 == 0x81) {
        lower_str[i] = 0xD1;
        lower_str[i + 1] = 0x91;
      }
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
