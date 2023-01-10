cmake_minimum_required(VERSION 2.8)
project(test_tag_game)

#Google TestのCmakeプロジェクトを追加し、Google Testをビルド
add_subdirectory(lib/googletest/googletest)

# ゲームロジック用のライブラリをコンパイル
add_library(game ../src/libs/game.c)

# 通信用のライブラリをコンパイル
add_library(communication ../src/libs/communication.c)

# その他のライブラリをコンパイル
add_library(utils ../src/libs/utils.c)

# myapp-test実行ファイルをビルド
add_executable(testGame TestGame.cpp)
add_executable(testCommunication TestCommunication.cpp)
add_executable(testUtils TestUtils.cpp)

# ライブラリをリンクする
target_link_libraries(testUtils utils gtest_main)
target_link_libraries(testGame game utils gtest_main)
target_link_libraries(testCommunication communication gtest_main)