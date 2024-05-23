#ifndef TICKET_SYSTEM_CONFIG_HPP
#define TICKET_SYSTEM_CONFIG_HPP

//#define DEBUG_QUERY_TRANSFER
//#define DEBUG_BUY_TICKET
#define DEBUG_FILE_IN_TMP
//#define DEBUG_TRACKING
namespace CrazyDave {
static constexpr int DAY_NUM[13] = {0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31};
static constexpr int DAY_PREFIX[13] = {0, 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335};
}  // namespace CrazyDave
#endif  // TICKET_SYSTEM_CONFIG_HPP
