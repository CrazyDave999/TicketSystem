set(
        TRAIN_SRC_FILES
        train.cpp
        queue_system.cpp
)
add_library(Train SHARED ${TRAIN_SRC_FILES})
target_link_libraries(${PROJECT_NAME} Train)