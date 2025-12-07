#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 1024
#define PORT 9000 // 사용할 포트 번호

void error_handling(char *message);

int main(int argc, char *argv[])
{
    int serv_sock, clnt_sock;
    struct sockaddr_in serv_addr, clnt_addr;
    socklen_t clnt_addr_size;
    char message[BUF_SIZE];
    int str_len;

    // 1. 서버 소켓 생성 (IPv4, TCP)
    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    if (serv_sock == -1)
        error_handling("socket() error");

    // 2. 주소 정보 초기화 및 할당 (IP, Port)
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); // 현재 PC의 IP 자동 할당
    serv_addr.sin_port = htons(PORT);

    // 3. 소켓에 주소 할당 (Binding)
    if (bind(serv_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
        error_handling("bind() error");

    // 4. 연결 요청 대기 상태 진입 (Listening)
    if (listen(serv_sock, 5) == -1)
        error_handling("listen() error");

    printf("Echo Server is running on port %d...\n", PORT);

    clnt_addr_size = sizeof(clnt_addr);

    // 5. 연결 요청 수락 (Accept) - 연결될 때까지 대기(Blocking)
    // 반복문을 통해 여러 클라이언트를 순차적으로 처리 가능 (1:1 예시 후 종료됨)
    // 3명의 클라이언트만 순차적으로 처리하고 종료하도록 구현함.
    for (int i = 0; i < 3; i++) 
    {
        printf("Wait for client %d...\n", i + 1);
        clnt_sock = accept(serv_sock, (struct sockaddr *)&clnt_addr, &clnt_addr_size);
        if (clnt_sock == -1)
            error_handling("accept() error");
        else
            printf("Client %d connected!\n", i + 1);

        // 6. 데이터 송수신 (Echo)
        // 클라이언트가 EOF를 보낼 때까지(연결 종료 시까지) 반복해서 읽고 씀
        while ((str_len = read(clnt_sock, message, BUF_SIZE)) != 0)
        {
            write(clnt_sock, message, str_len); // 받은 만큼 그대로 전송
        }

        // 7. 클라이언트 소켓 종료
        close(clnt_sock);
        printf("Client %d disconnected.\n", i + 1);
    }

    // 8. 서버 소켓 종료
    close(serv_sock);
    return 0;
}

void error_handling(char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}
