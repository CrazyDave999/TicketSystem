#include "common/management_system.hpp"
namespace CrazyDave {
auto ManagementSystem::check_is_login(const std::string &username) -> bool {
  return account_sys_->check_is_login(username);
}
void ManagementSystem::run() {
  std::ios::sync_with_stdio(false);
  std::string line;
  while (std::getline(std::cin, line)) {
    StringUtil::RTrim(&line);
    if (!execute_line(line)) {
      return;
    }
  }
}
auto ManagementSystem::execute_line(const std::string &line) -> bool {
  auto tokens = StringUtil::Split(line, ' ');
  std::cout << tokens[0] << " ";
  enum OutputType { SIMPLE, F_SIMPLE, NORMAL } output_type = SIMPLE;
  bool success = false;
  if (tokens[1] == "add_user") {
    std::optional<std::string> cur_user_name;
    std::optional<int> privilege;
    std::string user_name, password, name, mail_addr;
    for (int i = 2; i < tokens.size(); i += 2) {
      auto &key = tokens[i];
      auto &value = tokens[i + 1];
      if (key == "-c") {
        cur_user_name = value;
      } else if (key == "-u") {
        user_name = value;
      } else if (key == "-p") {
        password = value;
      } else if (key == "-n") {
        name = value;
      } else if (key == "-m") {
        mail_addr = value;
      } else if (key == "-g") {
        privilege = std::stoi(value);
      }
    }
    output_type = SIMPLE;
    success = account_sys_->add_user(cur_user_name, user_name, password, name, mail_addr, privilege);
  } else if (tokens[1] == "login") {
    std::string user_name, password;
    for (int i = 2; i < tokens.size(); i += 2) {
      auto &key = tokens[i];
      auto &value = tokens[i + 1];
      if (key == "-u") {
        user_name = value;
      } else if (key == "-p") {
        password = value;
      }
    }
    output_type = SIMPLE;
    success = account_sys_->login(user_name, password);
  } else if (tokens[1] == "logout") {
    std::string user_name = tokens[3];
    output_type = SIMPLE;
    success = account_sys_->logout(user_name);
  } else if (tokens[1] == "query_profile") {
    std::string cur_user_name, user_name;
    for (int i = 2; i < tokens.size(); i += 2) {
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
  } else if (tokens[1] == "modify_profile") {
    std::string cur_user_name, user_name;
    std::optional<std::string> password, name, mail_addr;
    std::optional<int> privilege;
    for (int i = 2; i < tokens.size(); i += 2) {
      auto &key = tokens[i];
      auto &value = tokens[i + 1];
      if (key == "-c") {
        cur_user_name = value;
      } else if (key == "-u") {
        user_name = value;
      } else if (key == "-p") {
        password = value;
      } else if (key == "-n") {
        name = value;
      } else if (key == "-m") {
        mail_addr = value;
      } else if (key == "-g") {
        privilege = std::stoi(value);
      }
    }
    output_type = F_SIMPLE;
    success = account_sys_->modify_profile(cur_user_name, user_name, password, name, mail_addr, privilege);
  } else if (tokens[1] == "add_train") {
    std::string train_id;
    int seat_num;
    vector<std::string> stations;
    vector<int> prices;
    Time start_time;
    vector<int> travel_times, stopover_times;
    DateRange sale_date;
    char type;
    for (int i = 2; i < tokens.size(); i += 2) {
      auto &key = tokens[i];
      auto &value = tokens[i + 1];
      if (key == "-i") {
        train_id = value;
      } else if (key == "-m") {
        seat_num = std::stoi(value);
      } else if (key == "-s") {
        stations = StringUtil::Split(value, '|');
      } else if (key == "-p") {
        auto vec = StringUtil::Split(value, '|');
        for (auto &str : vec) {
          prices.push_back(std::stoi(str));
        }
      } else if (key == "-x") {
        start_time = Time{value};
      } else if (key == "-t") {
        auto vec = StringUtil::Split(value, '|');
        for (auto &str : vec) {
          travel_times.push_back(std::stoi(str));
        }
      } else if (key == "-o") {
        if (value != "_") {
          auto vec = StringUtil::Split(value, '|');
          for (auto &str : vec) {
            stopover_times.push_back(std::stoi(str));
          }
        }
      } else if (key == "-d") {
        auto vec = StringUtil::Split(value, '|');
        sale_date = DateRange{Date{vec[0]}, Date{vec[1]}};
      } else if (key == "-y") {
        type = value[0];
      }
    }
    output_type = SIMPLE;
    success = train_sys_->add_train(train_id, seat_num, stations, prices, start_time, travel_times, stopover_times,
                                    sale_date, type);
  } else if (tokens[1] == "delete_train") {
    std::string train_id = tokens[3];
    output_type = SIMPLE;
    success = train_sys_->delete_train(train_id);
  } else if (tokens[1] == "release_train") {
    std::string train_id = tokens[3];
    output_type = SIMPLE;
    success = train_sys_->release_train(train_id);
  } else if (tokens[1] == "query_train") {
    std::string train_id;
    Date date;
    for (int i = 2; i < tokens.size(); i += 2) {
      auto &key = tokens[i];
      auto &value = tokens[i + 1];
      if (key == "-i") {
        train_id = value;
      } else if (key == "-d") {
        date = Date{value};
      }
    }
    output_type = F_SIMPLE;
    success = train_sys_->query_train(train_id, date);
  } else if (tokens[1] == "query_ticket") {
    std::string station1, station2;
    Date date;
    QueryType type = QueryType::TIME;
    for (int i = 2; i < tokens.size(); i += 2) {
      auto &key = tokens[i];
      auto &value = tokens[i + 1];
      if (key == "-s") {
        station1 = value;
      } else if (key == "-t") {
        station2 = value;
      } else if (key == "-d") {
        date = Date{value};
      } else if (key == "-p") {
        if (value == "time") {
          type = QueryType::TIME;
        } else if (value == "cost") {
          type = QueryType::COST;
        }
      }
    }
    output_type = NORMAL;
    train_sys_->query_ticket(station1, station2, date, type);
  } else if (tokens[1] == "query_transfer") {
    std::string station1, station2;
    Date date;
    QueryType type = QueryType::TIME;
    for (int i = 2; i < tokens.size(); i += 2) {
      auto &key = tokens[i];
      auto &value = tokens[i + 1];
      if (key == "-s") {
        station1 = value;
      } else if (key == "-t") {
        station2 = value;
      } else if (key == "-d") {
        date = Date{value};
      } else if (key == "-p") {
        if (value == "time") {
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
  } else if (tokens[1] == "buy_ticket") {
    int time_stamp = std::stoi(tokens[0].substr(1, tokens[0].size() - 2));
    std::string user_name, train_id, station1, station2;
    Date date;
    int num;
    bool wait = false;
    for (int i = 2; i < tokens.size(); i += 2) {
      auto &key = tokens[i];
      auto &value = tokens[i + 1];
      if (key == "-u") {
        user_name = value;
      } else if (key == "-i") {
        train_id = value;
      } else if (key == "-d") {
        date = Date{value};
      } else if (key == "-n") {
        num = std::stoi(value);
      } else if (key == "-f") {
        station1 = value;
      } else if (key == "-t") {
        station2 = value;
      } else if (key == "-q") {
        if (value == "true") {
          wait = true;
        } else {
          wait = false;
        }
      }
    }
    output_type = F_SIMPLE;
    success = train_sys_->buy_ticket(time_stamp, user_name, train_id, date, num, station1, station2, wait);

  } else if (tokens[1] == "query_order") {
    std::string user_name = tokens[3];
    output_type = F_SIMPLE;
    success = train_sys_->query_order(user_name);
  } else if (tokens[1] == "refund_ticket") {
    std::string user_name;
    int n;
    for (int i = 2; i < tokens.size(); i += 2) {
      auto &key = tokens[i];
      auto &value = tokens[i + 1];
      if (key == "-u") {
        user_name = value;
      } else if (key == "-n") {
        n = std::stoi(value);
      }
    }
    output_type = SIMPLE;
    success = train_sys_->refund_ticket(user_name, n);
  } else if (tokens[1] == "clean") {
    output_type = NORMAL;
    account_sys_->clear();
    train_sys_->clear();
    std::cout << "0\n";
  } else if (tokens[1] == "exit") {
    std::cout << "bye\n";
    return false;
  }
  if (output_type == SIMPLE) {
    std::cout << (success ? "0\n" : "-1\n");
  } else if (output_type == F_SIMPLE) {
    if (!success) {
      std::cout << "-1\n";
    }
  }
  return true;
}
ManagementSystem::ManagementSystem() {
  account_sys_ = new AccountSystem{this};
  train_sys_ = new TrainSystem{this};
}
ManagementSystem::~ManagementSystem() {
  delete account_sys_;
  delete train_sys_;
}
}  // namespace CrazyDave