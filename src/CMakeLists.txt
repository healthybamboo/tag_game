cmake_minimum_required(VERSION 2.8)
project(tag_game)

add_subdirectory(libs)
add_executable(tags tags.c)
add_executable(tagc tagc.c)
add_executable(disp disp.c)

target_link_libraries(tags communication game utils)
target_link_libraries(tagc communication game utils)
target_link_libraries(disp communication game utils)