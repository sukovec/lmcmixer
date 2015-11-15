cmake_minimum_required(VERSION 3.2)
project(lmcmixer)

find_package(ALSA REQUIRED)


SET(sources main.cpp websock.cpp)
add_executable(${PROJECT_NAME} ${sources})
target_link_libraries(${PROJECT_NAME} ${ALSA_LIBRARY} jack pthread)

set_property(TARGET lmcmixer PROPERTY CXX_STANDARD 11)
