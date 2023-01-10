cmake_minimum_required(VERSION 2.8)

add_library(game STATIC game.c)
add_library(communication STATIC communication.c)
add_library(utils STATIC utils.c)