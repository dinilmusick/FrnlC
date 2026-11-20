cmake_minimum_required(VERSION 3.16)
project(FrnlC LANGUAGES C)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
add_library(frnl_event_dispatch STATIC src/frnl_event_dispatch.c)
target_include_directories(frnl_event_dispatch PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
include(CTest)
if(BUILD_TESTING)
  add_executable(frnl_event_dispatch_test tests/test_event_dispatch.c)
  target_link_libraries(frnl_event_dispatch_test PRIVATE frnl_event_dispatch)
  add_test(NAME frnl_event_dispatch COMMAND frnl_event_dispatch_test)
  add_executable(frnl_multi_event_dispatch_test tests/test_multi_event_dispatch.c)
  target_link_libraries(frnl_multi_event_dispatch_test PRIVATE frnl_event_dispatch)
  add_test(NAME frnl_multi_event_dispatch COMMAND frnl_multi_event_dispatch_test)
  add_executable(frnl_nested_failure_recovery_test tests/test_nested_failure_recovery.c)
  target_link_libraries(frnl_nested_failure_recovery_test PRIVATE frnl_event_dispatch)
  add_test(NAME frnl_nested_failure_recovery COMMAND frnl_nested_failure_recovery_test)
  add_executable(frnl_handler_registry_mutation_test tests/test_handler_registry_mutation.c)
  target_link_libraries(frnl_handler_registry_mutation_test PRIVATE frnl_event_dispatch)
  add_test(NAME frnl_handler_registry_mutation COMMAND frnl_handler_registry_mutation_test)
  add_executable(frnl_registry_lifecycle_test tests/test_registry_lifecycle.c)
  target_link_libraries(frnl_registry_lifecycle_test PRIVATE frnl_event_dispatch)
  add_test(NAME frnl_registry_lifecycle COMMAND frnl_registry_lifecycle_test)
  # Release builds define NDEBUG, which otherwise turns assert()-based tests
  # into no-ops. Keep assertions enabled in every CTest executable.
  foreach(test_target IN ITEMS
      frnl_event_dispatch_test
      frnl_multi_event_dispatch_test
      frnl_nested_failure_recovery_test
      frnl_handler_registry_mutation_test
      frnl_registry_lifecycle_test)
    if(MSVC)
      target_compile_options(${test_target} PRIVATE /UNDEBUG)
    else()
      target_compile_options(${test_target} PRIVATE -UNDEBUG)
    endif()
  endforeach()
endif()
