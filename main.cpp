#include "common/management_system.hpp"
CrazyDave::ManagementSystem m_sys;
int main() {
  std::ios::sync_with_stdio(false);
#ifdef DEBUG_FILE_IN_TMP
//  system("rm -rf ./tmp/*");
  std::freopen("../test/testcases/basic_6/24.in", "r", stdin);
//  std::freopen("../tracking", "r", stdin);
  std::freopen("../output.txt", "w", stdout);
#endif

  m_sys.run();

#ifdef DEBUG_FILE_IN_TMP
//  system("rm -rf ./tmp/*");
#endif
  return 0;
}