#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

// 1. 공유 자원 및 동기화 도구 선언
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

// 이진 플래그 (0: 부모 차례, 1: 자식 차례)
int is_child_turn = 0; 

// 2. 자식 쓰레드 함수
void *child_thread(void *arg) {
    while (1) {
        // 임계 영역 진입
        pthread_mutex_lock(&mutex);

        // 내 차례(1)가 아니면 대기
        // Spurious wakeup 방지를 위해 while 문 사용
        while (is_child_turn != 1) {
            pthread_cond_wait(&cond, &mutex);
        }

        // 작업 수행
        printf("hello child\n");
        sleep(1); // 1초 대기

        // 턴 넘기기 (자식 -> 부모)
        is_child_turn = 0;
        
        // 대기 중인 부모 쓰레드 깨우기
        pthread_cond_signal(&cond);

        // 임계 영역 탈출
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

// 3. 메인 함수 (부모 쓰레드)
int main() {
    pthread_t tid;

    // 자식 쓰레드 생성
    if (pthread_create(&tid, NULL, child_thread, NULL) != 0) {
        perror("Thread create failed");
        exit(1);
    }

    // 부모 쓰레드 로직
    while (1) {
        // 임계 영역 진입
        pthread_mutex_lock(&mutex);

        // 내 차례(0)가 아니면 대기
        while (is_child_turn != 0) {
            pthread_cond_wait(&cond, &mutex);
        }

        // 작업 수행
        printf("hello parent\n");
        sleep(1); // 1초 대기

        // 턴 넘기기 (부모 -> 자식)
        is_child_turn = 1;

        // 대기 중인 자식 쓰레드 깨우기
        pthread_cond_signal(&cond);

        // 임계 영역 탈출
        pthread_mutex_unlock(&mutex);
    }

    // (이 예제는 무한 루프이므로 여기까지 도달하지 않음)
    pthread_join(tid, NULL);
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);

    return 0;
}
