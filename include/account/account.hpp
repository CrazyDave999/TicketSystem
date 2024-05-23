#ifndef TICKET_SYSTEM_ACCOUNT_HPP
#define TICKET_SYSTEM_ACCOUNT_HPP
#include <iostream>
#include <optional>
#include <string>
#include "common/utils.hpp"
#include "data_structures/BPT.hpp"
#include "data_structures/linked_hashmap.h"
namespace CrazyDave {
class AccountSystem;
class Account {
  friend AccountSystem;

 private:
  String<21> user_name_;
  String<31> password_;
  String<21> name_;
  String<31> mail_addr_;
  int privilege_{};

 public:
  Account() = default;
  Account(const std::string &user_name, const std::string &password, const std::string &name,
          const std::string &mail_addr, int privilege);
  friend std::ostream &operator<<(std::ostream &os, Account &rhs);
  auto operator<(const Account &rhs) const -> bool { return user_name_ < rhs.user_name_; }
};
class ManagementSystem;
class AccountSystem {
 private:
#ifdef DEBUG_FILE_IN_TMP
  BPlusTree<size_t , Account> account_storage_{"tmp/ac1", "tmp/ac2", "tmp/ac3", "tmp/ac4"};
#else
  BPlusTree<size_t , Account> account_storage_{"ac1", "ac2", "ac3", "ac4"};
#endif
  linked_hashmap<size_t , int> login_list_;
  ManagementSystem *m_sys_{};
#ifdef DEBUG_FILE_IN_TMP
  File header_{"tmp/hd"};
#else
  File header_{"hd"};
#endif
  bool is_new_{true};

 public:
  AccountSystem()=default;
  explicit AccountSystem(ManagementSystem *m_sys);
  void load_management_system(ManagementSystem *m_sys);
  ~AccountSystem();
  auto check_is_login(const std::string &user_name) -> bool;
  auto add_user(const std::optional<std::string> &cur_user_name, const std::string &user_name,
                const std::string &password, const std::string &name, const std::string &mail_addr,
                std::optional<int> privilege) -> bool;
  auto login(const std::string &user_name, const std::string &password) -> bool;
  auto logout(const std::string &user_name) -> bool;
  auto query_profile(const std::string &cur_user_name, const std::string &user_name) -> bool;
  auto modify_profile(const std::string &cur_user_name, const std::string &user_name,
                      const std::optional<std::string> &password, const std::optional<std::string> &name,
                      const std::optional<std::string> &mail_addr, std::optional<int> privilege) -> bool;
  void clear();
};
}  // namespace CrazyDave
#endif  // TICKET_SYSTEM_ACCOUNT_HPP
