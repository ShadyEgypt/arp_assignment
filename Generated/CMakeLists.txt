
cmake_minimum_required(VERSION 3.20)

project("generated_code")

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find requirements
find_package(fastcdr REQUIRED)
find_package(fastdds 3 REQUIRED)


message(STATUS "Configuring Target...")
add_library(Target_lib src/map/TargetTypeObjectSupport.cxx src/map/TargetPubSubTypes.cxx)
target_link_libraries(Target_lib fastcdr fastdds)

add_executable(Target src/map/TargetApplication.cxx src/map/TargetPublisherApp.cxx src/map/TargetSubscriberApp.cxx src/map/Targetmain.cxx)
target_link_libraries(Target fastcdr fastdds
        Target_lib 
        )



