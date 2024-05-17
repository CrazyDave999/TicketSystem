set(
        ACCOUNT_SRC_FILES
        account.cpp
)
add_library(Account SHARED ${ACCOUNT_SRC_FILES})
target_link_libraries(${PROJECT_NAME} Account)