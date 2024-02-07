# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles\\HttpClient_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\HttpClient_autogen.dir\\ParseCache.txt"
  "HttpClient_autogen"
  )
endif()
