#include "common/string_utils.hpp"
#include <cstring>
#include <sstream>
#include <string>


namespace CrazyDave{

auto StringUtil::Contains(const std::string &haystack, const std::string &needle) -> bool {
  return (haystack.find(needle) != std::string::npos);
}

void StringUtil::RTrim(std::string *str) {
  // remove trailing ' ', \f, \n, \r, \t, \v
  str->erase(std::find_if(str->rbegin(), str->rend(), [](int ch) { return std::isspace(ch) == 0; }).base(), str->end());
}

auto StringUtil::Indent(int num_indent) -> std::string { return std::string(num_indent, ' '); }  // NOLINT

auto StringUtil::StartsWith(const std::string &str, const std::string &prefix) -> bool {
  return std::equal(prefix.begin(), prefix.end(), str.begin());
}

auto StringUtil::EndsWith(const std::string &str, const std::string &suffix) -> bool {
  // http://stackoverflow.com/a/2072890
  if (suffix.size() > str.size()) {
    return false;
  }
  return std::equal(suffix.rbegin(), suffix.rend(), str.rbegin());
}

auto StringUtil::Repeat(const std::string &str, const std::size_t n) -> std::string {
  std::ostringstream os;
  if (n == 0 || str.empty()) {
    return (os.str());
  }
  for (int i = 0; i < static_cast<int>(n); i++) {
    os << str;
  }
  return (os.str());
}

void StringUtil::Split(const std::string &str, char delimiter, vector<std::string> &res)  {
  std::stringstream ss(str);
  std::string temp;
  while (std::getline(ss, temp, delimiter)) {
    res.push_back(temp);
  }
}

auto StringUtil::Join(const vector<std::string> &input, const std::string &separator) -> std::string {
  std::string result;

  // If the input isn't empty, append the first element. We do this so we don't need to introduce an if into the loop.
  if (!input.empty()) {
    result += input[0];
  }

  // Append the remaining input components, after the first.
  for (uint32_t i = 1; i < input.size(); i++) {
    result += separator + input[i];
  }

  return result;
}

auto StringUtil::Prefix(const std::string &str, const std::string &prefix) -> std::string {
  vector<std::string> lines ;
  StringUtil::Split(str, '\n', lines);

  if (lines.empty()) {
    return ("");
  }

  std::ostringstream os;
  for (uint64_t i = 0, cnt = lines.size(); i < cnt; i++) {
    if (i > 0) {
      os << std::endl;
    }
    os << prefix << lines[i];
  }
  return (os.str());
}

auto StringUtil::Bold(const std::string &str) -> std::string {
  std::string set_plain_text = "\033[0;0m";
  std::string set_bold_text = "\033[0;1m";

  std::ostringstream os;
  os << set_bold_text << str << set_plain_text;
  return (os.str());
}

auto StringUtil::Upper(const std::string &str) -> std::string {
  std::string copy(str);
  std::transform(copy.begin(), copy.end(), copy.begin(), [](unsigned char c) { return std::toupper(c); });
  return (copy);
}

auto StringUtil::Lower(const std::string &str) -> std::string {
  std::string copy(str);
  std::transform(copy.begin(), copy.end(), copy.begin(), [](unsigned char c) { return std::tolower(c); });
  return (copy);
}

auto StringUtil::Split(const std::string &input, const std::string &split) -> vector<std::string> {
  vector<std::string> splits;

  size_t last = 0;
  size_t input_len = input.size();
  size_t split_len = split.size();
  while (last <= input_len) {
    size_t next = input.find(split, last);
    if (next == std::string::npos) {
      next = input_len;
    }

    // Push the substring [last, next) on to splits.
    std::string substr = input.substr(last, next - last);
    if (!substr.empty()) {
      splits.push_back(substr);
    }
    last = next + split_len;
  }
  return splits;
}

auto StringUtil::Strip(const std::string &str, char c) -> std::string {
  // There's a copy here which is wasteful, so don't use this in performance-critical code!
  std::string tmp = str;
  tmp.erase(std::remove(tmp.begin(), tmp.end(), c), tmp.end());
  return tmp;
}

auto StringUtil::Replace(std::string source, const std::string &from, const std::string &to) -> std::string {
  uint64_t start_pos = 0;
  while ((start_pos = source.find(from, start_pos)) != std::string::npos) {
    source.replace(start_pos, from.length(), to);
    start_pos += to.length();  // In case 'to' contains 'from', like
                               // replacing 'x' with 'yx'
  }
  return source;
}


}  // namespace bustub
