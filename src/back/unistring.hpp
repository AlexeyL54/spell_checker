#pragma once
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

using std::string;
using std::vector;

namespace utf8 {

/**
 * @brief Class for working with UTF-8 strings.
 *
 * Provides functionality for correct handling of multi-byte UTF-8 characters,
 * including obtaining length in characters, index access, substring search,
 * and conversion to lowercase.
 */
class Unistring {
public:
  /**
   * @brief Default constructor.
   *
   * Initializes an empty string.
   */
  Unistring();

  /**
   * @brief Constructor from std::string.
   * @param str Source string in std::string format (assumed to be UTF-8).
   */
  Unistring(const string str);

  /**
   * @brief Constructor from C-string.
   * @param str Source C-string (assumed to be UTF-8).
   */
  Unistring(const char *str);

  /**
   * @brief Converts the Unistring object back to std::string.
   * @return Copy of the internal string in std::string format.
   */
  string to_string() const;

  /**
   * @brief Converts all characters in the string to lowercase.
   *
   * Supports basic Cyrillic characters (A-Ya, Yo, I, Ye) and ASCII.
   * @return A new Unistring object with lowercase letters.
   */
  Unistring to_lower();

  /**
   * @brief Returns the number of characters (code points) in the string.
   * @return Length of the string in UTF-8 characters.
   */
  size_t length() const;

  /**
   * @brief Splits the string into a vector of substrings by delimiter.
   * @param splitter The delimiter string.
   * @return Vector of substrings.
   * @todo Implement the split method.
   */
  // TODO: vector<Unistring> split(const Unistring &splitter);

  /**
   * @brief Finds the index of the first occurrence of a substring.
   *
   * Uses the Knuth-Morris-Pratt algorithm for searching.
   * @param substr The substring to search for.
   * @return The index of the start of the substring in the current string if
   * found; otherwise SIZE_MAX.
   */
  size_t find(const Unistring &substr);

  /**
   * @brief Finds the index of the first occurrence of a substring, starting
   * from the specified index.
   *
   * Uses the Knuth-Morris-Pratt algorithm for searching.
   * @param substr The substring to search for.
   * @param start_index The character index from which to start searching.
   * @return The index of the start of the substring in the current string if
   * found; otherwise SIZE_MAX.
   */
  size_t find(const Unistring &substr, size_t start_index);

  /**
   * @brief Assignment operator for another Unistring object.
   * @param right Right-hand operand.
   * @return Reference to the current object.
   */
  Unistring &operator=(const Unistring &right);

  /**
   * @brief Assignment operator from std::string.
   * @param right Right-hand operand.
   * @return Reference to the current object.
   */
  Unistring &operator=(const string &right);

  /**
   * @brief Assignment operator from C-string.
   * @param right Right-hand operand.
   * @return Reference to the current object.
   */
  Unistring &operator=(const char *right);

  /**
   * @brief Addition operator for Unistring with Unistring.
   * @param right Right-hand operand.
   * @return New Unistring object as the result of concatenation.
   */
  Unistring operator+(const Unistring &right);

  /**
   * @brief Addition operator for Unistring with string.
   * @param right Right-hand operand.
   * @return New Unistring object as the result of concatenation.
   */
  Unistring operator+(const string &right);

  /**
   * @brief Addition-assignment operator for Unistring with Unistring.
   * @param right Right-hand operand.
   * @return Reference to the current object.
   */
  Unistring &operator+=(const Unistring &right);

  /**
   * @brief Addition-assignment operator for Unistring with string.
   * @param right Right-hand operand.
   * @return Reference to the current object.
   */
  Unistring &operator+=(const string &right);

  /**
   * @brief Index access operator (size_t).
   *
   * Returns a new Unistring object containing a single character at the
   * specified index. If the index is out of bounds, returns an empty string.
   * @param index The character index.
   * @return Unistring containing a single character.
   */
  Unistring operator[](size_t index) const;

  /**
   * @brief Index access operator (int).
   *
   * Returns a new Unistring object containing a single character at the
   * specified index. Supports negative indices? (Current implementation returns
   * empty string when index < 0). If the index is out of bounds, returns an
   * empty string.
   * @param index The character index.
   * @return Unistring containing a single character.
   */
  Unistring operator[](int index) const;

  /**
   * @brief Returns the cached byte offsets for each character.
   * @return Vector of byte offsets.
   */
  vector<size_t> get_char_offsets() const;

  /**
   * @brief Extracts a substring from the string.
   * @param start The starting character index (inclusive).
   * @param end The ending character index (inclusive).
   * @return A new Unistring object containing the extracted substring.
   */
  Unistring substr(size_t start, size_t end) const;

private:
  string value; ///< Internal string representation as a UTF-8 byte sequence.
  mutable vector<size_t>
      char_offsets; ///< Cache of byte offsets for each UTF-8 character.
  mutable bool offsets_dirty; ///< Flag indicating that the offset cache is
                              ///< stale and needs updating.

  /**
   * @brief Updates the character offset cache if it is marked as dirty.
   *
   * Scans the internal value string and fills char_offsets with the starting
   * positions of each UTF-8 character.
   */
  void update_offsets() const;

  /**
   * @brief Computes the prefix function for the Knuth-Morris-Pratt algorithm.
   * @return A vector of prefix function values for the current string.
   */
  vector<size_t> compute_prefix_function() const;
};

/**
 * @brief Compares two Unistring objects for equality.
 * @param s1 First operand.
 * @param s2 Second operand.
 * @return true if the strings are identical; false otherwise.
 */
bool operator==(const Unistring &s1, const Unistring &s2);

/**
 * @brief Compares a Unistring object with a std::string for equality.
 * @param s1 Unistring object.
 * @param s2 std::string object.
 * @return true if the strings are identical; false otherwise.
 */
bool operator==(const Unistring &s1, const string &s2);

/**
 * @brief Compares a Unistring object with a C-string for equality.
 * @param s1 Unistring object.
 * @param s2 C-string.
 * @return true if the strings are identical; false otherwise.
 */
bool operator==(const Unistring &s1, const char *s2);

/**
 * @brief Compares two Unistring objects for inequality.
 * @param s1 First operand.
 * @param s2 Second operand.
 * @return true if the strings are different; false otherwise.
 */
bool operator!=(const Unistring &s1, const Unistring &s2);

/**
 * @brief Compares a Unistring object with a std::string for inequality.
 * @param s1 Unistring object.
 * @param s2 std::string object.
 * @return true if the strings are different; false otherwise.
 */
bool operator!=(const Unistring &s1, const string &s2);

/**
 * @brief Compares a Unistring object with a C-string for inequality.
 * @param s1 Unistring object.
 * @param s2 C-string.
 * @return true if the strings are different; false otherwise.
 */
bool operator!=(const Unistring &s1, const char *s2);

/**
 * @brief Determines the number of bytes needed to encode a UTF-8 character
 * starting from the given string.
 * @param symbol String starting with the character to examine.
 * @return Number of bytes (1-4) or 0 if the sequence is invalid.
 */
uint8_t bytes_to_encode_symbol(const string &symbol);

/**
 * @brief Determines the number of bytes needed to encode a UTF-8 character
 * based on the first byte.
 * @param symbol The first byte of the UTF-8 character.
 * @return Number of bytes (1-4) or 0 if the sequence is invalid.
 */
uint8_t bytes_to_encode_symbol(const unsigned char symbol);

/**
 * @brief Converts a single-character Unistring string to its Unicode code point
 * (int).
 *
 * Note: The current implementation only supports 2-byte characters or may work
 * incorrectly for other cases.
 * @param ch Unistring object containing exactly one character.
 * @return The character code as an int, or -1 if the string length is greater
 * than 1.
 */
int unichar_to_int(const Unistring &ch);

} // namespace utf8
