#include "common/management_system.hpp"
namespace CrazyDave {
auto ManagementSystem::check_is_login(const std::string &username) -> bool {
  return account_sys_->check_is_login(username);
}
void ManagementSystem::run() {
  std::string line;
  while (std::getline(std::cin, line)) {
    StringUtil::RTrim(&line);
    if (!execute_line(line)) {
      return;
    }
  }
}
auto ManagementSystem::execute_line(const std::string &line) -> bool {
  vector<std::string> tokens;
  StringUtil::Split(line, ' ', tokens);
  std::cout << tokens[0] << " ";
  enum OutputType { SIMPLE, F_SIMPLE, NORMAL } output_type = SIMPLE;
  bool success = false;
  auto &command = tokens[1];
  if (command == "add_user") {
    std::optional<std::string> cur_user_name;
    std::optional<int> privilege;
    std::string user_name, password, name, mail_addr;
    for (int i = 2; i < (int)tokens.size(); i += 2) {
      auto &key = tokens[i];
      auto &value = tokens[i + 1];
      if (key == "-c") {
        cur_user_name = value;
      } else if (key == "-u") {
        user_name = value;
      } else if (key[1] == 'p') {
        password = value;
      } else if (key[1] == 'n') {
        name = value;
      } else if (key[1] == 'm') {
        mail_addr = value;
      } else if (key == "-g") {
        privilege = std::stoi(value);
      }
    }
    output_type = SIMPLE;
    success = account_sys_->add_user(cur_user_name, user_name, password, name, mail_addr, privilege);
  } else if (command == "login") {
    std::string user_name, password;
    for (int i = 2; i < (int)tokens.size(); i += 2) {
      auto &key = tokens[i];
      auto &value = tokens[i + 1];
      if (key == "-u") {
        user_name = value;
      } else if (key[1] == 'p') {
        password = value;
      }
    }
    output_type = SIMPLE;
    success = account_sys_->login(user_name, password);
  } else if (command == "logout") {
    std::string user_name = tokens[3];
    output_type = SIMPLE;
    success = account_sys_->logout(user_name);
  } else if (command == "query_profile") {
    std::string cur_user_name, user_name;
    for (int i = 2; i < (int)tokens.size(); i += 2) {
      auto &key = tokens[i];
      auto &value = tokens[i + 1];
      if (key == "-c") {
        cur_user_name = value;
      } else if (key == "-u") {
        user_name = value;
      }
    }
    output_type = F_SIMPLE;
    success = account_sys_->query_profile(cur_user_name, user_name);
  } else if (command == "modify_profile") {
    std::string cur_user_name, user_name;
    std::optional<std::string> password, name, mail_addr;
    std::optional<int> privilege;
    for (int i = 2; i < (int)tokens.size(); i += 2) {
      auto &key = tokens[i];
      auto &value = tokens[i + 1];
      if (key == "-c") {
        cur_user_name = value;
      } else if (key == "-u") {
        user_name = value;
      } else if (key[1] == 'p') {
        password = value;
      } else if (key[1] == 'n') {
        name = value;
      } else if (key[1] == 'm') {
        mail_addr = value;
      } else if (key == "-g") {
        privilege = std::stoi(value);
      }
    }
    output_type = F_SIMPLE;
    success = account_sys_->modify_profile(cur_user_name, user_name, password, name, mail_addr, privilege);
  } else if (command == "add_train") {
    std::string train_id;
    int seat_num{};
    vector<std::string> stations;
    vector<int> prices;
    Time start_time;
    vector<int> travel_times, stopover_times;
    DateRange sale_date;
    char type{};
    for (int i = 2; i < (int)tokens.size(); i += 2) {
      auto &key = tokens[i];
      auto &value = tokens[i + 1];
      if (key[1] == 'i') {
        train_id = value;
      } else if (key[1] == 'm') {
        seat_num = std::stoi(value);
      } else if (key[1] == 's') {
        StringUtil::Split(value, '|', stations);
      } else if (key[1] == 'p') {
        vector<std::string> vec;
        StringUtil::Split(value, '|', vec);
        for (auto &str : vec) {
          prices.push_back(std::stoi(str));
        }
      } else if (key[1] == 'x') {
        start_time = Time{value};
      } else if (key[1] == 't') {
        vector<std::string> vec;
        StringUtil::Split(value, '|', vec);
        for (auto &str : vec) {
          travel_times.push_back(std::stoi(str));
        }
      } else if (key == "-o") {
        if (value != "_") {
          vector<std::string> vec;
          StringUtil::Split(value, '|', vec);
          for (auto &str : vec) {
            stopover_times.push_back(std::stoi(str));
          }
        }
      } else if (key[1] == 'd') {
        vector<std::string> vec;
        StringUtil::Split(value, '|', vec);
        sale_date = DateRange{Date{vec[0]}, Date{vec[1]}};
      } else if (key[1] == 'y') {
        type = value[0];
      }
    }
    output_type = SIMPLE;
    success = train_sys_->add_train(train_id, seat_num, stations, prices, start_time, travel_times, stopover_times,
                                   sale_date, type);
  } else if (command == "delete_train") {
    std::string train_id = tokens[3];
    output_type = SIMPLE;
    success = train_sys_->delete_train(train_id);
  } else if (command == "release_train") {
    std::string &train_id = tokens[3];
    output_type = SIMPLE;
    success = train_sys_->release_train(train_id);
  } else if (command == "query_train") {
    std::string train_id;
    Date date;
    for (int i = 2; i < (int)tokens.size(); i += 2) {
      auto &key = tokens[i];
      auto &value = tokens[i + 1];
      if (key[1] == 'i') {
        train_id = value;
      } else if (key[1] == 'd') {
        date = Date{value};
      }
    }
    output_type = F_SIMPLE;
    success = train_sys_->query_train(train_id, date);
  } else if (command == "query_ticket") {
    std::string station1, station2;
    Date date;
    QueryType type = QueryType::TIME;
    for (int i = 2; i < (int)tokens.size(); i += 2) {
      auto &key = tokens[i];
      auto &value = tokens[i + 1];
      if (key[1] == 's') {
        station1 = value;
      } else if (key[1] == 't') {
        station2 = value;
      } else if (key[1] == 'd') {
        date = Date{value};
      } else if (key[1] == 'p') {
        if (value[0] == 't') {
          type = QueryType::TIME;
        } else {
          type = QueryType::COST;
        }
      }
    }
    output_type = NORMAL;
    train_sys_->query_ticket(station1, station2, date, type);
  } else if (command == "query_transfer") {
    std::string station1, station2;
    Date date;
    QueryType type = QueryType::TIME;
    for (int i = 2; i < (int)tokens.size(); i += 2) {
      auto &key = tokens[i];
      auto &value = tokens[i + 1];
      if (key[1] == 's') {
        station1 = value;
      } else if (key[1] == 't') {
        station2 = value;
      } else if (key[1] == 'd') {
        date = Date{value};
      } else if (key[1] == 'p') {
        if (value[0] == 't') {
          type = QueryType::TIME;
        } else if (value == "cost") {
          type = QueryType::COST;
        }
      }
    }
    output_type = NORMAL;
    success = train_sys_->query_transfer(station1, station2, date, type);
    if (!success) {
      std::cout << "0\n";
    }
  } else if (command == "buy_ticket") {
    int time_stamp = std::stoi(tokens[0].substr(1, tokens[0].size() - 2));
    std::string user_name, train_id, station1, station2;
    Date date;
    int num{};
    bool wait = false;
    for (int i = 2; i < (int)tokens.size(); i += 2) {
      auto &key = tokens[i];
      auto &value = tokens[i + 1];
      if (key == "-u") {
        user_name = value;
      } else if (key[1] == 'i') {
        train_id = value;
      } else if (key[1] == 'd') {
        date = Date{value};
      } else if (key[1] == 'n') {
        num = std::stoi(value);
      } else if (key[1] == 'f') {
        station1 = value;
      } else if (key[1] == 't') {
        station2 = value;
      } else if (key[1] == 'q') {
        wait = value == "true";
      }
    }
    output_type = F_SIMPLE;
    success = train_sys_->buy_ticket(time_stamp, user_name, train_id, date, num, station1, station2, wait);
  } else if (command == "query_order") {
    std::string &user_name = tokens[3];
    output_type = F_SIMPLE;
    success = train_sys_->query_order(user_name);
  } else if (command == "refund_ticket") {
    std::string user_name;
    int n{};
    for (int i = 2; i < (int)tokens.size(); i += 2) {
      auto &key = tokens[i];
      auto &value = tokens[i + 1];
      if (key[1] == 'u') {
        user_name = value;
      } else if (key[1] == 'n') {
        n = std::stoi(value);
      }
    }
    output_type = SIMPLE;
    success = train_sys_->refund_ticket(user_name, n);
  } else if (command == "clean") {
    output_type = NORMAL;
    account_sys_->clear();
    train_sys_->clear();
    std::cout << "0\n";
  } else if (command == "exit") {
    std::cout << "bye\n";
    return false;
  }
#ifdef DEBUG_FILE_IN_TMP
  else if (command == "print_queue") {
    train_sys_->print_queue();
    return true;
  }
#endif
  if (output_type == SIMPLE) {
    std::cout << (success ? "0\n" : "-1\n");
  } else if (output_type == F_SIMPLE) {
    if (!success) {
      std::cout << "-1\n";
    }
  }
  return true;
}
ManagementSystem::ManagementSystem(AccountSystem *account_sys, TrainSystem *train_sys)
    : account_sys_{account_sys}, train_sys_{train_sys} {
}
ManagementSystem::~ManagementSystem() = default;
}  // namespace CrazyDave