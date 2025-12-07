#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 1024
#define PORT 9000 // 서버와 동일한 포트 번호

void error_handling(char *message);

int main(int argc, char *argv[])
{
    int sock;
    struct sockaddr_in serv_addr;
    char message[BUF_SIZE];
    int str_len;

    // 접속할 서버 IP 주소 설정 (자신에게 접속할 경우 127.0.0.1)
    char *server_ip = "127.0.0.1"; 
    
    // 1. 소켓 생성
    sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock == -1)
        error_handling("socket() error");

    // 2. 서버 주소 정보 초기화
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(server_ip);
    serv_addr.sin_port = htons(PORT);

    // 3. 서버에 연결 요청 (Connect)
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
        error_handling("connect() error!");
    else
        printf("Connected to Server (%s:%d). Press 'Q' to quit.\n", server_ip, PORT);

    // 4. 데이터 송수신
    while (1)
    {
        fputs("Input message: ", stdout);
        fgets(message, BUF_SIZE, stdin);

        // 'q'나 'Q' 입력 시 종료
        if (!strcmp(message, "q\n") || !strcmp(message, "Q\n"))
            break;

        // 서버로 메시지 전송
        write(sock, message, strlen(message));

        // 서버로부터 메시지 수신
        str_len = read(sock, message, BUF_SIZE - 1);
        if (str_len == -1)
            error_handling("read() error!");

        message[str_len] = 0; // 문자열 끝 처리
        printf("Message from server: %s", message);
    }

    // 5. 소켓 종료
    close(sock);
    return 0;
}

void error_handling(char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}
