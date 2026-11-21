cmake_minimum_required(VERSION 3.16)
project(FrnlC VERSION 0.2.0 LANGUAGES C)
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

add_library(frnl_scope STATIC src/frnl_scope.c)
add_library(frnl_event_dispatch STATIC src/frnl_event_dispatch.c)
add_library(frnl_semantic STATIC src/frnl_semantic.c)
foreach(lib IN ITEMS frnl_scope frnl_event_dispatch frnl_semantic)
  target_compile_features(${lib} PUBLIC c_std_11)
  set_target_properties(${lib} PROPERTIES C_STANDARD 11 C_STANDARD_REQUIRED YES C_EXTENSIONS NO)
  target_include_directories(${lib} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
endforeach()

include(CTest)
if(BUILD_TESTING)
  function(frnl_test target file linked)
    add_executable(${target} ${file})
    if(NOT "${linked}" STREQUAL "")
      target_link_libraries(${target} PRIVATE ${linked})
    else()
      target_include_directories(${target} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
    endif()
    # The existing assertions are checks, not debug-only decorations.
    # Force them on in multi-config Release too.
    if(MSVC)
      target_compile_options(${target} PRIVATE /UNDEBUG)
    else()
      target_compile_options(${target} PRIVATE -UNDEBUG)
    endif()
    add_test(NAME ${target} COMMAND ${target})
  endfunction()
  frnl_test(frnl_scope_test tests/scope_test.c frnl_scope)
  frnl_test(frnl_event_dispatch_test tests/test_event_dispatch.c frnl_event_dispatch)
  frnl_test(frnl_multi_event_dispatch_test tests/test_multi_event_dispatch.c frnl_event_dispatch)
  frnl_test(frnl_nested_failure_recovery_test tests/test_nested_failure_recovery.c frnl_event_dispatch)
  frnl_test(frnl_handler_registry_mutation_test tests/test_handler_registry_mutation.c frnl_event_dispatch)
  frnl_test(frnl_registry_lifecycle_test tests/test_registry_lifecycle.c frnl_event_dispatch)
  frnl_test(frnl_ui_action_gate_test tests/test_ui_action_gate.c "")
  frnl_test(frnl_ui_action_executor_test tests/test_ui_action_executor.c "")
  frnl_test(frnl_semantic_test tests/test_frnl_semantic.c frnl_semantic)
endif()

install(TARGETS frnl_scope frnl_event_dispatch frnl_semantic EXPORT FrnlCTargets
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(EXPORT FrnlCTargets NAMESPACE FrnlC:: DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/FrnlC)
configure_package_config_file(cmake/FrnlCConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/FrnlCConfig.cmake
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/FrnlC)
write_basic_package_version_file(${CMAKE_CURRENT_BINARY_DIR}/FrnlCConfigVersion.cmake
  VERSION ${PROJECT_VERSION} COMPATIBILITY SameMajorVersion)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/FrnlCConfig.cmake
              ${CMAKE_CURRENT_BINARY_DIR}/FrnlCConfigVersion.cmake
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/FrnlC)
