#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#define NUM_THREADS 3

// 쓰레드가 실행할 함수
void *thread_function(void *arg) {
    long id = (long)arg; // 전달받은 인자(쓰레드 번호)
    pthread_t tid = pthread_self(); // 자신의 쓰레드 ID 확인

    printf("[Thread %ld] Created. TID: %lu\n", id, (unsigned long)tid);
    
    // 작업을 시뮬레이션하기 위한 대기 (번호에 따라 다르게 대기)
    sleep(id + 1);

    printf("[Thread %ld] Finished.\n", id);
    
    // 쓰레드 종료 및 반환값 설정 (id 반환)
    pthread_exit((void*)id);
}

int main() {
    pthread_t threads[NUM_THREADS];
    int rc;
    long t;
    void *status;

    printf("Main: Creating %d threads...\n", NUM_THREADS);

    // 1. 쓰레드 생성 (Create)
    for(t = 0; t < NUM_THREADS; t++) {
        printf("Main: Creating thread %ld\n", t);
        // 인자로 t(0, 1, 2)를 넘겨줌
        rc = pthread_create(&threads[t], NULL, thread_function, (void *)t);
        
        if (rc) {
            printf("Error: return code from pthread_create() is %d\n", rc);
            exit(1);
        }
    }

    // 2. 쓰레드 종료 대기 (Join)
    for(t = 0; t < NUM_THREADS; t++) {
        // 해당 쓰레드가 종료될 때까지 메인 쓰레드는 여기서 멈춤
        rc = pthread_join(threads[t], &status);
        
        if (rc) {
            printf("Error: return code from pthread_join() is %d\n", rc);
            exit(1);
        }
        
        printf("Main: Completed join with thread %ld having a status of %ld\n", t, (long)status);
    }

    printf("Main: All threads completed. Exiting.\n");
    return 0;
}
