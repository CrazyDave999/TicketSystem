#ifndef CRAZYDAVE_CONFIG_H
#define CRAZYDAVE_CONFIG_H
#include <cstdint>

namespace CrazyDave {

static constexpr int INVALID_PAGE_ID = -1;     // invalid page id
static constexpr int BUSTUB_PAGE_SIZE = 8192;  // size of a data page in byte
static constexpr int LRUK_REPLACER_K = 10;     // lookback window for lru-k replacer

using frame_id_t = int32_t;  // frame id type
using page_id_t = int32_t;   // page id type
}  // namespace CrazyDave
#endif