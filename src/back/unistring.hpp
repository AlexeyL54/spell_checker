#pragma once

#include <string>
#include <vector>

using std::string;
using std::vector;

namespace utf8 {

class Unistring {
private:
  string value;

public:
  Unistring(const string);

  Unistring(const char *);

  string to_string() const;

  Unistring to_unistring(const string); // TODO:

  Unistring to_unistring(const char *); // TODO:

  size_t length(); // TODO:

  vector<Unistring> split(const Unistring &); // TODO:

  Unistring &operator=(const Unistring &right); // TODO:

  Unistring &operator=(const string &right); // TODO:

  Unistring &operator=(const char *right); // TODO:

  Unistring operator[](size_t);

private:
  bool is_utf8_cyrillic_start(unsigned char c);
  bool is_cyrillic_char(const string &c);
  string to_lower_cyrillic(const string &c);
  bool is_whitespace_utf8(const string &s);
  bool is_punctuation_utf8(const string &s);
  vector<string> utf8_split(const string &s); // дубляж
};

bool operator==(const Unistring &, const Unistring &);
bool operator==(const Unistring &, const string &);
bool operator==(const Unistring &, const char *);
} // namespace utf8
