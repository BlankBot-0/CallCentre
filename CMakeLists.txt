cmake_minimum_required(VERSION 3.22.1)
set(CMAKE_CXX_STANDARD 20)
project(ProjectName)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
configure_file(${CMAKE_SOURCE_DIR}/config/config.json ${CMAKE_BINARY_DIR}/config.json COPYONLY)

include(FetchContent)

set (FETCHCONTENT_QUIET FALSE)

SET(Boost_USE_STATIC_LIBS ON)
find_package(Boost 1.81.0 REQUIRED COMPONENTS beast)
find_package(nlohmann_json 3.11.3 REQUIRED)

set(Boost_INCLUDE_DIRS  ${Boost_INCLUDE_DIR} build/_deps/boost-src)

set(PROJ_INCLUDE_DIRS
        ${Boost_INCLUDE_DIRS}
        cmake-build-debug/_deps/json-src/include
)
set(PROJ_LINK_LIBRARIES
        ${Boost_LIBRARIES}
        Boost::beast
        Boost::asio
        Boost::log
        Boost::assert
        Boost::bind
        Boost::config
        Boost::container
        Boost::core
        Boost::endian
        Boost::intrusive
        Boost::logic
        Boost::mp11
        Boost::optional
        Boost::smart_ptr
        Boost::static_assert
#        Boost::static_string
        Boost::system
        Boost::throw_exception
        Boost::type_traits
        Boost::utility
        ${Boost_LIBRARY_DIRS}
        nlohmann_json::nlohmann_json
)
# MESSAGE("${Boost_LIBRARY_DIRS}")
MESSAGE("BEBRA")
message("Boost_INCLUDE_DIRS: ${Boost_INCLUDE_DIRS}")
add_subdirectory(cpp)