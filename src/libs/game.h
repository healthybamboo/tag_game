/* ゲームに関する処理*/
// 各定数の定義
#define BOARD_SIZE 10
#define PLAYER_NUM 2
#define HEAD "*:Hunter, o:Runner\n"
#define LINE "+-+-+-+-+-+-+-+-+-+-+\n"

// プレイヤーの情報を格納する構造体
typedef struct {
  // プレイヤーの場所
  int x;
  int y;
  // 鬼かどうか
  int is_hunter;
  // プレイヤーの状態(-1:ゲームを抜けている、0:ゲーム中、1:敗北)
  int status;
} player_t;

// ゲームの初期化を行う関数
int init_game(int board[BOARD_SIZE][BOARD_SIZE], player_t players[PLAYER_NUM]);

/* 移動用の関数 */
// 上に移動する関数
int move_up(int board[BOARD_SIZE][BOARD_SIZE], player_t players[PLAYER_NUM],
            int mover);

// 　左に移動する関数
int move_left(int board[BOARD_SIZE][BOARD_SIZE], player_t players[PLAYER_NUM],
              int mover);

// 下に移動する関数
int move_down(int board[BOARD_SIZE][BOARD_SIZE], player_t players[PLAYER_NUM],
              int mover);

// 右に移動する関数
int move_right(int board[BOARD_SIZE][BOARD_SIZE], player_t players[PLAYER_NUM],
               int mover);

// 移動する関数
int move(int board[BOARD_SIZE][BOARD_SIZE], player_t players[PLAYER_NUM],
         int direction, int mover);

/* UI関連の処理 */
// dispに送信するviewを作成する関数
void set_disp_view(char *disp_view, char *time_view, char *board_view);

// timeのviewを作成する関数
void set_time_view(char *buff, int time);

// 盤面の見た目を作る関数
void set_board_view(char *buff, player_t players[PLAYER_NUM]);

// 街画面を表示する関数
void set_waiting_view(char *buff);

// 結果画面を表示する関数
void set_result_view(char *buff, int result);

/* その他 */

// 移動した結果を判定する関数
int result_of_moving(player_t players[PLAYER_NUM], int mover,
                     int native_player);
