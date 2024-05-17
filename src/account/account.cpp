#include "account/account.hpp"
CrazyDave::Account::Account(const std::string &user_name, const std::string &password, const std::string &name,
                            const std::string &mail_addr, int privilege)
    : user_name_(user_name), password_(password), name_(name), mail_addr_(mail_addr), privilege_(privilege) {}

std::istream &CrazyDave::operator>>(std::istream &is, CrazyDave::Account &rhs) { return is; }
std::ostream &CrazyDave::operator<<(std::ostream &os, CrazyDave::Account &rhs) {
  std::cout << rhs.user_name_ << " " << rhs.name_ << " " << rhs.mail_addr_ << " " << rhs.privilege_ << "\n";
}
auto CrazyDave::AccountSystem::add_user(const std::string &cur_user_name, const std::string &user_name,
                                        const std::string &password, const std::string &name,
                                        const std::string &mail_addr, int privilege) -> bool {
  auto it = login_list_.find(cur_user_name);
  if (it == login_list_.end()) {
    return false;
  }
  auto &cur_user = account_storage_.find(user_name)[0];
  if (cur_user.privilege_ <= privilege) {
    return false;
  }
  Account user{user_name, password, name, mail_addr, privilege};
  account_storage_.insert(user.user_name_, user);
  return true;
}
auto CrazyDave::AccountSystem::login(const std::string &user_name, const std::string &password) -> bool {
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
auto CrazyDave::AccountSystem::logout(const std::string &user_name) -> bool {
  auto it = login_list_.find(user_name);
  if (it == login_list_.end()) {
    return false;
  }
  login_list_.erase(it);
  return true;
}
auto CrazyDave::AccountSystem::query_profile(const std::string &cur_user_name, const std::string &user_name) -> bool {
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
auto CrazyDave::AccountSystem::modify_profile(const std::string &cur_user_name, const std::string &user_name,
                                              const std::optional<std::string> &password,
                                              const std::optional<std::string> &name,
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
  return true;
}
