#ifndef TICKETSYSTEM_MANAGEMENT_SYSTEM_HPP
#define TICKETSYSTEM_MANAGEMENT_SYSTEM_HPP
#include <optional>
#include <string>
#include "account/account.hpp"
#include "common/string_utils.hpp"
#include "train/queue_system.hpp"
#include "train/train.hpp"
#include <fstream>

namespace CrazyDave {
class AccountSystem;
class TrainSystem;
class ManagementSystem {
 private:
  AccountSystem *account_sys_;
  TrainSystem *train_sys_;
  auto execute_line(const std::string &line) -> bool;

 public:
  ManagementSystem(AccountSystem *account_sys, TrainSystem *train_sys);
  ~ManagementSystem();
  void run();
  auto check_is_login(const std::string &username) -> bool;
};
}  // namespace CrazyDave
#endif  // TICKETSYSTEM_MANAGEMENT_SYSTEM_HPP
