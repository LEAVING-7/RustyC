macro(FindLLVM path_to_llvm)
  set(LLVM_DIR ${path_to_llvm}/lib/cmake/llvm)
  if(path_to_llvm STREQUAL "")
    find_package(LLVM REQUIRED CONFIG)
  else()
    find_package(LLVM REQUIRED CONFIG HINT ${path_to_llvm})
  endif()
  list(APPEND CMAKE_MODULE_PATH ${LLVM_DIR})
endmacro()
