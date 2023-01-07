#include <arpa/inet.h>
/* 通信に関する処理*/

/* ==========共通の処理処理==========*/
//  相手のアドレス構造体を設定する関数
int set_target_addr(struct sockaddr_in *addr, const char ip[],
                    unsigned short port);
// サーバーのアドレス構造体を設定する処理
int set_server_addr(struct sockaddr_in *addr, unsigned short port);
// udp: ポート番号はなしで、アドレス構造体を設定する処理
int set_server_addr_non_port(struct sockaddr_in *addr);
// ポートにbindする処理
int bind_sock(int sock, struct sockaddr_in *addr);
// sockにbindされたポート番号を取得する処理
int get_port_number_from_sock(int sock);

/* ==========udpの処理==========*/
// udp：ソケットを作成する関数
int make_udp_socket();
// udp：メッセージを送信する処理
int send_udp_msg(int sock, struct sockaddr_in *addr, char *buff);
// udp：メッセージを受信する処理
int recv_udp_msg(int sock, struct sockaddr_in *addr, char *buff, int buffsize);

/* ==========tcpの処理==========*/
// tcp：ソケットを作成する処理
int make_tcp_socket();
// tcp：接続を待ち受ける処理
int listen_connection(int sock, unsigned short max_pending);
// tcp：接続を受け付ける処理
int accept_connection(int sock, struct sockaddr_in *addr);
// tcp：接続を行う処理
int connect_server(int sock, struct sockaddr_in *addr);
// tcp：メッセージを送信する処理
int send_tcp_msg(int sock, char *buff);
// tcp：メッセージを受信する処理
int recv_tcp_msg(int sock, char *buff, int buffsize);

/* ==========multiCastの処理==========*/
// multiCast：ソケットを作成する処理(基本はudpと同じだが、TTLが１になっている)
int make_udp_socket_with_ttl();
// multiCast：グループに参加する処理
int join_multicast_group(int sock, struct ip_mreq *multicast, char *ip);
