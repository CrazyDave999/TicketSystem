#ifndef BPT_PRO_DISK_MANAGER_H
#define BPT_PRO_DISK_MANAGER_H

#include <fstream>
#include <string>
#include "common/config.h"
#include "file_wrapper.h"
namespace CrazyDave {

class MyDiskManager {
 public:
  explicit MyDiskManager(const std::string &name) {
    garbage_file = new MyFile(name + "_gb");
    data_file_ = new MyFile(name + "_dt");
    if (!garbage_file->IsNew()) {
      garbage_file->SetReadPointer(0);
      size_t size;
      garbage_file->ReadObj(size);
      page_id_t max_page_id;
      garbage_file->ReadObj(max_page_id);
      max_page_id_ = max_page_id;
      for (size_t i = 0; i < size; ++i) {
        page_id_t page_id;
        garbage_file->ReadObj(page_id);
        queue_.push_back(page_id);
      }
    }
  }
  ~MyDiskManager() {
    garbage_file->SetWritePointer(0);
    size_t size = queue_.size();
    garbage_file->WriteObj(size);

    garbage_file->WriteObj(max_page_id_);
    for (size_t i = 0; i < size; ++i) {
      page_id_t page_id = queue_.back();
      queue_.pop_back();
      garbage_file->WriteObj(page_id);
    }
    delete garbage_file;
    delete data_file_;
  }
  void WritePage(page_id_t page_id, const char *page_data) {
    int offset = page_id * BUSTUB_PAGE_SIZE;
    data_file_->SetWritePointer(offset);
    data_file_->Write(page_data, BUSTUB_PAGE_SIZE);
  }
  void ReadPage(page_id_t page_id, char *page_data) {
    int offset = page_id * BUSTUB_PAGE_SIZE;
    data_file_->SetReadPointer(offset);
    data_file_->Read(page_data, BUSTUB_PAGE_SIZE);
  }
  auto AllocatePage() -> page_id_t {
    if (!queue_.empty()) {
      auto page_id = queue_.front();
      queue_.pop_front();
      return page_id;
    }
    return ++max_page_id_;
  }

  void DeallocatePage(page_id_t page_id) { queue_.push_back(page_id); }
  auto IsNew() -> bool { return garbage_file->IsNew(); }

 private:
  MyFile *data_file_{nullptr};
  MyFile *garbage_file{nullptr};  // 第一位size_，第二位max_page_id_

    list<page_id_t> queue_{};
  page_id_t max_page_id_{0};
};
}  // namespace CrazyDave
#endif  // BPT_PRO_DISK_MANAGER_H
