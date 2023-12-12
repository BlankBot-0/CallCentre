project(CallCentre)
add_library(${PROJECT_NAME}_lib
        Server.h
        Server.cpp
        RequestQueue.h
        RequestQueue.cpp
        CallOperator.h
        CallOperator.cpp
        CDRwriter.h
        CDRwriter.cpp
        servUtils.h
        servUtils.cpp
)

add_executable(${PROJECT_NAME} main.cpp)
SET(Boost_USE_STATIC_LIBS ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/cmake-build-debug)
target_include_directories(${PROJECT_NAME}_lib PUBLIC ${PROJ_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME}_lib PUBLIC ${PROJ_LINK_LIBRARIES})
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJ_INCLUDE_DIRS} ${PROJECT_NAME}_lib)
target_link_libraries(${PROJECT_NAME} PUBLIC ${PROJ_LINK_LIBRARIES} ${PROJECT_NAME}_lib)