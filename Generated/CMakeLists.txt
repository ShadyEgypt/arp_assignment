
cmake_minimum_required(VERSION 3.20)

project("generated_code")

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find requirements
find_package(fastcdr REQUIRED)
find_package(fastdds 3 REQUIRED)


message(STATUS "Configuring TargetMessage...")
add_library(TargetMessage_lib src/map/TargetMessageTypeObjectSupport.cxx src/map/TargetMessagePubSubTypes.cxx)
target_link_libraries(TargetMessage_lib fastcdr fastdds)

add_executable(TargetMessage src/map/TargetMessageApplication.cxx src/map/TargetMessagePublisherApp.cxx src/map/TargetMessageSubscriberApp.cxx src/map/TargetMessagemain.cxx)
target_link_libraries(TargetMessage fastcdr fastdds
        TargetMessage_lib 
        )



