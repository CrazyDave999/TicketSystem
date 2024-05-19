#ifndef TICKETSYSTEM_MANAGEMENT_SYSTEM_HPP
#define TICKETSYSTEM_MANAGEMENT_SYSTEM_HPP
#include <optional>
#include <string>
#include "account/account.hpp"
#include "common/string_utils.hpp"
#include "train/train.hpp"
namespace CrazyDave {
class AccountSystem;
class TrainSystem;
class ManagementSystem {
 private:
  AccountSystem account_sys_{this};
  TrainSystem train_sys_{this};
  void execute_line(const std::string &line);

 public:
  ManagementSystem()= default;
  void run();
  auto check_is_login(const std::string &username) -> bool;
};
}  // namespace CrazyDave
#endif  // TICKETSYSTEM_MANAGEMENT_SYSTEM_HPP
