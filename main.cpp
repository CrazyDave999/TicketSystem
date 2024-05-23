#include "account/account.hpp"
#include "common/management_system.hpp"
#include "train/train.hpp"
CrazyDave::TrainSystem t_sys;
CrazyDave::AccountSystem a_sys;
CrazyDave::ManagementSystem m_sys{&a_sys, &t_sys};

int main() {
  a_sys.load_management_system(&m_sys);
  t_sys.load_management_system(&m_sys);
  std::ios::sync_with_stdio(false);
#ifdef DEBUG_FILE_IN_TMP
  //  system("rm -rf ./tmp/*");
  std::freopen("../test/testcases/basic_3/4.in", "r", stdin);
  //  std::freopen("../tracking", "r", stdin);
  std::freopen("../output.txt", "w", stdout);
#endif

  m_sys.run();

#ifdef DEBUG_FILE_IN_TMP
//  system("rm -rf ./tmp/*");
#endif
  return 0;
}