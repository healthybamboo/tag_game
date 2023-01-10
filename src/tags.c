#include <arpa/inet.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

#include "libs/communication.h"
#include "libs/game.h"
#include "libs/setting.h"
#include "libs/utils.h"

int main(int argc, char *argv[]) {
  // 乱数の種を設定
  srand((unsigned)time(NULL));
  int i;
  // 引数の数が正しくない場合はエラーを出力して終了
  if (argc != 4) {
    printf("Usage: %s <port number> <disp ip> <disp port>\n", argv[0]);
    exit(1);
  }
  char *disp_ip = argv[2];
  unsigned short disp_port = atoi(argv[3]);

  // ゲームの初期化
  int board[BOARD_SIZE][BOARD_SIZE];
  player_t players[PLAYER_NUM];
  init_game(board, players);

  // board_view
  char board_view[1024];

  // UDPソケットを作成
  int udp_sock;
  int disp_sock;

  // TCPソケットを作成
  int tcp_sock, tcp_csock[2];

  // アドレス構造体
  struct sockaddr_in *udp_server_addr, *tcp_server_addr, *from_addr, *disp_addr;
  udp_server_addr = (struct sockaddr_in *)malloc(sizeof(struct sockaddr_in));
  tcp_server_addr = (struct sockaddr_in *)malloc(sizeof(struct sockaddr_in));
  disp_addr = (struct sockaddr_in *)malloc(sizeof(struct sockaddr_in));

  // TODO.複数クライアントに対応するために、アドレス構造体を配列にする
  from_addr = (struct sockaddr_in *)malloc(sizeof(struct sockaddr_in));

  // ポートを設定（TODO.tcpについてはコマンドライン引数で設定できるようにする）
  unsigned short tpc_port;
  unsigned short udp_port;

  // ポート番号を設定
  tpc_port = atoi(argv[1]);

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

  udp_port = get_port_number_from_sock(udp_sock);

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
    sprintf(msg, "M %d %d %d", udp_port, i,players[i].is_hunter);

    // 送信する
    send_tcp_msg(tcp_csock[i], msg);

    if (DEBUG) printf("DEBUG:send msg: %s\n", msg);
  }

  printf("GAME START!!\n");

  // ゲームの　メインループ
  while (1) {
    // ビューを更新する
    set_board_view(board_view, players);

    // ビューを送信する
    send_udp_msg(disp_sock, disp_addr, board_view);

    // ビュー表示する`
    if (DEBUG) printf("%s\n", board_view);

    // TODO.MCでメッセージを送信する

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

      // 操作を実行
      if (DEBUG) printf("DEBUG:ope target=%d,command=%d \n", target, command);

      result = move(board, players, command, target);

      if (DEBUG) printf("DEBUG:move_result=%d\n", result);
      if (DEBUG) printf("DEBUG:player=%d,", target);
      if (DEBUG) printf("x=%d,y=%d\n", players[target].x, players[target].y);

      // 勝敗を通知する
      if (result == 1 || result == 2) {
        printf("GAME FINISH!!\n");

        // dispに終了を通知する
        send_udp_msg(disp_sock, disp_addr, "exit");

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
  // アドレス構造体の解放
  free(udp_server_addr);
  free(tcp_server_addr);
  free(from_addr);

  // ソケットを閉じる
  close(udp_sock);
  close(tcp_sock);
}