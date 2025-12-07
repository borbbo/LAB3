#include <QApplication>
#include <QPushButton>

int main(int argc, char **argv) {
    // 1. Qt 애플리케이션 객체 생성
    QApplication app(argc, argv);

    // 2. 버튼 위젯 생성 및 속성 설정
    QPushButton button("Hello Qt World!");
    button.resize(200, 100);

    // 3. 버튼 화면에 표시 (윈도우가 없으면 버튼 자체가 윈도우가 됨)
    button.show();

    // 4. 이벤트 루프 진입
    return app.exec();
}
