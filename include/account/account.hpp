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
  friend std::istream &operator>>(std::istream &is, Account &rhs);
  friend std::ostream &operator<<(std::ostream &os, Account &rhs);
  auto operator<(const Account &rhs) const -> bool { return user_name_ < rhs.user_name_; }
};

class AccountSystem {
 private:
  BPlusTree<String<21>, Account> account_storage_{"tmp/ac1", "tmp/ac2", "tmp/ac3", "tmp/ac4"};
  linked_hashmap<String<21>, int, HashString<21>> login_list_;

 public:
  auto add_user(const std::string &cur_user_name, const std::string &user_name, const std::string &password,
                const std ::string &name, const std::string &mail_addr, int privilege) -> bool;
  auto login(const std::string &user_name, const std::string &password) -> bool;
  auto logout(const std::string &user_name) -> bool;
  auto query_profile(const std::string &cur_user_name, const std::string &user_name) -> bool;
  auto modify_profile(const std::string &cur_user_name, const std::string &user_name,
                      const std::optional<std::string> &password, const std::optional<std::string> &name,
                      const std::optional<std::string> &mail_addr, const std::optional<int> privilege) -> bool;
};
}  // namespace CrazyDave
#endif  // TICKET_SYSTEM_ACCOUNT_HPP
