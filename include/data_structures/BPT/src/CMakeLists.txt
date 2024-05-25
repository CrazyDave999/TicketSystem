# Add source files to the project
set(SRC_FILES
        buffer/buffer_pool_manager.cpp
        buffer/lru_k_replacer.cpp
        storage/page/b_plus_tree_page.cpp
        storage/page/page_guard.cpp
        )

# Add the source directory to include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Add the source files to the project
add_library(BPT_src ${SRC_FILES})

# Link the library to the main executable
target_link_libraries(${PROJECT_NAME} PRIVATE BPT_src)