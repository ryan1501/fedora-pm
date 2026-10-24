#pragma once
#include <QMainWindow>

class QLineEdit;
class QPlainTextEdit;
class QPushButton;

namespace fedora_pm::gui {
class MainWindow final : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
private:
    QLineEdit* query_;
    QPlainTextEdit* output_;
    QPushButton* search_;
    void run_search();
};
}
