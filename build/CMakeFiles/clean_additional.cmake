# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "")
  file(REMOVE_RECURSE
  "CMakeFiles\\SistemPerpustakaan_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\SistemPerpustakaan_autogen.dir\\ParseCache.txt"
  "SistemPerpustakaan_autogen"
  )
endif()
