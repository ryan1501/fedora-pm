#include "gui/main_window.hpp"

#include <QHBoxLayout>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QProcess>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

namespace fedora_pm::gui {
MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), process_(new QProcess(this)) {
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
    connect(process_, &QProcess::readyReadStandardOutput, this, [this] {
        output_->appendPlainText(QString::fromLocal8Bit(process_->readAllStandardOutput()));
    });
    connect(process_, &QProcess::readyReadStandardError, this, [this] {
        output_->appendPlainText(QString::fromLocal8Bit(process_->readAllStandardError()));
    });
    connect(process_, &QProcess::finished, this, [this](int exitCode, QProcess::ExitStatus status) {
        if (status != QProcess::NormalExit || exitCode != 0) {
            output_->appendPlainText(QString("\nDNF exited with code %1.").arg(exitCode));
        }
        search_->setEnabled(true);
        query_->setEnabled(true);
    });
    connect(process_, &QProcess::errorOccurred, this, [this](QProcess::ProcessError error) {
        if (error == QProcess::FailedToStart) {
            output_->appendPlainText("Could not start dnf. Ensure DNF is installed and available in PATH.");
            search_->setEnabled(true);
            query_->setEnabled(true);
        }
    });
}

void MainWindow::run_search() {
    const QString query = query_->text().trimmed();
    if (query.isEmpty() || process_->state() != QProcess::NotRunning) {
        return;
    }
    output_->clear();
    output_->appendPlainText("Searching with DNF…");
    search_->setEnabled(false);
    query_->setEnabled(false);
    process_->start("dnf", {"search", query});
}
}
