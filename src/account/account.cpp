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
    auto it = login_list_.find(cur_user_name.value());
    if (it == login_list_.end()) {
      return false;
    }
    auto &cur_user = account_storage_.find(cur_user_name.value())[0];
    if (cur_user.privilege_ <= privilege) {
      return false;
    }
  } else {
    is_new_ = false;
    privilege = 10;
  }
  Account user{user_name, password, name, mail_addr, privilege.value()};
  account_storage_.insert(user.user_name_, user);
  return true;
}
auto AccountSystem::login(const std::string &user_name, const std::string &password) -> bool {
  auto it = login_list_.find(user_name);
  if (it != login_list_.end()) {
    return false;
  }
  auto user_vec = account_storage_.find(user_name);
  if (user_vec.empty() || user_vec[0].password_ != password) {
    return false;
  }
  login_list_.insert({user_name, 1});
  return true;
}
auto AccountSystem::logout(const std::string &user_name) -> bool {
  auto it = login_list_.find(user_name);
  if (it == login_list_.end()) {
    return false;
  }
  login_list_.erase(it);
  return true;
}
auto AccountSystem::query_profile(const std::string &cur_user_name, const std::string &user_name) -> bool {
  auto it = login_list_.find(cur_user_name);
  if (it == login_list_.end()) {
    return false;
  }
  auto user_vec = account_storage_.find(user_name);
  if (user_vec.empty()) {
    return false;
  }
  auto &user = user_vec[0];
  if (cur_user_name != user_name) {
    auto cur_user_vec = account_storage_.find(cur_user_name);
    auto &cur_user = cur_user_vec[0];
    if (cur_user.privilege_ <= user.privilege_) {
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
  auto it = login_list_.find(cur_user_name);
  if (it == login_list_.end()) {
    return false;
  }
  auto user_vec = account_storage_.find(user_name);
  if (user_vec.empty()) {
    return false;
  }
  auto &user = user_vec[0];
  auto cur_user_vec = account_storage_.find(cur_user_name);
  auto &cur_user = cur_user_vec[0];
  if (cur_user_name != user_name && cur_user.privilege_ <= user.privilege_) {
    return false;
  }

  account_storage_.remove(user_name, user);
  if (privilege.has_value()) {
    if (privilege >= cur_user.privilege_) {
      return false;
    }
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
  account_storage_.insert(user_name, user);
  std::cout << user;
  return true;
}
auto AccountSystem::check_is_login(const std::string &username) -> bool {
  return login_list_.find(username) != login_list_.end();
}
AccountSystem::AccountSystem(ManagementSystem *m_sys) : m_sys_(m_sys) {
  header_.open();
  if (header_.get_is_new()) {
    return;
  }
  header_.seekg(0);
  header_.read(is_new_);
}
void AccountSystem::clear() {
  account_storage_.clear();
  login_list_.clear();
}
AccountSystem::~AccountSystem() {
  header_.seekp(0);
  header_.write(is_new_);
  header_.close();
}
}  // namespace CrazyDave