#include "unistring.hpp"
#include <cstddef>
#include <cstdint>
#include <cstring>

using std::string;

using utf8::Unistring;

/**
 * @brief Default constructor.
 *
 * Initializes an empty string.
 */
Unistring::Unistring() : offsets_dirty(true) { update_offsets(); };

/**
 * @brief Constructor from std::string.
 * @param str Source string in std::string format (assumed to be UTF-8).
 */
Unistring::Unistring(const string s) : value(s), offsets_dirty(true) {
  update_offsets();
};

/**
 * @brief Constructor from C-string.
 * @param str Source C-string (assumed to be UTF-8).
 */
Unistring::Unistring(const char *s) : value(s), offsets_dirty(true) {
  update_offsets();
};

/**
 * @brief Converts the Unistring object back to std::string.
 * @return Copy of the internal string in std::string format.
 */
string Unistring::to_string() const { return value; }

/**
 * @brief Assignment operator for another Unistring object.
 * @param right Right-hand operand.
 * @return Reference to the current object.
 */
Unistring &Unistring::operator=(const Unistring &right) {
  if (this == &right) {
    return *this;
  }
  value = right.value;
  update_offsets();
  return *this;
}

/**
 * @brief Assignment operator from std::string.
 * @param right Right-hand operand.
 * @return Reference to the current object.
 */
Unistring &Unistring::operator=(const string &right) {
  value = right;
  offsets_dirty = true;
  update_offsets();
  return *this;
}

/**
 * @brief Assignment operator from C-string.
 * @param right Right-hand operand.
 * @return Reference to the current object.
 */
Unistring &Unistring::operator=(const char *right) {
  value = right;
  offsets_dirty = true;
  update_offsets();
  return *this;
}

/**
 * @brief Addition operator for Unistring with Unistring.
 * @param right Right-hand operand.
 * @return New Unistring object as the result of concatenation.
 */
Unistring Unistring::operator+(const Unistring &right) {
  Unistring result(*this);
  result.value += right.value;
  result.offsets_dirty = true;
  result.update_offsets();
  return result;
}

/**
 * @brief Addition operator for Unistring with string.
 * @param right Right-hand operand.
 * @return New Unistring object as the result of concatenation.
 */
Unistring Unistring::operator+(const string &right) {
  Unistring uniright(right);
  return (*this) + uniright;
}

/**
 * @brief Addition-assignment operator for Unistring with Unistring.
 * @param right Right-hand operand.
 * @return Reference to the current object.
 */
Unistring &Unistring::operator+=(const Unistring &right) {
  value += right.value;
  char_offsets.insert(char_offsets.end(), right.char_offsets.begin(),
                      right.char_offsets.end());
  return *this;
}

/**
 * @brief Addition-assignment operator for Unistring with string.
 * @param right Right-hand operand.
 * @return Reference to the current object.
 */
Unistring &Unistring::operator+=(const string &right) {
  Unistring uniright(right);
  value += uniright.value;
  char_offsets.insert(char_offsets.end(), uniright.char_offsets.begin(),
                      uniright.char_offsets.end());
  return *this;
}

/**
 * @brief Updates the character offset cache if it is marked as dirty.
 *
 * Scans the internal value string and fills char_offsets with the starting
 * positions of each UTF-8 character.
 */
void Unistring::update_offsets() const {
  if (!offsets_dirty)
    return;

  char_offsets.clear();
  size_t len = value.length();
  size_t i = 0;
  uint8_t bytes;
  while (i < len) {
    char_offsets.push_back(i);
    bytes = bytes_to_encode_symbol(value[i]);
    if (bytes == 0)
      bytes = 1;
    i += bytes;
  }
}

/**
 * @brief Index access operator (size_t).
 *
 * Returns a new Unistring object containing a single character at the specified
 * index. If the index is out of bounds, returns an empty string.
 * @param index The character index.
 * @return Unistring containing a single character.
 */
Unistring Unistring::operator[](size_t index) const {
  if (index >= char_offsets.size()) {
    return Unistring();
  }

  update_offsets();

  size_t offset = char_offsets[index];
  uint8_t bytes = bytes_to_encode_symbol(value[offset]);
  size_t start_byte = char_offsets[index];
  // Determine the length of the next character to know how many bytes to copy
  // You can take the difference between the next offset and the current one,
  // or calculate the length of the current character
  size_t end_byte;
  if (index + 1 < char_offsets.size()) {
    end_byte = char_offsets[index + 1];
  } else {
    end_byte = value.size();
  }

  std::string symbol_str = value.substr(start_byte, end_byte - start_byte);
  return Unistring(symbol_str);
}

/**
 * @brief Index access operator (int).
 *
 * Returns a new Unistring object containing a single character at the specified
 * index. Supports negative indices? (Current implementation returns empty
 * string when index < 0). If the index is out of bounds, returns an empty
 * string.
 * @param index The character index.
 * @return Unistring containing a single character.
 */
Unistring Unistring::operator[](int index) const {
  if (index < 0 or index >= char_offsets.size()) {
    return Unistring();
  }

  update_offsets();

  size_t offset = char_offsets[index];
  uint8_t bytes = bytes_to_encode_symbol(value[offset]);
  size_t start_byte = char_offsets[index];
  // Determine the length of the next character to know how many bytes to copy
  // You can take the difference between the next offset and the current one,
  // or calculate the length of the current character
  size_t end_byte;
  if (index + 1 < char_offsets.size()) {
    end_byte = char_offsets[index + 1];
  } else {
    end_byte = value.size();
  }

  std::string symbol_str = value.substr(start_byte, end_byte - start_byte);
  return Unistring(symbol_str);
}

/**
 * @brief Determines the number of bytes needed to encode a UTF-8 character
 * starting from the given string.
 * @param symbol String starting with the character to examine.
 * @return Number of bytes (1-4) or 0 if the sequence is invalid.
 */
uint8_t utf8::bytes_to_encode_symbol(const string &symbol) {
  const unsigned char ch = static_cast<const unsigned char>(symbol[0]);

  if ((ch & 0b10000000) == 0) { // 0xxxxxxxx
    return 1;
  } else if ((ch & 0b11100000) == 0b11000000) { // 110xxxxx
    return 2;
  } else if ((ch & 0b11110000) == 0b11100000) { // 1110xxxx
    return 3;
  } else if ((ch & 0b11111000) == 0b11110000) { // 11110xxx
    return 4;
  } else {
    return 0;
  }
}

/**
 * @brief Determines the number of bytes needed to encode a UTF-8 character
 * based on the first byte.
 * @param symbol The first byte of the UTF-8 character.
 * @return Number of bytes (1-4) or 0 if the sequence is invalid.
 */
uint8_t utf8::bytes_to_encode_symbol(const unsigned char symbol) {
  if ((symbol & 0b10000000) == 0) { // 0xxxxxxxx
    return 1;
  } else if ((symbol & 0b11100000) == 0b11000000) { // 110xxxxx
    return 2;
  } else if ((symbol & 0b11110000) == 0b11100000) { // 1110xxxx
    return 3;
  } else if ((symbol & 0b11111000) == 0b11110000) { // 11110xxx
    return 4;
  } else {
    return 0;
  }
}

/**
 * @brief Returns the number of characters (code points) in the string.
 * @return Length of the string in UTF-8 characters.
 */
size_t Unistring::length() const { return char_offsets.size(); }

/**
 * @brief Computes the prefix function for the Knuth-Morris-Pratt algorithm.
 * @return A vector of prefix function values for the current string.
 */
vector<size_t> Unistring::compute_prefix_function() const {
  size_t m = this->length();
  if (m == 0)
    return {};

  vector<size_t> pi(m, 0);
  // length of the current prefix-suffix
  size_t k = 0;

  // starting from the second character (index 1)
  for (size_t q = 1; q < m; ++q) {
    // While there's no match and k > 0, roll back k
    while (k > 0 && (*this)[k] != (*this)[q]) {
      k = pi[k - 1];
    }

    // If characters match, increment k
    if ((*this)[k] == (*this)[q]) {
      ++k;
    }

    pi[q] = k;
  }
  return pi;
}

/**
 * @brief Finds the index of the first occurrence of a substring.
 *
 * Uses the Knuth-Morris-Pratt algorithm for searching.
 * @param substr The substring to search for.
 * @return The index of the start of the substring in the current string if
 * found; otherwise SIZE_MAX.
 */
size_t Unistring::find(const Unistring &substr) {
  size_t n = this->length();
  size_t m = substr.length();

  // Empty substring is found everywhere
  if (m == 0)
    return 0;
  // Substring is longer than the text
  if (n < m)
    return SIZE_MAX;

  vector<size_t> pi = substr.compute_prefix_function();
  size_t q = 0; // Number of matched characters

  for (size_t i = 0; i < n; ++i) {
    while (q > 0 && substr[q] != (*this)[i]) {
      q = pi[q - 1];
    }

    // If characters match, increment q
    if (substr[q] == (*this)[i]) {
      ++q;
    }

    // If all characters of the substring matched
    if (q == m) {
      // Return the starting index of the substring in the text
      return i - m + 1;
    }
  }

  return SIZE_MAX;
}

/**
 * @brief Finds the index of the first occurrence of a substring, starting from
 * the specified index.
 *
 * Uses the Knuth-Morris-Pratt algorithm for searching.
 * @param substr The substring to search for.
 * @param start_index The character index from which to start searching.
 * @return The index of the start of the substring in the current string if
 * found; otherwise SIZE_MAX.
 */
size_t Unistring::find(const Unistring &substr, size_t start_index) {
  size_t n = this->length();
  size_t m = substr.length();

  // Empty substring is found everywhere, return start_index or 0?
  // Typically for an empty substring, return the search start index if valid.
  if (m == 0) {
    if (start_index <= n) {
      return start_index;
    }
    return SIZE_MAX;
  }

  if (start_index >= n or n - start_index < m) {
    return SIZE_MAX;
  }

  vector<size_t> pi = substr.compute_prefix_function();
  // Number of matched characters in the substring
  size_t q = 0;

  for (size_t i = start_index; i < n; ++i) {
    while (q > 0 && substr[q] != (*this)[i]) {
      q = pi[q - 1];
    }

    // If characters match, increment q
    if (substr[q] == (*this)[i]) {
      ++q;
    }

    // If all characters of the substring matched
    if (q == m) {
      // Return the starting index of the substring in the text
      return i - m + 1;
    }
  }

  return SIZE_MAX;
}

/**
 * @brief Converts all characters in the string to lowercase.
 *
 * Supports basic Cyrillic characters (A-Ya, Yo, I, Ye) and ASCII.
 * @return A new Unistring object with lowercase letters.
 */
Unistring Unistring::to_lower() {
  std::string lower_str = value;
  size_t len = value.length();

  for (size_t i = 0; i < len;) {
    unsigned char c1 = static_cast<unsigned char>(value[i]);

    // If this is the start of a 2-byte UTF-8 character (110xxxxx)
    if ((c1 & 0xE0) == 0xC0) {

      // Protection against invalid UTF-8
      if (i + 1 >= len)
        break;

      unsigned char c2 = static_cast<unsigned char>(value[i + 1]);

      // Special cases (Yo, I, Ye)
      if (c1 == 0xD0) {
        if (c2 == 0x81) { // Yo (Ё) -> yo (ё)
          lower_str[i] = 0xD1;
          lower_str[i + 1] = 0x91;
        } else if (c2 == 0x86) { // I (І) -> i (і)
          lower_str[i] = 0xD1;
          lower_str[i + 1] = 0x96;
        } else if (c2 == 0x88) { // Ye (Є) -> ye (є)
          lower_str[i] = 0xD1;
          lower_str[i + 1] = 0x94;
        }
        // A-P (D0 90-9F) -> a-p (D0 B0-BF)
        else if (c2 >= 0x90 && c2 <= 0x9F) {
          lower_str[i] = 0xD0;
          lower_str[i + 1] = c2 + 0x20;
        }
        // R-Ya (D0 A0-AF) -> r-ya (D1 80-8F)
        else if (c2 >= 0xA0 && c2 <= 0xAF) {
          lower_str[i] = 0xD1;
          lower_str[i + 1] = c2 - 0x20;
        }
      }

      i += 2;
    } else {
      // TODO: Single-byte character (ASCII) or other UTF-8 length
      i++;
    }
  }

  return Unistring(lower_str);
}

/**
 * @brief Returns the cached byte offsets for each character.
 * @return Vector of byte offsets.
 */
vector<size_t> Unistring::get_char_offsets() const { return char_offsets; }

/**
 * @brief Extracts a substring from the string.
 * @param start The starting character index (inclusive).
 * @param end The ending character index (inclusive).
 * @return A new Unistring object containing the extracted substring.
 */
Unistring Unistring::substr(size_t start, size_t end) const {
  size_t len = this->length();

  if (start >= len || start > end) {
    return Unistring("");
  }
  if (end >= len) {
    end = len - 1;
  }
  if (this->offsets_dirty)
    update_offsets();

  // Get the byte start of the first character of the substring
  size_t byte_start = char_offsets[start];

  // Byte start of the character following the last character
  size_t byte_end;
  if (end + 1 < char_offsets.size()) {
    byte_end = char_offsets[end + 1];
  } else {
    byte_end = value.size();
  }

  std::string sub_value = value.substr(byte_start, byte_end - byte_start);
  return Unistring(sub_value);
}

/**
 * @brief Compares two Unistring objects for equality.
 * @param s1 First operand.
 * @param s2 Second operand.
 * @return true if the strings are identical; false otherwise.
 */
bool utf8::operator==(const Unistring &s1, const Unistring &s2) {
  return s1.to_string() == s2.to_string();
}

/**
 * @brief Compares a Unistring object with a std::string for equality.
 * @param s1 Unistring object.
 * @param s2 std::string object.
 * @return true if the strings are identical; false otherwise.
 */
bool utf8::operator==(const Unistring &s1, const string &s2) {
  return s1.to_string() == s2;
}

/**
 * @brief Compares a Unistring object with a C-string for equality.
 * @param s1 Unistring object.
 * @param s2 C-string.
 * @return true if the strings are identical; false otherwise.
 */
bool utf8::operator==(const Unistring &s1, const char *s2) {
  return s1.to_string() == s2;
}

/**
 * @brief Compares two Unistring objects for inequality.
 * @param s1 First operand.
 * @param s2 Second operand.
 * @return true if the strings are different; false otherwise.
 */
bool utf8::operator!=(const Unistring &s1, const Unistring &s2) {
  return s1.to_string() != s2.to_string();
}

/**
 * @brief Compares a Unistring object with a std::string for inequality.
 * @param s1 Unistring object.
 * @param s2 std::string object.
 * @return true if the strings are different; false otherwise.
 */
bool utf8::operator!=(const Unistring &s1, const string &s2) {
  return s1.to_string() != s2;
}

/**
 * @brief Compares a Unistring object with a C-string for inequality.
 * @param s1 Unistring object.
 * @param s2 C-string.
 * @return true if the strings are different; false otherwise.
 */
bool utf8::operator!=(const Unistring &s1, const char *s2) {
  return s1.to_string() != s2;
}

// TODO: support for multi-byte characters
/**
 * @brief Converts a single-character Unistring string to its Unicode code point
 * (int).
 * @param ch The character to convert.
 * @return The character code as an int, or -1 if ch is a substring (length >
 * 1).
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
