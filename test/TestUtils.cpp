#include "gtest/gtest.h"

extern "C" {
#include "../src/libs/utils.h"
}

/* その他の処理 */
// 指定された範囲の乱数を返す関数のテスト
TEST(utils, get_random) {
  unsigned int seed = (unsigned)time(NULL);
  srand(seed);
  int res;
  // 100回繰り返せば、0, 1, 2が最低１回は出るはず
  for (int i = 0; i < 100; i++) {
    seed = (unsigned int)i;
    res = get_random_number(0, 2);

    // 0 <= res <= 2であることを確認
    EXPECT_GE(res, 0);
    EXPECT_LE(res, 2);
  }
}