CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
# Gtest source archive to use
SET(GTEST_ARCHIVE gtest-1.7.0.zip)
# Where to download archive (if it doesn't exist)
SET(GTEST_DLSERVER http://googletest.googlecode.com/files)


PROJECT(commontypes)
INCLUDE_DIRECTORIES(include/typelib)
ADD_LIBRARY(commontypes src/genhash.c src/simplestring.c src/hashset.c src/list.c)
IF(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-strict-aliasing -Wextra -Wall -Wmissing-declarations")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-strict-aliasing")
ENDIF()
ENABLE_TESTING()
ADD_SUBDIRECTORY(test test)
