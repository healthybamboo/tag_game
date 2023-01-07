#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "gtest/gtest.h"

extern "C" {
#include "../src/libs/communication.h"
}

#define IP "127.0.0.1"
#define MSG "hello"
#define BUFFSIZE 1024
#define MAX_PENDING 5

// COMMON:通信相手のアドレスの設定ができているかを確認する
TEST(common, test_set_target_addr) {
  struct sockaddr_in *addr;
  addr = (sockaddr_in *)malloc(sizeof(sockaddr_in));

  char ip[16] = IP;
  unsigned short port = 11111;

  // ターゲットのアドレス構造体の設定
  set_target_addr(addr, ip, port);

  // 正しく設定されているかを確認
  EXPECT_EQ(addr->sin_family, AF_INET);
  EXPECT_EQ(addr->sin_addr.s_addr, inet_addr(IP));
  EXPECT_EQ(ntohs(addr->sin_port), port);
  free(addr);
}

// COMMON:サーバー用のアドレス構造体の設定が正しくできているかを確認する
TEST(common, test_set_server_addr) {
  struct sockaddr_in *addr;
  addr = (sockaddr_in *)malloc(sizeof(sockaddr_in));

  unsigned short port = 11112;

  // アドレス構造体を設定
  set_server_addr(addr, port);

  // 正しく設定されているかを確認
  EXPECT_EQ(addr->sin_family, AF_INET);
  EXPECT_EQ(addr->sin_addr.s_addr, htonl(INADDR_ANY));
  EXPECT_EQ(ntohs(addr->sin_port), port);
  free(addr);
}

// UDP:ソケットの作成ができているか
TEST(udp, test_make_udp_socket) {
  // ソケットの作成
  int sock = make_udp_socket();

  // ソケットが０より大きいかを確認
  EXPECT_GT(sock, 0);

  // ソケットのクローズ
  close(sock);
}

// UDP:UDPソケットとアドレス構造体を結びつけることができているかを確認する
TEST(udp, test_bind_sock) {
  int sock;
  struct sockaddr_in *addr;
  unsigned short port = 11113;

  sock = make_udp_socket();
  addr = (sockaddr_in *)malloc(sizeof(sockaddr_in));

  // アドレス構造体を設定
  set_server_addr(addr, port);

  // bindが成功することを確認する
  int res = -1;
  res = bind_sock(sock, addr);

  EXPECT_EQ(res, 0);

  // ソケットのクローズ
  close(sock);
  free(addr);
}

// udp:メッセージの送受信が正しくできているかを確認する
TEST(udp, send_and_receive_message) {
  int csock, ssock;
  struct sockaddr_in *target_addr, *server_addr, *from_addr;
  target_addr = (struct sockaddr_in *)malloc(sizeof(sockaddr_in));
  server_addr = (struct sockaddr_in *)malloc(sizeof(sockaddr_in));
  from_addr = (struct sockaddr_in *)malloc(sizeof(sockaddr_in));

  char ip[16] = IP;
  unsigned short port = 11114;

  // クライアントサイドのソケットを作成
  csock = make_udp_socket();

  // サーバーサイドのソケットを作成
  ssock = make_udp_socket();

  pid_t pid;
  // 受信用バッファ
  char send_msg[BUFFSIZE] = MSG;
  char recv_msg[BUFFSIZE];

  int send_len, recv_len;

  // ソケットが正しく作成できているかの確認
  EXPECT_NE(csock, -1);
  EXPECT_NE(ssock, -1);

  // 送信先の定義
  set_target_addr(target_addr, ip, port);

  // 受信用の定義
  set_server_addr(server_addr, port);

  // 受信用ソケットとポートの結びつけ
  bind_sock(ssock, server_addr);

  // フォーク
  pid = fork();
  if (pid > 0) {
    // 親プロセスで受信
    recv_len = recv_udp_msg(ssock, from_addr, recv_msg, BUFFSIZE);
    // 送信元のアドレスを表示
  } else if (pid == 0) {
    // 子供プロセスで送信
    send_len = send_udp_msg(csock, target_addr, send_msg);

    _exit(0);
  } else {
    // error
    perror("fork");
  }
  // 受信した文字列が意図したものであるかを確認
  EXPECT_EQ(strcmp(send_msg, MSG), 0);

  // 受信した文字列と送信した文字列が正しいかを確認
  EXPECT_EQ(strcmp(send_msg, recv_msg), 0);

  // 受信元のアドレスが正しいかを確認
  EXPECT_EQ(strcmp(inet_ntoa(from_addr->sin_addr), "127.0.0.1"), 0);

  // ソケットのクローズ
  close(csock);
  close(ssock);

  // メモリの解放
  free(target_addr);
  free(server_addr);
  free(from_addr);
}

// TCP:ソケットの作成が正しくできているかを確認する
TEST(tcp, test_make_tcp_socket) {
  int sock = make_tcp_socket();

  // ソケットが0より大きいかを確認
  EXPECT_GT(sock, 0);

  // ソケットのクローズ
  close(sock);
}

// TCP:TCPソケットとアドレス構造体を結びつけることができているかを確認する
TEST(tcp, test_bind_sock) {
  int sock;
  struct sockaddr_in *addr;
  unsigned short port;

  sock = make_tcp_socket();
  addr = (sockaddr_in *)malloc(sizeof(sockaddr_in));
  port = 11115;

  // アドレス構造体を設定
  set_server_addr(addr, port);

  // bindが成功することを確認する
  int res = -1;
  res = bind_sock(sock, addr);

  EXPECT_EQ(res, 0);

  // ソケットのクローズ
  close(sock);

  // アドレスの解放
  free(addr);
}

// UDP:設定したポート番号がソケットから取得できているかを確認する
TEST(common, test_get_port_number_from_sock) {
  int sock;
  struct sockaddr_in *addr;
  unsigned port = 11118;

  addr = (sockaddr_in *)malloc(sizeof(sockaddr_in));

  // ソケットを作成
  sock = make_udp_socket();

  // アドレス構造体を設定
  set_server_addr(addr, port);

  // アドレス構造体をソケットに結びつける
  bind_sock(sock, addr);

  // ソケットからポート番号を取得できているかを確認する。
  EXPECT_EQ(get_port_number_from_sock(sock), port);

  free(addr);
  close(sock);
}

// TCP:listen,accept,connectionが正しくできているかを確認する
TEST(tcp, test_liten_accept_connection) {
  int sock, scsock, csock;
  struct sockaddr_in *target_addr, *server_addr, *from_addr;
  pid_t pid;

  // IPアドレスとポート番号の定義
  char ip[16] = IP;
  unsigned short port = 11116;

  // tcpソケットの作成
  sock = make_tcp_socket();
  scsock = make_tcp_socket();
  csock = make_tcp_socket();

  // アドレス構造体にそれぞれメモリを確保
  target_addr = (sockaddr_in *)malloc(sizeof(sockaddr_in));
  server_addr = (sockaddr_in *)malloc(sizeof(sockaddr_in));
  from_addr = (sockaddr_in *)malloc(sizeof(sockaddr_in));

  // サーバー用のアドレス構造体を設定
  set_server_addr(server_addr, port);

  // bindが成功することを確認する
  EXPECT_EQ(bind_sock(sock, server_addr), 0);

  // listenが成功することを確認する
  EXPECT_EQ(listen_connection(sock, MAX_PENDING), sock);

  // クライアント用に接続先のアドレス構造体を設定
  set_target_addr(target_addr, ip, port);
  // フォーク
  pid = fork();
  if (pid > 0) {
    // 親プロセス
    // acceptが成功することを確認する
    scsock = accept_connection(sock, from_addr);
    // 送信元のアドレスを表示
  } else if (pid == 0) {
    // 子供プロセスでコネクションをする
    EXPECT_EQ(connect_server(csock, target_addr), 0);
    _exit(0);
  } else {
    // error
    perror("fork");
  }

  // ソケットのクローズ
  close(sock);
  close(csock);

  // アドレスの解放
  free(target_addr);
  free(server_addr);
  free(from_addr);
}

// TCP:送信した内容が正しく、受信できているかを確認する　TODO.短い間隔でテストを実行するとポートが使用できないことがあるので修正したい
TEST(tcp, test_send_recv) {
  int sock, scsock, csock;
  struct sockaddr_in *target_addr, *server_addr, *from_addr;
  pid_t pid;

  // 受信用バッファ
  char send_msg[BUFFSIZE] = MSG;
  char recv_msg[BUFFSIZE];

  // IPアドレスとポート番号の定義
  char ip[16] = IP;
  // 前のテストで使用したポートが使用できないことがあるので+1
  unsigned short port = 11117;

  // tcpソケットの作成
  sock = make_tcp_socket();
  scsock = make_tcp_socket();
  csock = make_tcp_socket();

  // アドレス構造体にそれぞれメモリを確保
  target_addr = (sockaddr_in *)malloc(sizeof(sockaddr_in));
  server_addr = (sockaddr_in *)malloc(sizeof(sockaddr_in));
  from_addr = (sockaddr_in *)malloc(sizeof(sockaddr_in));

  // サーバー用のアドレス構造体を設定
  set_server_addr(server_addr, port);

  // bindが成功することを確認する
  EXPECT_EQ(bind_sock(sock, server_addr), 0);

  // listenが成功することを確認する
  EXPECT_EQ(listen_connection(sock, MAX_PENDING), sock);

  // クライアント用に接続先のアドレス構造体を設定
  set_target_addr(target_addr, ip, port);
  // フォーク
  pid = fork();
  if (pid > 0) {
    // 親プロセス
    // acceptが成功することを確認する
    scsock = accept_connection(sock, from_addr);
    recv_tcp_msg(scsock, recv_msg, BUFFSIZE);
    // 送信元のアドレスを表示
  } else if (pid == 0) {
    // 子供プロセスでコネクションをする
    EXPECT_EQ(connect_server(csock, target_addr), 0);
    send_tcp_msg(csock, send_msg);

    _exit(0);
  } else {
    // error
    perror("fork");
  }

  // 受信したメッセージが一致することを確認する
  EXPECT_EQ(strcmp(send_msg, recv_msg), 0);

  // ソケットのクローズ
  close(sock);
  close(scsock);
  close(csock);

  // アドレスの解放
  free(target_addr);
  free(server_addr);
  free(from_addr);
}

// MULTICAST:ソケットの作成ができているかを確認する
TEST(multicast, test_make_socket) {
  int sock;
  int ttl;
  ttl = 0;
  socklen_t len;
  len = sizeof(ttl);
  // multicastソケットの作成
  sock = make_udp_socket_with_ttl();

  // ソケットが作成できていることを確認する
  EXPECT_GT(sock, 0);

  getsockopt(sock, IPPROTO_IP, IP_MULTICAST_TTL, (void *)&ttl, &len);

  // ttlが設定されていることを確認する
  EXPECT_EQ(ttl, 1);

  // ソケットのクローズ
  close(sock);
}

// MULTIICAST:multicastグループに参加できているかを確認する
TEST(multicast, test_join_group) {
  int sock;
  struct sockaddr_in *addr;
  addr = (sockaddr_in *)malloc(sizeof(sockaddr_in));
  struct ip_mreq *multicast;
  multicast = (ip_mreq *)malloc(sizeof(ip_mreq));

  char ip[16] = "224.0.0.1";

  unsigned short port = 11119;

  // udpソケットの作成
  sock = make_udp_socket();

  // サーバー用のアドレス構造体を設定
  set_server_addr(addr, port);

  // bindが成功することを確認する
  EXPECT_EQ(bind_sock(sock, addr), 0);

  // multicastグループに参加することが成功することを確認する
  EXPECT_EQ(join_multicast_group(sock, multicast, ip), 0);

  // multicastに正しくipアドレスが設定されていることを確認する
  EXPECT_EQ(multicast->imr_multiaddr.s_addr, inet_addr(ip));

  // ソケットのクローズ
  close(sock);

  // アドレスの解放
  free(addr);
}