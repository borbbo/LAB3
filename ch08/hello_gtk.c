#include <gtk/gtk.h>
#include <stdio.h>

// 버튼 클릭 시 호출될 콜백 함수
void on_button_clicked(GtkWidget *widget, gpointer data) {
    g_print("Hello World! Button clicked.\n");
}

// 윈도우 닫기 버튼(X) 클릭 시 호출될 콜백 함수
// 이 함수가 없으면 윈도우는 사라지지만 프로세스는 종료되지 않음
void on_destroy(GtkWidget *widget, gpointer data) {
    gtk_main_quit();
}

int main(int argc, char *argv[]) {
    GtkWidget *window;
    GtkWidget *button;

    // 1. GTK 초기화
    gtk_init(&argc, &argv);

    // 2. 윈도우 생성 및 속성 설정
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Hello GTK");
    gtk_window_set_default_size(GTK_WINDOW(window), 200, 200);
    gtk_container_set_border_width(GTK_CONTAINER(window), 10);

    // 3. 버튼 생성
    button = gtk_button_new_with_label("Click Me");

    // 4. 시그널 연결 (이벤트 핸들러)
    // 버튼 클릭 -> on_button_clicked 함수 실행
    g_signal_connect(button, "clicked", G_CALLBACK(on_button_clicked), NULL);
    
    // 윈도우 닫기 -> on_destroy 함수 실행 (프로그램 종료)
    g_signal_connect(window, "destroy", G_CALLBACK(on_destroy), NULL);

    // 5. 위젯 배치 (윈도우 안에 버튼 넣기)
    gtk_container_add(GTK_CONTAINER(window), button);

    // 6. 모든 위젯 화면에 표시
    gtk_widget_show_all(window);

    // 7. 메인 루프 진입 (이벤트 대기)
    gtk_main();

    return 0;
}
