cmake_minimum_required(VERSION 3.31.6)

project(
  srd
  VERSION 1.0.0
  LANGUAGES CXX
)

set(
  SRCS src/srd.cpp
)

add_executable(srd ${SRCS})
