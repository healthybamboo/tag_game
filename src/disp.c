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
  // マルチキャストに対応した環境用の処理
  if (USE_MULTI_CAST) {
    // マルチキャスト用のIPアドレスを設定
    char ip[16];
    // マルチキャスト用のポート番号を設定
    unsigned short port;

    // マルチキャスト用のソケット構造体を宣言
    int sock;

    // 受信用のバッファを宣言
    char buff[BUFFSIZE];

    // 受信したバイト数を格納する変数を宣言
    int bytes;
    // アドレス構造体を宣言
    struct sockaddr_in *addr;
    addr = (struct sockaddr_in *)malloc(sizeof(struct sockaddr_in));

    // マルチキャスト用のアドレス構造体を宣言
    struct ip_mreq *multicast;
    multicast = (struct ip_mreq *)malloc(sizeof(struct ip_mreq));

    /* 引数が二つなければ使用方法を表示して終了する */
    if (argc != 3) {
      fprintf(stderr, "Usage: %s <mulicast IP> <port>\n", argv[0]);
      exit(1);
    }

    /* 一つ目の引数: IP アドレス */
    strcpy(ip, argv[1]);

    /* 二つ目の引数: ポート番号 */
    port = atoi(argv[2]);

    // 受信用のUDP:ソケットを作成する
    sock = make_udp_socket();

    // アドレス構造体を設定する
    set_server_addr(addr, port);

    // ポートに結びつける
    set_server_addr(addr, port);

    // sockにアドレス構造体をバインドする
    bind_sock(sock, addr);

    // ipのマルチキャストグループに参加する
    join_multicast_group(sock, multicast, ip);

    while (1) {
      /* 受信する */
      recv_udp_msg(sock, addr, buff, BUFFSIZE);

      /* 送信されてきたメッセージをそのまま表示する */
      printf("%s\n", buff);
    }
    /* ソケットを閉じる */
    close(sock);

    // アドレスを解放する
    free(addr);
    free(multicast);

    return 0;

    // マルチキャスト非対応の環境用(DEBUG用)
  } else {
    // ポート番号を格納する変数を宣言
    unsigned short port;
    // ソケットを格納する変数を宣言
    int sock;
    // 受信用のバッファを宣言
    char buff[BUFFSIZE];
    // 受信したバイト数を格納する変数を宣言
    int bytes;

    // 引数が二つなければ使用方法を表示して終了する
    if (argc != 2) {
      fprintf(stderr, "Usage: %s <Port>", argv[0]);

      exit(1);
    }
    // アドレス構造体を宣言（サーバー用とクライアントの情報を格納するために２つ）
    struct sockaddr_in *addr, *client;
    addr = (struct sockaddr_in *)malloc(sizeof(struct sockaddr_in));
    client = (struct sockaddr_in *)malloc(sizeof(struct sockaddr_in));

    // 引数からポート番号を取得する
    port = atoi(argv[1]);

    // ソケットを作成する
    sock = make_udp_socket();

    // アドレス構造体にポート番号を設定する
    set_server_addr(addr, port);

    // ソケットとアドレス構造体を結びつける
    bind_sock(sock, addr);

    while (1) {
      // メッセージを受信する
      bytes = recv_udp_msg(sock, client, buff, BUFFSIZE);

      // 文字列として扱うため最後を \0 で終端して表示する
      buff[bytes] = '\0';

      // 受信したメッセージを表示する
      printf("%s", buff);
    }
    // ソケットを閉じる
    close(sock);

    // アドレス構造体のメモリを解放する
    free(addr);
    free(client);
  }
}