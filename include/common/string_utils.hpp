
#ifndef TICKETSYSTEM_STRING_UTILS_HPP
#define TICKETSYSTEM_STRING_UTILS_HPP

#include <string>
#include "data_structures/vector.h"

namespace CrazyDave {

/**
 * StringUtil provides INEFFICIENT utility functions for working with strings. They should only be used for debugging.
 */
class StringUtil {
 public:
  /** @return true if haystack contains needle, false otherwise */
  static auto Contains(const std::string &haystack, const std::string &needle) -> bool;

  /** @return true if target string starts with given prefix, false otherwise */
  static auto StartsWith(const std::string &str, const std::string &prefix) -> bool;

  /** @return true if target string ends with the given suffix, false otherwise */
  static auto EndsWith(const std::string &str, const std::string &suffix) -> bool;

  /** @return str repeated n times */
  static auto Repeat(const std::string &str, std::size_t n) -> std::string;

  /** @return input string split based on the delimiter */
  static void Split(const std::string &str, char delimiter, vector<std::string> &res);

  /** @return concatenation of all input strings, separated by the separator */
  static auto Join(const vector<std::string> &input, const std::string &separator) -> std::string;

  /** @return join multiple items of container with given size, transformed to string
  using function, into one string using the given separator. */
  template <typename C, typename S, typename Func>
  static auto Join(const C &input, S count, const std::string &separator, Func f) -> std::string {
    // The result
    std::string result;

    // If the input isn't empty, append the first element. We do this so we
    // don't need to introduce an if into the loop.
    if (count > 0) {
      result += f(input[0]);
    }

    // Append the remaining input components, after the first
    for (size_t i = 1; i < count; i++) {
      result += separator + f(input[i]);
    }

    return result;
  }

  /** @return prefix prepended to the beginning of each line in str */
  static auto Prefix(const std::string &str, const std::string &prefix) -> std::string;

  /** @return string wrapped with control characters to appear bold in the console */
  static auto Bold(const std::string &str) -> std::string;

  /** @return uppercase version of the string */
  static auto Upper(const std::string &str) -> std::string;

  /** @return lowercase version of the string */
  static auto Lower(const std::string &str) -> std::string;

  /** @return input string split based on the split string */
  static auto Split(const std::string &input, const std::string &split) -> vector<std::string>;

  /**
   * Removes the whitespace characters from the right side of the string.
   * @param[in,out] str string to be trimmed on the right
   */
  static void RTrim(std::string *str);

  /** @return indented string */
  static auto Indent(int num_indent) -> std::string;

  /**
   * Return a new string that has stripped all occurrences of the provided character from the provided string.
   *
   * NOTE: WASTEFUL. Performs a copy. Do NOT use for performance-critical code!
   *
   * @param str input string
   * @param c character to be removed
   * @return a new string with no occurrences of the provided character
   */
  static auto Strip(const std::string &str, char c) -> std::string;

  /**
   * Replace parts of the string from `from` to `to`.
   *
   * @param source input string
   * @param from substring to be searched
   * @param to replace `from` to `to`
   * @return a new string with all occurrences of `from` replaced with `to`.
   */
  static auto Replace(std::string source, const std::string &from, const std::string &to) -> std::string;

};

}  // namespace CrazyDave

#endif  // TICKETSYSTEM_STRING_UTILS_HPP
