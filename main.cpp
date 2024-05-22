#include "common/management_system.hpp"
CrazyDave::ManagementSystem m_sys;
int main() {
#ifdef DEBUG_FILE_IN_TMP
//  system("rm -rf ./tmp/*");
  std::freopen("../test/testcases/basic_6/25.in", "r", stdin);
//  std::freopen("../tracking", "r", stdin);
  std::freopen("../output.txt", "w", stdout);
#endif

  std::ios::sync_with_stdio(false);
  m_sys.run();

#ifdef DEBUG_FILE_IN_TMP
//  system("rm -rf ./tmp/*");
#endif
  return 0;
}