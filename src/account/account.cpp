#include "account/account.hpp"

namespace CrazyDave {
Account::Account(const std::string &user_name, const std::string &password, const std::string &name,
                 const std::string &mail_addr, int privilege)
    : user_name_(user_name), password_(password), name_(name), mail_addr_(mail_addr), privilege_(privilege) {}

std::ostream &operator<<(std::ostream &os, Account &rhs) {
  std::cout << rhs.user_name_ << " " << rhs.name_ << " " << rhs.mail_addr_ << " " << rhs.privilege_ << "\n";
  return os;
}
auto AccountSystem::add_user(const std::optional<std::string> &cur_user_name, const std::string &user_name,
                             const std::string &password, const std::string &name, const std::string &mail_addr,
                             std::optional<int> privilege) -> bool {
  if (!is_new_) {
    auto cur_hs = HashBytes(cur_user_name.value().c_str());
    auto it = login_list_.find(cur_hs);
    if (it == login_list_.end()) {
      return false;
    }
    vector<Account> cur_user_vec;
    account_storage_.find(cur_hs, cur_user_vec);
    auto &cur_user = cur_user_vec[0];
    if (cur_user.privilege_ <= privilege) {
      return false;
    }
  } else {
    is_new_ = false;
    privilege = 10;
  }
  Account user{user_name, password, name, mail_addr, privilege.value()};
  auto user_hs = HashBytes(user_name.c_str());
  account_storage_.insert(user_hs, user);
  return true;
}
auto AccountSystem::login(const std::string &user_name, const std::string &password) -> bool {
  auto user_hs = HashBytes(user_name.c_str());
  auto it = login_list_.find(user_hs);
  if (it != login_list_.end()) {
    return false;
  }
  vector<Account> user_vec;
  account_storage_.find(user_hs, user_vec);
  if (user_vec.empty() || user_vec[0].password_ != password) {
    return false;
  }
  login_list_.insert({user_hs, user_vec[0].privilege_});
  return true;
}
auto AccountSystem::logout(const std::string &user_name) -> bool {
  auto user_name_hs = HashBytes(user_name.c_str());
  auto it = login_list_.find(user_name_hs);
  if (it == login_list_.end()) {
    return false;
  }
  login_list_.erase(it);
  return true;
}
auto AccountSystem::query_profile(const std::string &cur_user_name, const std::string &user_name) -> bool {
  auto cur_hs = HashBytes(cur_user_name.c_str());
  auto it = login_list_.find(cur_hs);
  if (it == login_list_.end()) {
    return false;
  }
  auto user_hs = HashBytes(user_name.c_str());
  vector<Account> user_vec;
  account_storage_.find(user_hs, user_vec);
  if (user_vec.empty()) {
    return false;
  }
  auto &user = user_vec[0];
  if (cur_hs != user_hs) {
    if (it->second <= user.privilege_) {
      return false;
    }
  }
  std::cout << user;
  return true;
}
auto AccountSystem::modify_profile(const std::string &cur_user_name, const std::string &user_name,
                                   const std::optional<std::string> &password, const std::optional<std::string> &name,
                                   const std::optional<std::string> &mail_addr,
                                   const std::optional<int> privilege) -> bool {
  auto cur_hs = HashBytes(cur_user_name.c_str());
  auto it = login_list_.find(cur_hs);
  if (it == login_list_.end()) {
    return false;
  }
  auto user_hs = HashBytes(user_name.c_str());
  vector<Account> user_vec;
  account_storage_.find(user_hs, user_vec);
  if (user_vec.empty()) {
    return false;
  }
  auto &user = user_vec[0];

  if (cur_hs != user_hs && it->second <= user.privilege_) {
    return false;
  }
  if (privilege.has_value() && privilege >= it->second) {
    return false;
  }
  account_storage_.remove(user_hs, user);
  if (privilege.has_value()) {
    user.privilege_ = privilege.value();
  }
  if (password.has_value()) {
    user.password_ = password.value();
  }
  if (name.has_value()) {
    user.name_ = name.value();
  }
  if (mail_addr.has_value()) {
    user.mail_addr_ = mail_addr.value();
  }
  account_storage_.insert(user_hs, user);
  auto user_it = login_list_.find(user_hs);
  if (user_it != login_list_.end()) {
    user_it->second = user.privilege_;
  }
  std::cout << user;
  return true;
}
auto AccountSystem::check_is_login(const std::string &user_name) -> bool {
  auto user_name_hs = HashBytes(user_name.c_str());
  return login_list_.find(user_name_hs) != login_list_.end();
}
AccountSystem::AccountSystem() {
  header_.open();
  if (header_.get_is_new()) {
    return;
  }
  header_.seekg(0);
  header_.read(is_new_);
}
void AccountSystem::clear() {
  //  account_storage_.clear();
  login_list_.clear();
}
AccountSystem::~AccountSystem() {
  header_.seekp(0);
  header_.write(is_new_);
  header_.close();
}
void AccountSystem::load_management_system(ManagementSystem *m_sys) { m_sys_ = m_sys; }
}  // namespace CrazyDave