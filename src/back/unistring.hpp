#pragma once

#include <cstddef>
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

  Unistring to_lower();

  size_t length();

  vector<Unistring> split(const Unistring &); // TODO:

  size_t find(const Unistring &substr, const Unistring &str); // TODO:

  Unistring &operator=(const Unistring &right);

  Unistring &operator=(const string &right);

  Unistring &operator=(const char *right);

  Unistring operator[](size_t);

private:
};

bool operator==(const Unistring &, const Unistring &);
bool operator==(const Unistring &, const string &);
bool operator==(const Unistring &, const char *);
} // namespace utf8
