#include "gtest/gtest.h"

extern "C" {
#include "../src/libs/game.h"
}

/* <移動に関するテスト>*/
// セットアップ用の関数(playersの0が逃走者、1が鬼)
void setup(int board[BOARD_SIZE][BOARD_SIZE], player_t players[PLAYER_NUM],
           int fugitive_x, int fugitive_y, int hunter_x, int hunter_y) {
  // ボードの初期化(ボードは場所を管理するための配列)
  for (int i = 0; i < BOARD_SIZE; i++) {
    for (int j = 0; j < BOARD_SIZE; j++) {
      board[i][j] = -1;
    }
  }

  // 逃走者の初期位置を設定
  board[fugitive_y][fugitive_x] = 0;
  // 鬼の初期位置を設定
  board[hunter_y][hunter_x] = 1;

  // playersはプレイヤーの役割を管理する(indexがプレイヤー番号)
  players[0].x = fugitive_x;
  players[0].y = fugitive_y;
  players[0].is_hunter = 0;
  players[1].x = hunter_x;
  players[1].y = hunter_y;
  players[1].is_hunter = 1;
};

// ボードとプレイヤーの情報をコピーする
void copy_boad_and_players(int board[BOARD_SIZE][BOARD_SIZE],
                           player_t players[PLAYER_NUM],
                           int board_copy[BOARD_SIZE][BOARD_SIZE],
                           player_t players_copy[PLAYER_NUM]) {
  // ボードとプレイヤーの情報をコピー
  for (int i = 0; i < BOARD_SIZE; i++) {
    for (int j = 0; j < BOARD_SIZE; j++) {
      board_copy[i][j] = board[i][j];
    }
  }

  // プレイヤーの情報をコピー
  for (int i = 0; i < PLAYER_NUM; i++) {
    players_copy->x = players->x;
    players_copy->y = players->y;
    players_copy->is_hunter = players->is_hunter;
  }
}

// 必要な変数を定義
class BaseVariable : public ::testing::Test {
 protected:
  int x;
  int y;
  int mover;
  int board[BOARD_SIZE][BOARD_SIZE];
  int board_copy[BOARD_SIZE][BOARD_SIZE];
  player_t players[PLAYER_NUM];
  player_t players_copy[PLAYER_NUM];
};

// 盤上左上にいる場合のフィクチャ
class MoveFromBoardLeftTop : public ::BaseVariable {
 protected:
  virtual void SetUp() {
    mover = 0;
    x = 0;
    y = 0;
    setup(board, players, x, y, BOARD_SIZE - 1 - x, BOARD_SIZE - 1 - y);
    copy_boad_and_players(board, players, board_copy, players_copy);
  }
};

// 盤上左下にいる場合のフィクチャ
class MoveFromBoardLeftBottom : public ::BaseVariable {
 protected:
  virtual void SetUp() {
    mover = 0;
    x = 0;
    y = BOARD_SIZE - 1;
    setup(board, players, x, y, BOARD_SIZE - 1 - x, BOARD_SIZE - 1 - y);
    copy_boad_and_players(board, players, board_copy, players_copy);
  }
};
// 盤上右下にいる場合のフィクチャ
class MoveFromBoardRightBottom : public ::BaseVariable {
 protected:
  virtual void SetUp() {
    mover = 0;
    x = BOARD_SIZE - 1;
    y = BOARD_SIZE - 1;
    setup(board, players, x, y, BOARD_SIZE - 1 - x, BOARD_SIZE - 1 - y);
    copy_boad_and_players(board, players, board_copy, players_copy);
  }
};
// 盤上右上にいる場合のフィクチャ
class MoveFromBoardRightTop : public ::BaseVariable {
 protected:
  virtual void SetUp() {
    mover = 0;
    x = BOARD_SIZE - 1;
    y = 0;
    setup(board, players, x, y, BOARD_SIZE - 1 - x, BOARD_SIZE - 1 - y);
    copy_boad_and_players(board, players, board_copy, players_copy);
  }
};

// 盤上左上から上へ移動しようとした場合、移動ができないことを確認する
TEST_F(MoveFromBoardLeftTop, to_up) {
  move_up(board, players, mover);

  // 盤上の位置情報が変更されていないことを確認
  EXPECT_EQ(memcmp(board, board_copy, sizeof(board)), 0);

  // プレイヤーの位置情報が変更されていないことを確認
  EXPECT_EQ(players[mover].x, players_copy[mover].x);
  EXPECT_EQ(players[mover].y, players_copy[mover].y);
}

// 盤上左上から左へ移動しようとした場合、移動ができないことを確認する
TEST_F(MoveFromBoardLeftTop, to_left) {
  move_left(board, players, mover);

  // 盤上の位置情報が変更されていないことを確認
  EXPECT_EQ(memcmp(board, board_copy, sizeof(board)), 0);

  // プレイヤーの位置情報が変更されていないことを確認
  EXPECT_EQ(players[mover].x, players_copy[mover].x);
  EXPECT_EQ(players[mover].y, players_copy[mover].y);
}

// 盤上左上から下へ移動しようとした場合、移動ができることを確認する
TEST_F(MoveFromBoardLeftTop, to_down) {
  // コピーした盤上の情報を更新
  board_copy[y][x] = -1;
  board_copy[y + 1][x] = mover;

  // コピーしたプレイヤーの情報を更新
  players_copy[mover].y = y + 1;

  move_down(board, players, mover);

  // 盤上の位置情報が更新されていることを確認
  EXPECT_EQ(memcmp(board, board_copy, sizeof(board)), 0);

  // プレイヤーの位置情報が更新されていることを確認
  EXPECT_EQ(players[mover].x, players_copy[mover].x);
  EXPECT_EQ(players[mover].y, players_copy[mover].y);
}

// 盤上左上から右へ移動しようとした場合、移動ができることを確認する
TEST_F(MoveFromBoardLeftTop, to_right) {
  // コピーした盤面の位置情報を更新
  board_copy[y][x] = -1;
  board_copy[y][x + 1] = mover;

  // コピーしたプレイヤーの位置情報を更新
  players_copy[mover].x = x + 1;

  move_right(board, players, mover);

  // 盤上の位置情報が更新されていることを確認
  EXPECT_EQ(memcmp(board, board_copy, sizeof(board)), 0);

  // プレイヤーの位置情報が更新されていることを確認
  EXPECT_EQ(players[mover].x, players_copy[mover].x);
  EXPECT_EQ(players[mover].y, players_copy[mover].y);
}

// 盤上左下から上へ移動しようとした場合、移動ができることを確認する
TEST_F(MoveFromBoardLeftBottom, to_up) {
  // コピーした盤上の情報を更新
  board_copy[y][x] = -1;
  board_copy[y - 1][x] = mover;

  // コピーしたプレイヤーの情報を更新
  players_copy[mover].y = y - 1;

  move_up(board, players, mover);

  // 盤上の位置情報が更新されていることを確認
  EXPECT_EQ(memcmp(board, board_copy, sizeof(board)), 0);

  // プレイヤーの位置情報が更新されていることを確認
  EXPECT_EQ(players[mover].x, players_copy[mover].x);
  EXPECT_EQ(players[mover].y, players_copy[mover].y);
}

// 盤上左下から左へ移動しようとした場合、移動ができないことを確認する
TEST_F(MoveFromBoardLeftBottom, to_left) {
  move_left(board, players, mover);

  // 盤上の位置情報が変更されていないことを確認
  EXPECT_EQ(memcmp(board, board_copy, sizeof(board)), 0);

  // プレイヤーの位置情報が変更されていないことを確認
  EXPECT_EQ(players[mover].x, players_copy[mover].x);
  EXPECT_EQ(players[mover].y, players_copy[mover].y);
}

// 盤上左下から下へ移動しようとした場合、移動ができないことを確認する
TEST_F(MoveFromBoardLeftBottom, to_down) {
  move_down(board, players, mover);

  // 盤上の位置情報が変更されていないことを確認
  EXPECT_EQ(memcmp(board, board_copy, sizeof(board)), 0);

  // プレイヤーの位置情報が変更されていないことを確認
  EXPECT_EQ(players[mover].x, players_copy[mover].x);
  EXPECT_EQ(players[mover].y, players_copy[mover].y);
}

// 盤上左下から右へ移動しようとした場合、移動ができることを確認する
TEST_F(MoveFromBoardLeftBottom, to_right) {
  // コピーした盤面の位置情報を更新
  board_copy[y][x] = -1;
  board_copy[y][x + 1] = mover;

  // コピーしたプレイヤーの位置情報を更新
  players_copy[mover].x = x + 1;

  move_right(board, players, mover);

  // 盤上の位置情報が更新されていることを確認
  EXPECT_EQ(memcmp(board, board_copy, sizeof(board)), 0);

  // プレイヤーの位置情報が更新されていることを確認
  EXPECT_EQ(players[mover].x, players_copy[mover].x);
  EXPECT_EQ(players[mover].y, players_copy[mover].y);
}

// 盤上右上から上へ移動しようとした場合、移動できることを確認する
TEST_F(MoveFromBoardRightBottom, to_up) {
  // コピーした盤上の情報を更新
  board_copy[y][x] = -1;
  board_copy[y - 1][x] = mover;

  // コピーしたプレイヤーの情報を更新
  players_copy[mover].y = y - 1;

  move_up(board, players, mover);

  // 盤上の位置情報が更新されていることを確認
  EXPECT_EQ(memcmp(board, board_copy, sizeof(board)), 0);

  // プレイヤーの位置情報が更新されていることを確認
  EXPECT_EQ(players[mover].x, players_copy[mover].x);
  EXPECT_EQ(players[mover].y, players_copy[mover].y);
}

// 盤上右下から左へ移動しようとした場合、移動ができることを確認する
TEST_F(MoveFromBoardRightBottom, to_left) {
  // コピーした盤上の情報を更新
  board_copy[y][x] = -1;
  board_copy[y][x - 1] = mover;

  // コピーしたプレイヤーの情報を更新
  players_copy[mover].x = x - 1;

  move_left(board, players, mover);

  // 盤上の位置情報が更新されていることを確認
  EXPECT_EQ(memcmp(board, board_copy, sizeof(board)), 0);

  // プレイヤーの位置情報が更新されていることを確認
  EXPECT_EQ(players[mover].x, players_copy[mover].x);
  EXPECT_EQ(players[mover].y, players_copy[mover].y);
}

// 盤上右下から下へ移動しようとした場合、移動ができないことを確認する
TEST_F(MoveFromBoardRightBottom, to_down) {
  move_down(board, players, mover);
  // 盤上の位置情報が変更されていないことを確認
  EXPECT_EQ(memcmp(board, board_copy, sizeof(board)), 0);

  // プレイヤーの位置情報が変更されていないことを確認
  EXPECT_EQ(players[mover].x, players_copy[mover].x);
  EXPECT_EQ(players[mover].y, players_copy[mover].y);
}
// 盤上右下から右へ移動しようとした場合、移動ができないことを確認する
TEST_F(MoveFromBoardRightBottom, to_right) {
  move_right(board, players, mover);
  // 盤上の位置情報が変更されていないことを確認
  EXPECT_EQ(memcmp(board, board_copy, sizeof(board)), 0);

  // プレイヤーの位置情報が変更されていないことを確認
  EXPECT_EQ(players[mover].x, players_copy[mover].x);
  EXPECT_EQ(players[mover].y, players_copy[mover].y);
}

// 盤上右上から上へ移動しようとした場合、移動ができないことを確認する
TEST_F(MoveFromBoardRightTop, to_up) {
  move_up(board, players, mover);

  // 盤上の位置情報が変更されていないことを確認
  EXPECT_EQ(memcmp(board, board_copy, sizeof(board)), 0);

  // プレイヤーの位置情報が変更されていないことを確認
  EXPECT_EQ(players[mover].x, players_copy[mover].x);
  EXPECT_EQ(players[mover].y, players_copy[mover].y);
}

// 盤上左上から左へ移動しようとした場合、移動ができることを確認する
TEST_F(MoveFromBoardRightTop, to_left) {
  // コピーした盤上の情報を更新
  board_copy[y][x] = -1;
  board_copy[y][x - 1] = mover;

  // コピーしたプレイヤーの情報を更新
  players_copy[mover].x = x - 1;

  move_left(board, players, mover);

  // 盤上の位置情報が更新されていることを確認
  EXPECT_EQ(memcmp(board, board_copy, sizeof(board)), 0);

  // プレイヤーの位置情報が更新されていることを確認
  EXPECT_EQ(players[mover].x, players_copy[mover].x);
  EXPECT_EQ(players[mover].y, players_copy[mover].y);
}

// 盤上右上から下へ移動しようとした場合、移動ができることを確認する
TEST_F(MoveFromBoardRightTop, to_down) {
  // コピーした盤上の情報を更新
  board_copy[y][x] = -1;
  board_copy[y + 1][x] = mover;

  // コピーしたプレイヤーの情報を更新
  players_copy[mover].y = y + 1;

  move_down(board, players, mover);

  // 盤上の位置情報が更新されていることを確認
  EXPECT_EQ(memcmp(board, board_copy, sizeof(board)), 0);

  // プレイヤーの位置情報が更新されていることを確認
  EXPECT_EQ(players[mover].x, players_copy[mover].x);
  EXPECT_EQ(players[mover].y, players_copy[mover].y);
}

// 盤上右上から右へ移動しようとした場合、移動ができないことを確認する
TEST_F(MoveFromBoardRightTop, to_right) {
  move_right(board, players, mover);
  // 盤上の位置情報が変更されていないことを確認
  EXPECT_EQ(memcmp(board, board_copy, sizeof(board)), 0);

  // プレイヤーの位置情報が変更されていないことを確認
  EXPECT_EQ(players[mover].x, players_copy[mover].x);
  EXPECT_EQ(players[mover].y, players_copy[mover].y);
}

// 別ユーザーがいる場所へ移動しようとした場合、移動ができないことを確認する
class ConflictUser : public ::testing::Test {
 protected:
  int x;
  int y;
  int mover;
  int board[BOARD_SIZE][BOARD_SIZE];
  int board_copy[BOARD_SIZE][BOARD_SIZE];
  player_t players[PLAYER_NUM];
  player_t players_copy[PLAYER_NUM];
};

// 　逃走者が鬼のいる場所へ移動しようとした場合、捕縛される場合のフィクスチャ
class FugtiveSuicide : public BaseVariable {
 protected:
  virtual void SetUp() {
    fugtive_x = 5;
    fugtive_y = 5;
    mover = 0;
  }
  int fugtive_x;
  int fugtive_y;
  int hunter_x;
  int hunter_y;
};

// 鬼が逃走者のいる場所へ移動しようとした場合、捕獲される場合のフィクスチャ
class HunterKillFugtive : public BaseVariable {
 protected:
  virtual void SetUp() {
    hunter_x = 5;
    hunter_y = 5;
    mover = 1;
  }
  int fugtive_x;
  int fugtive_y;
  int hunter_x;
  int hunter_y;
};

/* <勝敗に関するテスト>*/
// 逃走者が上に移動して、鬼に捕縛される場合のテスト
TEST_F(FugtiveSuicide, to_up) {
  // プレイヤーの位置の情報を設定（逃走者のすぐ上に鬼を配置）
  hunter_x = fugtive_x;
  hunter_y = fugtive_y - 1;

  setup(board, players, fugtive_x, fugtive_y, hunter_x, hunter_y);

  int res = move_up(board, players, mover);

  // 逃走者が鬼に捕縛されたことを確認(0:勝敗がついていない、1:(mover)の敗北、2:(mover)の勝利)
  EXPECT_EQ(res, 1);
}

// 逃走者が左に移動して、鬼に捕縛される場合のテスト
TEST_F(FugtiveSuicide, to_left) {
  // プレイヤーの位置の情報を設定（逃走者のすぐ左に鬼を配置）
  hunter_x = fugtive_x - 1;
  hunter_y = fugtive_y;

  setup(board, players, fugtive_x, fugtive_y, hunter_x, hunter_y);

  int res = move_left(board, players, mover);

  // 逃走者が鬼に捕縛されたことを確認(0:勝敗がついていない、1:(mover)の敗北、2:(mover)の勝利)
  EXPECT_EQ(res, 1);
}

// 逃走者が下に移動して、鬼に捕縛される場合のテストï
TEST_F(FugtiveSuicide, to_down) {
  // プレイヤーの位置の情報を設定（逃走者のすぐ左に鬼を配置）
  hunter_x = fugtive_x;
  hunter_y = fugtive_y + 1;

  setup(board, players, fugtive_x, fugtive_y, hunter_x, hunter_y);

  int res = move_down(board, players, mover);

  // 逃走者が鬼に捕縛されたことを確認(0:勝敗がついていない、1:(mover)の敗北、2:(mover)の勝利)
  EXPECT_EQ(res, 1);
}

// 逃走者が右に移動して、鬼に捕縛される場合のテスト
TEST_F(FugtiveSuicide, to_right) {
  // プレイヤーの位置の情報を設定（逃走者のすぐ左に鬼を配置）
  hunter_x = fugtive_x + 1;
  hunter_y = fugtive_y;

  setup(board, players, fugtive_x, fugtive_y, hunter_x, hunter_y);

  int res = move_right(board, players, mover);

  // 逃走者が鬼に捕縛されたことを確認(0:勝敗がついていない、1:(mover)の敗北、2:(mover)の勝利)
  EXPECT_EQ(res, 1);
}

// 鬼が上に移動して、逃走者を捕獲する場合のテスト
TEST_F(HunterKillFugtive, to_up) {
  // プレイヤーの位置の情報を設定（鬼のすぐ上に逃走者を配置）
  fugtive_x = hunter_x;
  fugtive_y = hunter_y - 1;

  setup(board, players, fugtive_x, fugtive_y, hunter_x, hunter_y);

  int res = move_up(board, players, mover);

  // 鬼が逃走者を捕獲したことを確認(0:勝敗がついていない、1:(mover)の敗北、2:(mover)の勝利)
  EXPECT_EQ(res, 2);
}
// 鬼が左に移動して、逃走者を捕獲する場合のテスト
TEST_F(HunterKillFugtive, to_left) {
  // プレイヤーの位置の情報を設定（鬼のすぐ上に逃走者を配置）
  fugtive_x = hunter_x - 1;
  fugtive_y = hunter_y;

  setup(board, players, fugtive_x, fugtive_y, hunter_x, hunter_y);

  int res = move_left(board, players, mover);

  // 鬼が逃走者を捕獲したことを確認(0:勝敗がついていない、1:(mover)の敗北、2:(mover)の勝利)
  EXPECT_EQ(res, 2);
}
// 鬼が下に移動して、逃走者を捕獲する場合のテスト
TEST_F(HunterKillFugtive, to_down) {
  // プレイヤーの位置の情報を設定（鬼のすぐ上に逃走者を配置）
  fugtive_x = hunter_x;
  fugtive_y = hunter_y + 1;

  setup(board, players, fugtive_x, fugtive_y, hunter_x, hunter_y);

  int res = move_down(board, players, mover);

  // 鬼が逃走者を捕獲したことを確認(0:勝敗がついていない、1:(mover)の敗北、2:(mover)の勝利)
  EXPECT_EQ(res, 2);
}
// 鬼が上に移動して、逃走者を捕獲する場合のテスト
TEST_F(HunterKillFugtive, to_right) {
  // プレイヤーの位置の情報を設定（鬼のすぐ上に逃走者を配置）
  fugtive_x = hunter_x;
  fugtive_y = hunter_y - 1;

  setup(board, players, fugtive_x, fugtive_y, hunter_x, hunter_y);

  int res = move_up(board, players, mover);

  // 鬼が逃走者を捕獲したことを確認(0:勝敗がついていない、1:(mover)の敗北、2:(mover)の勝利)
  EXPECT_EQ(res, 2);
}

/* <表示に関するテスト> */
// 敗者の結果画面のviewが正しく設定されているかどうかを確認する
TEST(set_result_view_win, loser) {
  char buff[256];
  set_result_view(buff, 1);

  EXPECT_STRNE(strstr(buff, "LOSER"), NULL);
}

// 勝者の結果画面のviewが正しく設定されているかどうかを確認する
TEST(set_result_view_win, winner) {
  char buff[256];
  set_result_view(buff, 2);

  EXPECT_STRNE(strstr(buff, "WINNER"), NULL);
}

// 待ち画面のviewが正しく設定されているかどうかを確認する
TEST(set_waiting_view, waiting) {
  char buff[256];
  set_waiting_view(buff);
  EXPECT_STRNE(strstr(buff, "PLEASE WAIT A FEW MOMENTS"), NULL);
}

/* <結果についてのテスト> */

class Result_of_Move : public ::testing::Test {
 protected:
  virtual void SetUp() {
    int fugtive_x = BOARD_SIZE / 2;
    int fugtive_y = BOARD_SIZE / 2;
    int hunter_x = fugtive_x + 1;
    int hunter_y = fugtive_y;

    // 0 が逃走者、1 が鬼
    players[0].x = fugtive_x;
    players[0].y = fugtive_y;
    players[0].status = 0;
    players[0].is_hunter = 0;
    players[1].x = hunter_x;
    players[1].y = hunter_y;
    players[1].status = 0;
    players[1].is_hunter = 1;
  }
  player_t players[PLAYER_NUM];
  int res;
  int mover;   // 移動するプレイヤーの番号
  int native;  // 移動先のプレイヤーの番号
};

// 逃走者が鬼に捕まりに行く場合のテスト(プレイヤー０が敗北する)
TEST_F(Result_of_Move, fugtive_to_suicide) {
  mover = 0;
  native = 1;

  res = result_of_moving(players, mover, native);

  // 敗北コードが返ってくることを確認する
  EXPECT_EQ(res, 1);

  // プレイヤー０のステータスが敗北になっていることを確認する
  EXPECT_EQ(players[0].status, 1);

  // プレイヤー１のステータスが勝利になっていることを確認する
  EXPECT_EQ(players[1].status, 2);
}

// 鬼が逃走者を捕獲する場合のテスト（プレイヤー１が勝利する）
TEST_F(Result_of_Move, hunter_capture_fugtive) {
  mover = 1;
  native = 0;

  res = result_of_moving(players, mover, native);

  // 勝利コードが返ってることを確認する
  EXPECT_EQ(res, 2);

  // プレイヤー１のステータスが勝利になっていることを確認する
  EXPECT_EQ(players[1].status, 2);

  // プレイヤー０のステータスが敗北になっていることを確認する
  EXPECT_EQ(players[0].status, 1);
}

