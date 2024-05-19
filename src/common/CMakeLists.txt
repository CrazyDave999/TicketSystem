set(
        COMMON_SRC_FILES
        utils.cpp
        management_system.cpp
        string_utils.cpp
)
add_library(Common SHARED ${COMMON_SRC_FILES})
target_link_libraries(${PROJECT_NAME} Common)