#include "game.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <time.h>
#include <unistd.h>

#include "setting.h"
#include "utils.h"

// ゲームの初期化を行う関数
int init_game(int board[BOARD_SIZE][BOARD_SIZE], player_t players[PLAYER_NUM]) {
  int i, j;
  // 乱数の初期化
  srand((unsigned)time(NULL));

  // ハンターの番号をランダムに決定
  int hunter_num = get_random_number(0, PLAYER_NUM - 1);

  // 1.ボードを-1で初期化
  for (i = 0; i < BOARD_SIZE; i++) {
    for (j = 0; j < BOARD_SIZE; j++) {
      board[i][j] = -1;
    }
  }

  // 2.プレイヤー情報を初期化
  for (i = 0; i < PLAYER_NUM; i++) {
    int x, y;
    do {
      x = get_random_number(0, BOARD_SIZE - 1);
      y = get_random_number(0, BOARD_SIZE - 1);
    } while (board[x][y] != -1);

    players[i].x = x;
    players[i].y = y;
    players[i].is_hunter = (i == hunter_num) ? 1 : 0;
    players[i].status = 0;

    board[x][y] = i;
  }

  return 0;
}

// 勝敗の判定
// TODO.３人以上のユーザーでも判定を可能にする(現在は２人対戦のみ対応)
int result_of_moving(player_t players[PLAYER_NUM], int mover,
                     int native_player) {
  // 移動するプレイヤーがハンターで、移動先のプレイヤーもハンターの場合(移動しない)
  if (players[mover].is_hunter == players[native_player].is_hunter) {
    return 0;

    // 移動するプレイヤーが逃走者で、移動先のプレイヤーがハンターの場合(敗北)
  } else if (!players[mover].is_hunter && players[native_player].is_hunter) {
    // 移動するプレイヤーを敗北状態にして、移動先のプレイヤーを勝利状態にする
    players[mover].status = 1;
    players[native_player].status = 2;

    return 1;

    // 移動するプレイヤーがハンターで、移動先のプレイヤーが逃走者の場合(勝利)
  } else if (players[mover].is_hunter && !players[native_player].is_hunter) {
    // 移動先のプレイヤーを敗北状態にして、移動するプレイヤーを勝利状態にする
    players[mover].status = 2;
    players[native_player].status = 1;

    return 2;
  }

  // ここには来ないはず
  return -1;
}

// 上に移動する関数
int move_up(int board[BOARD_SIZE][BOARD_SIZE], player_t players[PLAYER_NUM],
            int mover) {
  // 移動対象のプレイヤーの座標を取得

  int x = players[mover].x;

  int y = players[mover].y;
  // 移動対象のプレイヤーが上端にはいない場合
  if (y > 0) {
    int destinatin = board[y - 1][x];
    // 移動先の座標にプレイヤーがいる場合
    if (destinatin >= 0) {
      return result_of_moving(players, mover, destinatin);
    } else {
      // プレイヤー情報の更新
      players[mover].y--;

      // 盤の更新
      board[y][x] = -1;
      board[y - 1][x] = mover;
    }
  }

  return 0;
};

// 左に移動する関数
int move_left(int board[BOARD_SIZE][BOARD_SIZE], player_t players[PLAYER_NUM],
              int mover) {
  // 移動対象のプレイヤー現在の座標を取得
  int x = players[mover].x;
  int y = players[mover].y;

  // 移動対象のプレイヤーが左端にはいない場合)
  if (x > 0) {
    int destinatin = board[y][x - 1];
    // 移動先の座標にプレイヤーがいる場合
    if (destinatin >= 0) {
      return result_of_moving(players, mover, destinatin);
    } else {
      // プレイヤー情報の更新
      players[mover].x--;

      // 盤の更新
      board[y][x] = -1;
      board[y][x - 1] = mover;
    }
  }
  return 0;
};

// 　下に移動する関数
int move_down(int board[BOARD_SIZE][BOARD_SIZE], player_t players[PLAYER_NUM],
              int mover) {
  // 移動対象のプレイヤー現在の座標を取得
  int x = players[mover].x;
  int y = players[mover].y;

  // 移動対象のプレイヤーが下端にはいない場合
  if (y < BOARD_SIZE - 1) {
    int destinatin = board[y + 1][x];
    // 移動先の座標にプレイヤーがいる場合
    if (destinatin >= 0) {
      return result_of_moving(players, mover, destinatin);
    } else {
      // プレイヤー情報の更新
      players[mover].y++;

      // 盤の更新
      board[y][x] = -1;
      board[y + 1][x] = mover;
    }
  }
  return 0;
};

// 右に移動する関数
int move_right(int board[BOARD_SIZE][BOARD_SIZE], player_t players[PLAYER_NUM],
               int mover) {
  // 移動対象のプレイヤーの座標を取得
  int x = players[mover].x;
  int y = players[mover].y;

  //  移動対象のプレイヤーが右端にはいない場合
  if (x < BOARD_SIZE - 1) {
    int destinatin = 0;
    destinatin = board[y][x + 1];
    if (destinatin >= 0) {
      return result_of_moving(players, mover, destinatin);
    } else {
      // プレイヤー情報の更新
      players[mover].x++;

      // 盤の更新
      board[y][x] = -1;
      board[y][x + 1] = mover;
    }
  }
  return 0;
};

// 方向を決定する関数
int move(int board[BOARD_SIZE][BOARD_SIZE], player_t players[PLAYER_NUM],
         int direction, int mover) {
  // 8 は上
  if (direction == 8) {
    return move_up(board, players, mover);
  }

  // 4 は左
  if (direction == 4) {
    return move_left(board, players, mover);
  }
  // 2 は下
  if (direction == 2) {
    return move_down(board, players, mover);
  }

  // 6 は右
  if (direction == 6) {
    return move_right(board, players, mover);
  }

  // それ以外の場合は何もしない
  return 0;
}
// 10 x 10 の盤面を表示する文字列を作成する．
void set_board_view(char *buff, player_t players[PLAYER_NUM]) {
  int i, j, k;

  /* 画面をクリアする特殊文字（エスケープ・シーケンス） */
  memset(buff, 0, sizeof(*buff));

  // 盤面のビューを作成
  for (i = 0; i < BOARD_SIZE; i++) {
    strcat(buff, LINE);
    for (j = 0; j < 10; j++) {
      for (k = 0; k < PLAYER_NUM; k++) {
        if (players[k].x == j && players[k].y == i) {
          if (players[k].is_hunter)
            strcat(buff, "|*");
          else
            strcat(buff, "|o");
          // ここで、次のますに進む(TODO.gotoを使わないようにする)
          goto next;
        }
      }
      strcat(buff, "| ");
    next:;
    }
    strcat(buff, "|\n");
  }
  strcat(buff, LINE);
}

void set_result_view(char *buff, int result) {
  /* 画面をクリアする特殊文字（エスケープ・シーケンス） */
  strcpy(buff, "\033[2J\033[0;0H\n");

  if (result == 1) {
    strcat(buff,
           "===================\n"
           "  YOU ARE LOSER!\n"
           "===================\n");
  } else if (result == 2) {
    strcat(buff,
           "===================\n"
           "  YOU ARE WINNER!\n"
           "===================\n");
  } else {
    // ここには来ないはず
    strcat(buff, "error!.");
  }
}

void set_waiting_view(char *buff) {
  /* 画面をクリアする特殊文字（エスケープ・シーケンス） */
  strcpy(buff, "\033[2J\033[0;0H\n");

  strcat(buff,
         "=========================\n"
         "PLEASE WAIT A FEW MOMENTS\n"
         "=========================\n");
}
