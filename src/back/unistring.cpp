#include "unistring.hpp"
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <strings.h>

using std::string;
using std::vector;

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

Unistring Unistring::operator[](size_t index) {
  string symbol;
  int8_t bytes_to_decode_symbol;
  unsigned char first_symbol = static_cast<unsigned char>(value[0]);

  if (first_symbol == 0xD0 or first_symbol == 0xD1) {
    bytes_to_decode_symbol = 2;
    index *= bytes_to_decode_symbol;
    symbol.push_back(value[index]);
    symbol.push_back(value[index + 1]);
  } else {
    symbol = value[index];
  }
  return Unistring(symbol);
}

bool Unistring::is_utf8_cyrillic_start(unsigned char c) {
  return (c >= 0xD0 && c <= 0xD1);
}

bool Unistring::is_cyrillic_char(const string &c) {
  if (c.length() == 2) {
    unsigned char c1 = static_cast<unsigned char>(c[0]);
    unsigned char c2 = static_cast<unsigned char>(c[1]);

    if (c1 == 0xD0) {
      return (c2 >= 0x90 && c2 <= 0xBF) || c2 == 0x81;
    } else if (c1 == 0xD1) {
      return (c2 >= 0x80 && c2 <= 0x8F) || c2 == 0x91;
    }
  }
  return false;
}

string Unistring::to_lower_cyrillic(const string &c) {
  if (c.length() != 2)
    return c;

  unsigned char c1 = static_cast<unsigned char>(c[0]);
  unsigned char c2 = static_cast<unsigned char>(c[1]);

  string result;
  result.resize(2);

  if (c1 == 0xD0) {
    if (c2 >= 0x90 && c2 <= 0x9F) {
      result[0] = 0xD0;
      result[1] = c2 + 0x20;
      return result;
    } else if (c2 >= 0xA0 && c2 <= 0xAF) {
      result[0] = 0xD1;
      result[1] = c2 - 0x20;
      return result;
    } else if (c2 == 0x81) {
      result[0] = 0xD1;
      result[1] = 0x91;
      return result;
    }
  }

  return c;
}

bool Unistring::is_whitespace_utf8(const string &s) {
  if (s.length() == 1) {
    char c = s[0];
    return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f' ||
           c == '\v';
  }
  return false;
}

bool Unistring::is_punctuation_utf8(const string &s) {
  if (s.length() == 1) {
    char c = s[0];
    return ispunct(static_cast<unsigned char>(c));
  } else if (s.length() == 2) {
    unsigned char c1 = static_cast<unsigned char>(s[0]);
    unsigned char c2 = static_cast<unsigned char>(s[1]);

    if (c1 == 0xE2 && c2 == 0x80) {
      return true;
    }

    return !is_cyrillic_char(s) && !is_whitespace_utf8(s);
  }
  return true;
}

vector<string> Unistring::utf8_split(const string &s) {
  vector<string> chars;
  for (size_t i = 0; i < s.length();) {
    unsigned char c = static_cast<unsigned char>(s[i]);
    size_t len = 1;

    if (c >= 0xF0)
      len = 4;
    else if (c >= 0xE0)
      len = 3;
    else if (c >= 0xC0)
      len = 2;

    if (i + len <= s.length()) {
      chars.push_back(s.substr(i, len));
    } else {
      chars.push_back(s.substr(i, 1));
    }
    i += len;
  }
  return chars;
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
