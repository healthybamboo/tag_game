#include <arpa/inet.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include "libs/communication.h"
#include "libs/game.h"
#include "libs/setting.h"
#include "libs/utils.h"

enum { SEGMENT_SIZE = 0x6400 };

int main(int argc, char *argv[]) {
  // 添字の宣言(ループ用)
  int i;

  // プロセスID
  pid_t alive;
  pid_t pid;

  // プロセスのステータス
  int status;

  // 共有メモリのセグメントID
  int segment_id[2];

  // 共有メモリのセグメントを作成
  for (i = 0; i < 2; i++) {
    segment_id[i] = shmget(IPC_PRIVATE, SEGMENT_SIZE,
                           IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
  }

  // 共有メモリをアタッチ
  // 時間表示用のview
  char *time_view = (char *)shmat(segment_id[0], 0, 0);
  // 盤面表示用のview
  char *board_view = (char *)shmat(segment_id[1], 0, 0);

  // 乱数の種を設定
  srand((unsigned)time(NULL));

  // 引数の数が正しくない場合はエラーを出力して終了
  if (argc != 4) {
    printf("Usage: %s <port number> <disp ip> <disp port>\n", argv[0]);
    exit(1);
  }

  // 盤面の状態を保持する二次元配列
  int board[BOARD_SIZE][BOARD_SIZE];

  // プレイヤーの情報を保持する構造体の配列
  player_t players[PLAYER_NUM];

  // ゲームの初期化
  init_game(board, players);

  // disp用のview
  char disp_view[BUFFSIZE];

  // UDPソケット用の変数
  int udp_sock;
  int disp_sock;

  // TCPソケット用の変数
  int tcp_sock, tcp_csock[2];

  // アドレス構造体
  struct sockaddr_in *udp_server_addr, *tcp_server_addr, *from_addr, *disp_addr;
  udp_server_addr = (struct sockaddr_in *)malloc(sizeof(struct sockaddr_in));
  tcp_server_addr = (struct sockaddr_in *)malloc(sizeof(struct sockaddr_in));
  disp_addr = (struct sockaddr_in *)malloc(sizeof(struct sockaddr_in));

  // クライアントのアドレスを格納する構造体（引数用）
  from_addr = (struct sockaddr_in *)malloc(sizeof(struct sockaddr_in));

  // ポートを設定
  unsigned short tpc_port;
  unsigned short udp_port;

  // ポート番号を設定
  tpc_port = atoi(argv[1]);

  // ディスプレイのIPアドレスを設定
  char *disp_ip = argv[2];

  // ディスプレイのポート番号を設定
  unsigned short disp_port = atoi(argv[3]);

  // ソケットを作成
  tcp_sock = make_tcp_socket();
  udp_sock = make_udp_socket();
  disp_sock = (USE_MULTI_CAST) ? make_udp_socket_with_ttl() : make_udp_socket();

  // アドレス構造体にポート番号を設定
  set_server_addr(tcp_server_addr, tpc_port);
  set_server_addr_non_port(udp_server_addr);
  set_target_addr(disp_addr, disp_ip, disp_port);

  // 受信用ソケットとアドレス構造体をバインド
  bind_sock(tcp_sock, tcp_server_addr);
  bind_sock(udp_sock, udp_server_addr);

  // ソケットから割り当てられたポート番号を取得
  udp_port = get_port_number_from_sock(udp_sock);

  // デバッグ用
  if (DEBUG) printf("udp-port is %d\n", udp_port);

  // 接続を待ち受ける
  listen_connection(tcp_sock, MAX_CLIENT);

  printf("WAITING FOR CONNECTION...\n");

  // 接続を受け付ける
  for (i = 0; i < MAX_CLIENT; i++) {
    tcp_csock[i] = accept_connection(tcp_sock, from_addr);
    printf("CONNECTED %d\n", i + 1);
  }

  // クライアントの接続を確認する
  if (DEBUG) printf("MATCH %d-%d\n", tcp_csock[0], tcp_csock[1]);

  /* マッチング部分 */
  // MATHING: IDを送信して、クライアントにゲームの開始を通知する
  for (i = 0; i < MAX_CLIENT; i++) {
    char msg[BUFFSIZE];
    memset(msg, 0, sizeof(msg));

    // IDを文字列に埋め込む
    sprintf(msg, "M %d %d %d", udp_port, i, players[i].is_hunter);

    // 送信する
    send_tcp_msg(tcp_csock[i], msg);

    if (DEBUG) printf("DEBUG:send msg: %s\n", msg);
  }

  printf("GAME START!!\n");

  // 並列処理を開始
  pid = fork();

  /* 親プロセス*/
  if (pid > 0) {
    int count = 0;
    while (count++ < GAME_TIME_SEC) {
      // 子プロセスが終了していたら、whileを抜ける
      alive = waitpid(pid, &status, WNOHANG);
      // 子プロセスが終了していないなら続行
      if (alive == 0) {
        // 時間のviewを変更する
        set_time_view(time_view, GAME_TIME_SEC - count);

        // disp_viewを設定する
        set_disp_view(disp_view, time_view, board_view);

        // dispへviewを送信する
        send_udp_msg(disp_sock, disp_addr, disp_view);

        // ループ１回につき、１秒待つ。
        sleep(1);
      }
      // エラー発生ならエラーメッセージ出力
      else if (alive == -1) {
        printf("waitpid error\n");
      } else {
        // 子プロセスが終了しているのであれば、親プロセスも終了させる。
        if (DEBUG) printf("child process is dead %d", alive);
        exit(0);
      }
    }

    printf("TIME OUT!\n");

    // 子プロセスを終了
    kill(pid, SIGKILL);

    // dispに逃走者の勝利を通知する
    send_udp_msg(disp_sock, disp_addr, "Fugtive Victory!\n");

    char result_msg[BUFFSIZE];

    // 文字列に埋め込む
    for (i = 0; i < MAX_CLIENT; i++) {
      // 文字列にステータスを埋め込む(ハンターなら負け、逃走者なら勝ち)
      sprintf(result_msg, "S %d", players[i].is_hunter ? 1 : 2);

      // 送信する
      send_tcp_msg(tcp_csock[i], result_msg);

      // ソケットを閉じる
      close(tcp_csock[i]);
    }

  }
  /* 子プロセス */
  else if (pid == 0) {
    // ゲームの　メインループ
    while (1) {
      // 盤のビューを更新する
      set_board_view(board_view, players);

      // dispへ送信するのビューを設定する
      set_disp_view(disp_view, time_view, board_view);

      // ビューをdispへ送信する
      send_udp_msg(disp_sock, disp_addr, disp_view);

      if (DEBUG) printf("%s\n", disp_view);

      // 操作メッセージ用のバッファ
      char operation_msg[BUFFSIZE];

      memset(operation_msg, 0, sizeof(operation_msg));

      // 操作メッセージを受信する
      recv_udp_msg(udp_sock, from_addr, operation_msg, BUFFSIZE);

      // 操作メッセージから必要な情報を取得する。
      if (operation_msg[0] == 'O') {
        int result = 0;
        int target, command;
        // 操作を取得
        sscanf(operation_msg, "O %d %d", &target, &command);

        if (DEBUG) printf("DEBUG:ope target=%d,command=%d \n", target, command);

        // 操作を実行
        result = move(board, players, command, target);

        if (DEBUG) printf("DEBUG:move_result=%d\n", result);

        // 勝敗を通知する
        if (result == 1 || result == 2) {
          printf("GAME FINISH!!\n");

          // dispに鬼の勝利を通知する
          send_udp_msg(disp_sock, disp_addr, "Hunter Victory!\n");

          char result_msg[BUFFSIZE];

          // 文字列に埋め込む
          for (i = 0; i < MAX_CLIENT; i++) {
            if (DEBUG) printf("DEBUG:id=%d, status=%d\n", i, players[i].status);
            // 文字列にステータスを埋め込む
            sprintf(result_msg, "S %d", players[i].status);
            // 送信する
            send_tcp_msg(tcp_csock[i], result_msg);

            // ソケットを閉じる
            close(tcp_csock[i]);
          }
          // 処理を終了する
          break;
        }
      } else {
        // 操作メッセージが不正な場合は、処理をスキップする
        perror("ERROR:invalid operation");
        continue;
      }
    }
  } else {
    perror("fork");
  }

  if (DEBUG) printf("START POST-PROCESSING.\n");

  // 共有メモリを解放する
  shmdt(time_view);
  shmctl(segment_id[0], IPC_RMID, 0);
  shmdt(board_view);
  shmctl(segment_id[1], IPC_RMID, 0);

  // アドレス構造体の解放
  free(udp_server_addr);
  free(tcp_server_addr);
  free(from_addr);

  // ソケットを閉じる
  close(udp_sock);
  close(tcp_sock);
  // TODO. close disp_sock
  if (DEBUG) printf("BYE!\n");
}