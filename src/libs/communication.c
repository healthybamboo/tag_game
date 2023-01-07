#include "communication.h"

#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "setting.h"

/* ==========共通の処理処理==========*/

//  相手のアドレス構造体を設定する関数
int set_target_addr(struct sockaddr_in *addr, const char ip[],
                    unsigned short port) {
  addr->sin_family = AF_INET;
  addr->sin_addr.s_addr = inet_addr(ip);
  addr->sin_port = htons(port);
  return 0;
}

// サーバーのアドレス構造体を設定する処理
int set_server_addr(struct sockaddr_in *addr, unsigned short port) {
  addr->sin_family = AF_INET;
  addr->sin_addr.s_addr = htonl(INADDR_ANY);
  addr->sin_port = htons(port);
  return 0;
}

// ポート番号はなしで、アドレス構造体を設定する処理
int set_server_addr_non_port(struct sockaddr_in *addr) {
  addr->sin_family = AF_INET;
  addr->sin_addr.s_addr = htonl(INADDR_ANY);
  return 0;
}

// ポートにbindする処理
int bind_sock(int sock, struct sockaddr_in *addr) {
  if (bind(sock, (struct sockaddr *)addr, sizeof(*addr)) < 0) {
    perror("bind() faild");
    exit(1);
  }
  return 0;
}

// sockにbindされたポート番号を取得する処理
int get_port_number_from_sock(int sock) {
  struct sockaddr_in addr;
  socklen_t len = sizeof(addr);
  if (getsockname(sock, (struct sockaddr *)&addr, &len) < 0) {
    perror("getsockname() failed");
    exit(1);
  }
  return ntohs(addr.sin_port);
}

/* ==========udpの処理==========*/
/* udp：ソケットを作成する関数 */
int make_udp_socket() {
  int sock;
  if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
    perror("socket() failed");
    exit(1);
  }
  return sock;
}

// udp：メッセージを送信する処理
int send_udp_msg(int sock, struct sockaddr_in *addr, char *buff) {
  int bytes;  // 送信メッセージのバイト数
  int len;
  len = strlen(buff);
  if ((bytes = sendto(sock, buff, len, 0, (struct sockaddr *)addr,
                      sizeof(*addr))) < 0) {
    perror("sendto() failed");
    exit(1);
  }
  return bytes;
}

// udp：メッセージを受信する処理
int recv_udp_msg(int sock, struct sockaddr_in *addr, char *buff, int buffsize) {
  // 受信メッセージのバイト数
  int bytes;

  // 送信元のアドレス構造体
  struct sockaddr_in caddr;

  // 送信元のアドレス構造体のサイズ
  unsigned int csize;

  csize = sizeof(caddr);
  if ((bytes = recvfrom(sock, buff, BUFFSIZE - 1, 0, (struct sockaddr *)&caddr,
                        &csize)) <= 0) {
    perror("recvfrom() failed");
    exit(1);
  }

  buff[bytes] = '\0';  // 文字列の終端を設定する

  // 送信元のアドレスをaddrにコピーする
  memcpy(addr, &caddr, sizeof(caddr));
  return bytes;
}

/* ==========tcpの処理==========*/
// tcp：メッセージを送信する処理
int make_tcp_socket() {
  int sock;
  if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
    perror("socket() failed");
    exit(1);
  }
  return sock;
};

// tcp:着信を設定する処理
int listen_connection(int sock, unsigned short max_pending) {
  if (listen(sock, max_pending) < 0) {
    perror("listen() failed");
    exit(1);
  }
  return sock;
}

// tcp:接続を受け付ける処理
int accept_connection(int sock, struct sockaddr_in *addr) {
  int csock;
  unsigned int csize;
  struct sockaddr_in caddr;
  csize = sizeof(caddr);
  if ((csock = accept(sock, (struct sockaddr *)&caddr, &csize)) < 0) {
    perror("accept() failed");
    exit(1);
  }
  // 送信元のアドレスをaddrにコピーする
  memcpy(addr, &caddr, sizeof(caddr));

  return csock;
}
// tcp:接続を要求する処理
int connect_server(int sock, struct sockaddr_in *addr) {
  if (connect(sock, (struct sockaddr *)addr, sizeof(*addr)) < 0) {
    perror("connect() failed");
    exit(1);
  }
  return 0;
}

// tcp:メッセージを送信する処理
int send_tcp_msg(int sock, char *buff) {
  int bytes;
  int len;
  len = strlen(buff);
  if ((bytes = send(sock, buff, len, 0)) != len) {
    perror("send() failed");
    exit(1);
  }
  return bytes;
}

// tcp：メッセージを受信する処理
int recv_tcp_msg(int sock, char *buff, int buffsize) {
  int bytes;
  if ((bytes = recv(sock, buff, buffsize - 1, 0)) <= 0) {
    perror("recv() failed");
    exit(1);
  }
  // 文字列の終端を設定する
  buff[bytes] = '\0';
  return bytes;
}

/* ==========multiCastの処理==========*/
// multicast: マルチキャストを設定する処理
int set_multicast(int sock, char *ip, struct ip_mreq *multicast) {
  struct ip_mreq mreq;
  // マルチキャストアドレスを設定する
  multicast->imr_multiaddr.s_addr = inet_addr(ip);

  // マルチキャストを受信するインターフェースを設定する
  multicast->imr_interface.s_addr = htonl(INADDR_ANY);

  // マルチキャストグループに参加する
  if (setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (void *)&multicast,
                 sizeof(multicast)) < 0) {
    perror("setsockopt() failed");
    exit(1);
  }
  return 0;
}

// multiCast：ソケットを作成する処理（忘れると危険なので一緒にTTLも設定する）
// 尚、送信処理と受信処理はUDPと同じ
int make_udp_socket_with_ttl() {
  int sock;
  // ソケットを作成するUDP（敢えて関数は呼び出さない）
  if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
    perror("socket() failed");
    exit(1);
  }

  int ttl = 1;
  // マルチキャストTTLの設定
  if (setsockopt(sock, IPPROTO_IP, IP_MULTICAST_TTL, (void *)&ttl,
                 sizeof(ttl)) < 0) {
    perror("setsockopt() failed");
    exit(1);
  }
  return sock;
}

// multiCast：グループに参加する処理
int join_multicast_group(int sock, struct ip_mreq *multicast, char *ip) {
  // マルチキャストアドレスを設定する
  multicast->imr_multiaddr.s_addr = inet_addr(ip);

  // マルチキャストを受信するインターフェースを設定する
  multicast->imr_interface.s_addr = htonl(INADDR_ANY);

  // マルチキャストグループに参加する
  if (setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (void *)multicast,
                 sizeof(*multicast)) < 0) {
    perror("setsockopt() failed");
    exit(1);
  }
  return 0;
}
