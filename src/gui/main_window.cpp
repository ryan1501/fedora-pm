#include "gui/main_window.hpp"
#include "package_manager/package_manager.hpp"
#include <QHBoxLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

namespace fedora_pm::gui {
MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle("Fedora Package Manager — C++ migration");
    resize(760, 520);
    auto* central = new QWidget(this);
    auto* layout = new QVBoxLayout(central);
    auto* row = new QHBoxLayout;
    query_ = new QLineEdit(central);
    query_->setPlaceholderText("Search packages…");
    search_ = new QPushButton("Search", central);
    row->addWidget(query_);
    row->addWidget(search_);
    output_ = new QPlainTextEdit(central);
    output_->setReadOnly(true);
    output_->setPlaceholderText("Search results and command output appear here.");
    layout->addLayout(row);
    layout->addWidget(output_);
    setCentralWidget(central);
    connect(search_, &QPushButton::clicked, this, &MainWindow::run_search);
    connect(query_, &QLineEdit::returnPressed, this, &MainWindow::run_search);
}
void MainWindow::run_search() {
    const auto query = query_->text().trimmed().toStdString();
    if (query.empty()) {
        QMessageBox::information(this, "Search", "Enter a package search term.");
        return;
    }
    search_->setEnabled(false);
    output_->setPlainText("Searching with DNF…\n");
    const auto result = fedora_pm::packages::PackageManager{}.search(query);
    output_->setPlainText(QString::fromStdString(result.output + (result.error.empty() ? "" : "\n" + result.error)));
    search_->setEnabled(true);
}
}
