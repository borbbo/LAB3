#include <QApplication>
#include <QWidget>
#include <QGridLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QDebug>

class Calculator : public QWidget {
public:
    Calculator(QWidget *parent = nullptr) : QWidget(parent) {
        // 1. UI 초기화
        setWindowTitle("Qt Calculator");
        
        // 디스플레이 창 설정
        display = new QLineEdit("0");
        display->setReadOnly(true);
        display->setAlignment(Qt::AlignRight);
        display->setFixedHeight(50);
        
        // 레이아웃 설정
        QGridLayout *layout = new QGridLayout;
        layout->addWidget(display, 0, 0, 1, 4); // (row, col, rowSpan, colSpan)

        // 2. 버튼 생성 및 배치
        const char *buttons[4][4] = {
            {"7", "8", "9", "/"},
            {"4", "5", "6", "*"},
            {"1", "2", "3", "-"},
            {"0", "C", "=", "+"}
        };

        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                QPushButton *btn = new QPushButton(buttons[i][j]);
                btn->setFixedSize(60, 60);
                layout->addWidget(btn, i + 1, j);
                
                // 버튼 클릭 이벤트 연결
                connect(btn, &QPushButton::clicked, this, [=]() {
                    handleButton(btn->text());
                });
            }
        }

        setLayout(layout);
        
        // 3. 변수 초기화
        currentValue = 0.0;
        storedValue = 0.0;
        waitingForOperand = true;
        pendingOperator = "";
    }

private:
    QLineEdit *display;
    double currentValue;
    double storedValue;
    bool waitingForOperand;
    QString pendingOperator;

    // 버튼 클릭 처리 로직
    void handleButton(QString key) {
        if (key >= "0" && key <= "9") {
            // 숫자 입력 처리
            if (waitingForOperand) {
                display->setText(key);
                waitingForOperand = false;
            } else {
                if (display->text() == "0") display->setText(key);
                else display->setText(display->text() + key);
            }
        } else if (key == "C") {
            // 초기화
            display->setText("0");
            storedValue = 0.0;
            pendingOperator = "";
            waitingForOperand = true;
        } else if (key == "=") {
            // 계산 수행
            if (!pendingOperator.isEmpty()) {
                double result = calculate(storedValue, display->text().toDouble(), pendingOperator);
                display->setText(QString::number(result));
                pendingOperator = "";
                waitingForOperand = true;
                storedValue = result; // 연속 계산을 위해 저장
            }
        } else {
            // 연산자 (+, -, *, /) 처리
            if (!waitingForOperand) {
                if (!pendingOperator.isEmpty()) {
                    // 이전 연산 수행
                    double result = calculate(storedValue, display->text().toDouble(), pendingOperator);
                    display->setText(QString::number(result));
                    storedValue = result;
                } else {
                    storedValue = display->text().toDouble();
                }
                waitingForOperand = true;
            }
            pendingOperator = key;
        }
    }

    double calculate(double left, double right, QString op) {
        if (op == "+") return left + right;
        if (op == "-") return left - right;
        if (op == "*") return left * right;
        if (op == "/") {
            if (right == 0.0) return 0.0; // 0으로 나누기 방지
            return left / right;
        }
        return right;
    }
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    Calculator calc;
    calc.show();
    return app.exec();
}
