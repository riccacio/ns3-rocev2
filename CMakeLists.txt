cmake_minimum_required(VERSION 3.10)
project(ns3-rocev2)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# Percorsi NS-3 relativi
set(NS3_ROOT_DIR "${CMAKE_SOURCE_DIR}/../ns-3-dev")
set(NS3_BUILD_DIR "${NS3_ROOT_DIR}/build")
set(NS3_INCLUDE_DIR "${NS3_BUILD_DIR}/include")

# Includi headers NS-3
include_directories(
  ${NS3_INCLUDE_DIR}
  ${NS3_ROOT_DIR}/src/core
  ${NS3_ROOT_DIR}/src/network
  ${NS3_ROOT_DIR}/src/internet
  ${NS3_ROOT_DIR}/src/point-to-point
  ${NS3_ROOT_DIR}/src/applications
)

# Link alle librerie NS-3
link_directories(${NS3_BUILD_DIR}/lib)

# Aggiungi i tuoi file .cc come libreria
file(GLOB_RECURSE MODEL_SOURCES "model/*.cc")
add_library(rocev2 ${MODEL_SOURCES})

# Simulazione principale
add_executable(rocev2-skeleton examples/rocev2-skeleton.cc)
target_link_libraries(rocev2-skeleton
    rocev2
    ns3-dev-core-default
    ns3-dev-network-default
    ns3-dev-internet-default
    ns3-dev-point-to-point-default
    ns3-dev-applications-default
)