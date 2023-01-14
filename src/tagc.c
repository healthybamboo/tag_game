#include <arpa/inet.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "libs/communication.h"
#include "libs/game.h"
#include "libs/setting.h"
#include "libs/utils.h"

int main(int argc, char *argv[]) {
  // ソケット用の変数を宣言
  int tcp_sock, udp_sock;

  // ポート番号を格納する変数を宣言
  unsigned short tcp_port;
  unsigned short udp_port;

  // 自分が鬼かどうかを判定する変数
  int is_hunter;

  // 並列処理用の変数を宣言
  pid_t pid;

  // 接続先のアドレス構造体を設定
  struct sockaddr_in *tcp_server_addr;
  struct sockaddr_in *udp_server_addr;
  tcp_server_addr = (struct sockaddr_in *)malloc(sizeof(struct sockaddr_in));
  udp_server_addr = (struct sockaddr_in *)malloc(sizeof(struct sockaddr_in));

  // 引数の数をチャック
  if (argc != 3) {
    printf("Usage: %s <IP address> <port number>\n", argv[0]);
    exit(1);
  }

  // 接続先のIPアドレスを設定する
  char ip[16];

  // エラー処理後、接続先のIPアドレスを設定
  strcpy(ip, argv[1]);

  // 接続先のポート番号を設定する
  tcp_port = atoi(argv[2]);

  // 正しいポート番号かチェック
  if (tcp_port < 0 && tcp_port > 65535) {
    perror("Incorrect Port error.\n");
    exit(1);
  }

  // アドレス構造体にポート番号を設定
  set_target_addr(tcp_server_addr, ip, tcp_port);

  // ソケットを作成
  tcp_sock = make_tcp_socket();
  udp_sock = make_udp_socket();

  // サーバーに接続
  connect_server(tcp_sock, tcp_server_addr);

  // UDP通信のポート番号をサーバーから受信
  char match_msg[BUFFSIZE];
  int id;

  printf("Waiting for match...\n");

  // マッチングメッセージを待つ
  recv_tcp_msg(tcp_sock, match_msg, sizeof(match_msg));

  // マッチングメッセージを解析(UDP通信のポート番号 + 操作する駒のID +
  // 鬼かどうか)
  sscanf(match_msg, "M %hd %d %d", &udp_port, &id, &is_hunter);

  if (DEBUG) printf("DEBUG:udp_port: %d\n", udp_port);

  // TODO.IDは使わないので、後で消す
  printf("Matched! Your ID is %d.\n", id);
  printf("You are %s.\n", is_hunter ? "hunter : *" : "fugtive : o");

  // UDP通信のアドレス構造体を設定(TODO.set_target_addr()を使うでないと、自ホストのサーバーとしか通信できない)
  set_server_addr(udp_server_addr, udp_port);

  // 並列処理を開始
  pid = fork();
  /*親プロセス(TCP通信でメッセージを受信)*/
  if (pid > 0) {
    if (DEBUG) printf("DEBUG:start parent process(%d).\n", getpid());

    // 受信した、プレイヤーの状態メッセージを格納する変数を宣言
    char status_msg[128];
    // 受信した、プレイヤーの状態を格納する変数
    int state;

    while (1) {
      // メッセージを受信
      recv_tcp_msg(tcp_sock, status_msg, sizeof(status_msg));

      if (DEBUG) printf("DEBUG:recv_msg: %s\n", status_msg);

      // メッセージを解析
      char code = status_msg[0];

      // ステータス情報を受信
      if (code == 'S') {
        // メッセージの先頭がSの場合、IDを取得
        sscanf(status_msg, "S %d", &state);

        if (DEBUG) printf("DEBUG:recv_state: %d\n", state);

        // 勝敗がついたら終了
        if (state == 1 || state == 2) {
          // 子プロセスを終了(操作を受け付けないようにする)
          kill(pid, SIGKILL);
          break;
        }
      }
    }

    printf("Game is over.\n");

    char buff[BUFSIZ];
    // 結果を格納する変数(TODO.buffをresult_buffに変更する)
    set_result_view(buff, state);

    // 結果を表示
    printf("%s", buff);

  } /* 子プロセス（操作を受け付け、UDP通信でメッセージを送信）*/
  else if (pid == 0) {
    if (DEBUG) printf("DEBUG:start child process.\n");

    // メッセージを格納する変数を宣言(TODO.msgをoperation_msgに変更する)
    char msg[128];

    // メッセージを初期化
    memset(msg, 0, sizeof(msg));

    if (DEBUG) printf("DEBUG: parent process is %d\n", getppid());

    while (1) {
      // 親プロセスが死んだら終了(これがないと無限ループになる)
      if (getppid() == 1) break;

      if (DEBUG) printf("DEBUG:input wasd to move.\n");

      // メッセージを入力
      int ch = getch();

      // 入力されたキーを変換
      int key = convert_key(ch);

      // w,a,s,d以外の入力は無視
      if (key != 0) {
        // メッセージを作成
        sprintf(msg, "O %d %d", id, key);

        // メッセージを送信
        send_udp_msg(udp_sock, udp_server_addr, msg);

        if (DEBUG) printf("DEBUG:send_msg:%s\n", msg);
      }
    }
  } else {
    perror("fork error.\n");
  }

  // ソケットを閉じる
  close(tcp_sock);
  close(udp_sock);

  // メモリを解放
  free(tcp_server_addr);
  free(udp_server_addr);

  return 0;
}
