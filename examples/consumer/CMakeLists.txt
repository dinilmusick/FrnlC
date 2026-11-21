cmake_minimum_required(VERSION 3.16)
project(FrnlCInstalledConsumer LANGUAGES C)
find_package(FrnlC 0.2 CONFIG REQUIRED)
add_executable(frnlc_external main.c)
target_link_libraries(frnlc_external PRIVATE
  FrnlC::frnl_semantic FrnlC::frnl_scope FrnlC::frnl_event_dispatch)
set_target_properties(frnlc_external PROPERTIES C_STANDARD 11 C_STANDARD_REQUIRED YES C_EXTENSIONS NO)
enable_testing()
add_test(NAME external_consumer COMMAND frnlc_external)
