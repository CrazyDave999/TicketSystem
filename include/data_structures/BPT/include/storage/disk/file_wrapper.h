#ifndef BPT_PRO_FILE_WRAPPER_H
#define BPT_PRO_FILE_WRAPPER_H
#include <cstring>
#include <fstream>
#include <string>

/**
 * A helper class for implementing i/o manipulation.
 */

namespace CrazyDave {
class MyFile {
  std::string name_;
  std::fstream fs_;
  bool is_new_{false};

 public:
  explicit MyFile(const std::string &name) {
    fs_.open(name, std::ios::in);
    if (!fs_) {
      is_new_ = true;
      fs_.open(name, std::ios::out);
    }
    fs_.close();
    fs_.open(name, std::ios::in | std::ios::out);
  }

  ~MyFile() { fs_.close(); }

  void SetReadPointer(int offset) { fs_.seekg(offset); }

  void SetWritePointer(int offset) { fs_.seekp(offset); }

  void Read(char *data, int size) {
    fs_.read(data, size);
    if (!fs_.good()) {
      fs_.clear();
    }
  }

  void Write(const char *data, int size) { fs_.write(data, size); }

  template <class T>
  void ReadObj(T &dst, size_t size = sizeof(T)) {
    fs_.read(reinterpret_cast<char *>(&dst), (long)size);
  }

  template <class T>
  void WriteObj(const T &src, size_t size = sizeof(T)) {
    fs_.write(reinterpret_cast<const char *>(&src), (long)size);
  }

  void Flush() { fs_.flush(); }

  auto IsNew() const -> bool { return is_new_; }
};
}  // namespace CrazyDave
#endif  // BPT_PRO_FILE_WRAPPER_H
