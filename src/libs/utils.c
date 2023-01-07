#include "utils.h"

#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>

#include "setting.h"
// キー入力受け付ける関数
int getch(void) {
  struct termios oldattr, newattr;
  int ch;
  tcgetattr(STDIN_FILENO, &oldattr);
  newattr = oldattr;
  newattr.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &newattr);
  ch = getchar();
  tcsetattr(STDIN_FILENO, TCSANOW, &oldattr);
  return ch;
}

// w,a,s,dを送信用のコードに変換する関数
int convert_key(int key) {
  // w,a,s,dの入力をチェック
  switch (key) {
    case 'w':
      // フォーマットを整える
      if (DEBUG) printf("DEBUG:input w.\n");
      return 8;
      break;

    case 'a':
      if (DEBUG) printf("DEBUG:input a.\n");
      return 4;

    case 's':
      if (DEBUG) printf("DEBUG:input s.\n");
      return 2;
      break;

    case 'd':
      if (DEBUG) printf("DEBUG:input d.\n");
      return 6;

    default:
      if (DEBUG) printf("DEBUG:other input.\n");
      return 0;
  }
}

// 最小値から最大値までの乱数を返す関数
int get_random_number(unsigned int seed, int min, int max) {
  int res;
  // 乱数の初期化
  srand(seed);

  // 乱数の生成
  res = (rand() % (max - min + 1)) + min;
  return res;
}
