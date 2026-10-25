#include "gui/main_window.hpp"
#include <QApplication>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    fedora_pm::gui::MainWindow window;
    window.show();
    return app.exec();
}
