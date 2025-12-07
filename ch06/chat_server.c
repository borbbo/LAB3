#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/epoll.h>

#define BUF_SIZE 100
#define MAX_CLNT 256
#define EPOLL_SIZE 50
#define PORT 9000

void send_msg(char *msg, int len);
void error_handling(char *message);

int clnt_cnt = 0;
int clnt_socks[MAX_CLNT]; // 접속한 클라이언트 소켓들을 저장할 배열
pthread_mutex_t mutx;     

int main(int argc, char *argv[])
{
    int serv_sock, clnt_sock;
    struct sockaddr_in serv_adr, clnt_adr;
    socklen_t adr_sz;
    int str_len, i;
    char buf[BUF_SIZE];

    struct epoll_event *ep_events;
    struct epoll_event event;
    int epfd, event_cnt;

    // 1. 서버 소켓 생성 및 설정
    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_adr.sin_port = htons(PORT);

    // Time-wait 에러 방지 옵션
    int option = 1;
    setsockopt(serv_sock, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

    if (bind(serv_sock, (struct sockaddr *)&serv_adr, sizeof(serv_adr)) == -1)
        error_handling("bind() error");
    if (listen(serv_sock, 5) == -1)
        error_handling("listen() error");

    // 2. epoll 인스턴스 생성 및 리스닝 소켓 등록
    epfd = epoll_create(EPOLL_SIZE);
    ep_events = malloc(sizeof(struct epoll_event) * EPOLL_SIZE);

    event.events = EPOLLIN; // 수신 이벤트 감지
    event.data.fd = serv_sock;
    epoll_ctl(epfd, EPOLL_CTL_ADD, serv_sock, &event);

    printf("Chat Server Started on port %d...\n", PORT);

    while (1)
    {
        // 3. 이벤트 대기 (무한 루프)
        event_cnt = epoll_wait(epfd, ep_events, EPOLL_SIZE, -1);
        if (event_cnt == -1)
        {
            puts("epoll_wait() error");
            break;
        }

        for (i = 0; i < event_cnt; i++)
        {
            // A. 새로운 연결 요청이 들어온 경우
            if (ep_events[i].data.fd == serv_sock)
            {
                adr_sz = sizeof(clnt_adr);
                clnt_sock = accept(serv_sock, (struct sockaddr *)&clnt_adr, &adr_sz);
                
                // 클라이언트 배열에 추가
                clnt_socks[clnt_cnt++] = clnt_sock; 
                
                // epoll 관찰 대상에 추가
                event.events = EPOLLIN;
                event.data.fd = clnt_sock;
                epoll_ctl(epfd, EPOLL_CTL_ADD, clnt_sock, &event);
                
                printf("Connected client: %d \n", clnt_sock);
            }
            // B. 클라이언트로부터 메시지가 온 경우
            else
            {
                str_len = read(ep_events[i].data.fd, buf, BUF_SIZE);
                
                // 연결 종료 (EOF 수신)
                if (str_len == 0)
                {
                    // epoll 에서 삭제 및 소켓 닫기
                    epoll_ctl(epfd, EPOLL_CTL_DEL, ep_events[i].data.fd, NULL);
                    close(ep_events[i].data.fd);
                    
                    // 배열에서 제거 (간단하게 구현하기 위해 마지막 요소를 현재 위치로 이동)
                    for(int j=0; j<clnt_cnt; j++) {
                        if(clnt_socks[j] == ep_events[i].data.fd) {
                            while(j < clnt_cnt - 1) {
                                clnt_socks[j] = clnt_socks[j+1];
                                j++;
                            }
                            break;
                        }
                    }
                    clnt_cnt--;
                    printf("Closed client: %d \n", ep_events[i].data.fd);
                }
                // 메시지 수신 (브로드캐스팅)
                else
                {
                    send_msg(buf, str_len);
                }
            }
        }
    }
    close(serv_sock);
    close(epfd);
    return 0;
}

// 모든 클라이언트에게 메시지 전송
void send_msg(char *msg, int len)
{
    int i;
    for (i = 0; i < clnt_cnt; i++)
        write(clnt_socks[i], msg, len);
}

void error_handling(char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}
