cmake_minimum_required (VERSION 3.2 FATAL_ERROR)
project (reverse-engineering-dungeon-siege VERSION 0.1 LANGUAGES CXX)

file (GLOB_RECURSE SIEGE_SOURCE "source/siege/*.cpp")
file (GLOB_RECURSE UTILS_SOURCE "source/utils/*.cpp")

include_directories ("source")

add_executable (asp2obj "source/tools/asp2obj/asp2obj.cpp" ${SIEGE_SOURCE} ${UTILS_SOURCE})
# add_executable (raw2x "source/tools/raw2x/raw2x.cpp" ${SIEGE_SOURCE} ${UTILS_SOURCE})
add_executable (sno2obj "source/tools/sno2obj/sno2obj.cpp" ${SIEGE_SOURCE} ${UTILS_SOURCE})
add_executable (tankdump "source/tools/tankdump/tankdump.cpp" ${SIEGE_SOURCE} ${UTILS_SOURCE})