#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

// 1. 설정값 정의
#define BUFFER_SIZE 5       // 버퍼의 크기 (제한 버퍼)
#define NUM_PRODUCERS 2     // 생산자 쓰레드 수
#define NUM_CONSUMERS 2     // 소비자 쓰레드 수
#define MAX_ITEMS 20        // 각 생산자가 만들 아이템 개수

// 2. 공유 자원 구조체 정의
typedef struct {
    int buffer[BUFFER_SIZE]; // 실제 데이터를 담을 배열
    int in;                  // 생산자가 데이터를 넣을 인덱스
    int out;                 // 소비자가 데이터를 꺼낼 인덱스
    int count;               // 현재 버퍼에 차 있는 데이터 개수
    pthread_mutex_t mutex;   // 상호 배제를 위한 뮤텍스
    pthread_cond_t not_full; // 버퍼가 가득 차지 않았음을 알리는 조건 변수 (생산자용)
    pthread_cond_t not_empty;// 버퍼가 비어 있지 않음을 알리는 조건 변수 (소비자용)
} BoundedBuffer;

// 전역 공유 버퍼 선언
BoundedBuffer bb;

// 3. 버퍼 초기화 함수
void init_buffer(BoundedBuffer *b) {
    b->in = 0;
    b->out = 0;
    b->count = 0;
    pthread_mutex_init(&b->mutex, NULL);
    pthread_cond_init(&b->not_full, NULL);
    pthread_cond_init(&b->not_empty, NULL);
}

// 4. 생산자 쓰레드 함수
void *producer(void *arg) {
    long id = (long)arg;
    
    for (int i = 0; i < MAX_ITEMS; i++) {
        int item = (rand() % 100); // 0~99 사이의 랜덤 데이터 생성
        
        // --- 임계 영역 시작 (Critical Section) ---
        pthread_mutex_lock(&bb.mutex);

        // 버퍼가 가득 찼다면 기다림 (wait)
        // while 루프를 쓰는 이유: 깨어났을 때 다시 조건을 검사하기 위해 (Spurious Wakeup 방지)
        while (bb.count == BUFFER_SIZE) {
            printf("[생산자 %ld] 버퍼가 가득 찼습니다. 대기 중...\n", id);
            pthread_cond_wait(&bb.not_full, &bb.mutex);
        }

        // 데이터 삽입
        bb.buffer[bb.in] = item;
        bb.in = (bb.in + 1) % BUFFER_SIZE; // 원형 버퍼 로직
        bb.count++;
        
        printf("[생산자 %ld] 데이터 %d 생산 (개수: %d)\n", id, item, bb.count);

        // 소비자를 깨움 (버퍼가 비어있지 않다고 신호 보냄)
        pthread_cond_signal(&bb.not_empty);

        pthread_mutex_unlock(&bb.mutex);
        // --- 임계 영역 끝 ---

        // 생산 속도 조절을 위한 잠시 휴식
        usleep((rand() % 100) * 1000); 
    }
    return NULL;
}

// 5. 소비자 쓰레드 함수
void *consumer(void *arg) {
    long id = (long)arg;

    // 예제에서는 무한 루프처럼 돌지만, 실습 종료를 위해 여기서는 생산자와 맞춰서 돎.
    for (int i = 0; i < MAX_ITEMS; i++) {
        
        // --- 임계 영역 시작 (Critical Section) ---
        pthread_mutex_lock(&bb.mutex);

        // 버퍼가 비어있다면 기다림 (wait)
        while (bb.count == 0) {
            printf("\t\t[소비자 %ld] 버퍼가 비었습니다. 대기 중...\n", id);
            pthread_cond_wait(&bb.not_empty, &bb.mutex);
        }

        // 데이터 꺼냄
        int item = bb.buffer[bb.out];
        bb.out = (bb.out + 1) % BUFFER_SIZE; // 원형 버퍼 로직
        bb.count--;

        printf("\t\t[소비자 %ld] 데이터 %d 소비 (개수: %d)\n", id, item, bb.count);

        // 생산자를 깨움 (버퍼가 가득 차지 않았다고 신호 보냄)
        pthread_cond_signal(&bb.not_full);

        pthread_mutex_unlock(&bb.mutex);
        // --- 임계 영역 끝 ---

        // 소비 속도 조절
        usleep((rand() % 150) * 1000);
    }
    return NULL;
}

int main() {
    pthread_t prod_threads[NUM_PRODUCERS];
    pthread_t cons_threads[NUM_CONSUMERS];
    
    // 버퍼 초기화
    init_buffer(&bb);
    
    printf("--- 생산자 소비자 문제 시뮬레이션 시작 ---\n");
    printf("버퍼 크기: %d, 생산자 수: %d, 소비자 수: %d\n\n", BUFFER_SIZE, NUM_PRODUCERS, NUM_CONSUMERS);

    // 6. 쓰레드 생성
    // 생산자 쓰레드 생성
    for (long i = 0; i < NUM_PRODUCERS; i++) {
        if (pthread_create(&prod_threads[i], NULL, producer, (void*)i) != 0) {
            perror("생산자 쓰레드 생성 실패");
            exit(1);
        }
    }

    // 소비자 쓰레드 생성
    for (long i = 0; i < NUM_CONSUMERS; i++) {
        if (pthread_create(&cons_threads[i], NULL, consumer, (void*)i) != 0) {
            perror("소비자 쓰레드 생성 실패");
            exit(1);
        }
    }

    // 7. 쓰레드 종료 대기 (Join)
    for (int i = 0; i < NUM_PRODUCERS; i++) {
        pthread_join(prod_threads[i], NULL);
    }
    for (int i = 0; i < NUM_CONSUMERS; i++) {
        pthread_join(cons_threads[i], NULL);
    }

    // 8. 자원 해제
    pthread_mutex_destroy(&bb.mutex);
    pthread_cond_destroy(&bb.not_full);
    pthread_cond_destroy(&bb.not_empty);

    printf("\n--- 시뮬레이션 종료 ---\n");

    return 0;
}
