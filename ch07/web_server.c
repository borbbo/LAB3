#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>

#define BUF_SIZE 4096
#define PORT 8080

void error_handling(char *message);
void handle_request(int clnt_sock);
void send_file(int clnt_sock, char *path);
void handle_post_cgi(int clnt_sock, char *path, char *body);

int main(int argc, char *argv[])
{
    int serv_sock, clnt_sock;
    struct sockaddr_in serv_adr, clnt_adr;
    socklen_t clnt_adr_sz;

    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    int option = 1;
    setsockopt(serv_sock, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_adr.sin_port = htons(PORT);

    if (bind(serv_sock, (struct sockaddr *)&serv_adr, sizeof(serv_adr)) == -1)
        error_handling("bind() error");
    if (listen(serv_sock, 5) == -1)
        error_handling("listen() error");

    printf("Web Server started on port %d...\n", PORT);

    while (1)
    {
        clnt_adr_sz = sizeof(clnt_adr);
        clnt_sock = accept(serv_sock, (struct sockaddr *)&clnt_adr, &clnt_adr_sz);
        if (clnt_sock == -1) continue;

        // 요청 처리 (단일 프로세스 - 순차 처리)
        // fork()나 thread를 써야 하지만, CGI fork()와 헷갈리지 않게 함수 호출로 처리
        handle_request(clnt_sock);
        close(clnt_sock);
    }
    close(serv_sock);
    return 0;
}

void handle_request(int clnt_sock)
{
    char req_buf[BUF_SIZE] = {0,};
    char method[10];
    char path[100];
    char *body_ptr;
    int content_length = 0;

    // 1. 요청 읽기
    read(clnt_sock, req_buf, BUF_SIZE - 1);
    
    // 2. 요청 라인 파싱 (Method, Path)
    sscanf(req_buf, "%s %s", method, path);
    printf("Request: %s %s\n", method, path);

    // 3. GET 요청 처리 (정적 파일)
    if (strcmp(method, "GET") == 0)
    {
        if (strcmp(path, "/") == 0) // 기본 페이지
            strcpy(path, "/index.html");
        
        // 경로 앞에 . 을 붙여 현재 디렉토리 파일로 인식하게 함
        char local_path[100] = ".";
        strcat(local_path, path);
        
        send_file(clnt_sock, local_path);
    }
    // 4. POST 요청 처리 (CGI 실행)
    else if (strcmp(method, "POST") == 0)
    {
        // Body 시작 위치 찾기 (\r\n\r\n 뒤가 Body)
        body_ptr = strstr(req_buf, "\r\n\r\n");
        if(body_ptr) body_ptr += 4; // 헤더 끝 다음 포인터
        
        handle_post_cgi(clnt_sock, "./post_handler.py", body_ptr);
    }
}

// 정적 파일 전송 함수 (GET)
void send_file(int clnt_sock, char *path)
{
    char buf[BUF_SIZE];
    FILE *fp = fopen(path, "rb"); // 바이너리 읽기 모드

    if (fp == NULL)
    {
        // 404 Error 처리
        char *header = "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\n\r\n";
        write(clnt_sock, header, strlen(header));
        write(clnt_sock, "404 Not Found", 13);
    }
    else
    {
        // 200 OK 헤더 전송
        char *header = "HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=utf-8\r\n\r\n";
        write(clnt_sock, header, strlen(header));

        // 파일 내용 전송
        while (!feof(fp))
        {
            int len = fread(buf, 1, BUF_SIZE, fp);
            write(clnt_sock, buf, len);
        }
        fclose(fp);
    }
}

// CGI 실행 함수 (POST)
void handle_post_cgi(int clnt_sock, char *path, char *body)
{
    int input_pipe[2]; // 부모 -> 자식 데이터 전달용 파이프
    pid_t pid;

    pipe(input_pipe);
    pid = fork();

    if (pid == 0) // 자식 프로세스 (CGI 실행)
    {
        // 1. 표준 입력을 파이프로 연결 (부모가 준 Body를 읽기 위함)
        dup2(input_pipe[0], STDIN_FILENO); 
        
        // 2. 표준 출력을 클라이언트 소켓으로 연결 (스크립트 출력 -> 브라우저)
        dup2(clnt_sock, STDOUT_FILENO); 
        
        // 사용 안하는 파이프 끝 닫기
        close(input_pipe[1]);
        close(input_pipe[0]);

        // 3. HTTP 응답 헤더 먼저 출력 (스크립트가 출력하기 전에)
        // 실제 CGI는 스크립트가 헤더를 포함하기도 하지만 여기선 단순화
        printf("HTTP/1.1 200 OK\r\n");
        printf("Content-Type: text/html; charset=utf-8\r\n\r\n");
        
        // 4. 스크립트 실행
        execl("/usr/bin/python3", "python3", path, NULL);
        exit(0);
    }
    else // 부모 프로세스
    {
        close(input_pipe[0]); // 읽기 전용 닫기
        
        // POST Body 데이터를 파이프를 통해 자식에게 전달
        if(body != NULL)
            write(input_pipe[1], body, strlen(body));
            
        close(input_pipe[1]); // 쓰기 완료 후 닫기 -> 자식에게 EOF 전달됨
        wait(NULL); // 자식 종료 대기
    }
}

void error_handling(char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}
