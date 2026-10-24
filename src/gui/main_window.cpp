#include "gui/main_window.hpp"

#include <QHBoxLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QProcess>
#include <QProgressBar>
#include <QPushButton>
#include <QTimer>
#include <QRegularExpression>
#include <QVBoxLayout>
#include <QWidget>

namespace fedora_pm::gui {
MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), process_(new QProcess(this)) {
    setWindowTitle("Fedora Package Manager");
    setMinimumSize(760, 520);
    resize(900, 620);

    auto* central = new QWidget(this);
    auto* layout = new QVBoxLayout(central);
    auto* searchRow = new QHBoxLayout;
    query_ = new QLineEdit(central);
    query_->setPlaceholderText("Package name or search terms…");
    search_ = new QPushButton("Search", central);
    searchRow->addWidget(query_, 1);
    searchRow->addWidget(search_);
    layout->addLayout(searchRow);

    auto* actions = new QHBoxLayout;
    info_ = new QPushButton("Package info", central);
    installed_ = new QPushButton("Installed", central);
    available_ = new QPushButton("Available", central);
    install_ = new QPushButton("Install…", central);
    remove_ = new QPushButton("Remove…", central);
    update_ = new QPushButton("Update all…", central);
    for (auto* button : {info_, installed_, available_, install_, remove_, update_})
        actions->addWidget(button);
    layout->addLayout(actions);

    output_ = new QPlainTextEdit(central);
    output_->setReadOnly(true);
    output_->setPlaceholderText("Package results and transaction output appear here.");
    output_->setMaximumBlockCount(5000);
    layout->addWidget(output_, 1);

    auto* statusRow = new QHBoxLayout;
    progress_ = new QProgressBar(central);
    progress_->setTextVisible(false);
    progress_->setRange(0, 1);
    progress_->setValue(0);
    cancel_ = new QPushButton("Cancel", central);
    cancel_->setEnabled(false);
    statusRow->addWidget(progress_, 1);
    statusRow->addWidget(cancel_);
    layout->addLayout(statusRow);
    setCentralWidget(central);

    connect(search_, &QPushButton::clicked, this, &MainWindow::run_search);
    connect(query_, &QLineEdit::returnPressed, this, &MainWindow::run_search);
    connect(info_, &QPushButton::clicked, this, [this] {
        const auto package = query_->text().trimmed();
        if (!valid_package_input(package)) {
            QMessageBox::warning(this, "Invalid package", "Enter one valid package name first.");
            return;
        }
        run_dnf({"info", package});
    });
    connect(installed_, &QPushButton::clicked, this, [this] {
        const auto package = query_->text().trimmed();
        if (package.isEmpty()) run_dnf({"list", "--installed"});
        else if (valid_package_input(package)) run_dnf({"list", "--installed", package});
        else QMessageBox::warning(this, "Invalid pattern", "Use a package name or a simple package pattern.");
    });
    connect(available_, &QPushButton::clicked, this, [this] {
        const auto package = query_->text().trimmed();
        if (package.isEmpty()) run_dnf({"list", "--available"});
        else if (valid_package_input(package)) run_dnf({"list", "--available", package});
        else QMessageBox::warning(this, "Invalid pattern", "Use a package name or a simple package pattern.");
    });
    connect(install_, &QPushButton::clicked, this, [this] {
        const auto package = query_->text().trimmed();
        if (!valid_package_input(package)) {
            QMessageBox::warning(this, "Invalid package", "Enter one valid package name to install.");
            return;
        }
        if (QMessageBox::question(this, "Confirm installation",
            "Install package '" + package + "'? DNF may also install dependencies.",
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes)
            run_dnf({"install", "-y", package}, true);
    });
    connect(remove_, &QPushButton::clicked, this, [this] {
        const auto package = query_->text().trimmed();
        if (!valid_package_input(package)) {
            QMessageBox::warning(this, "Invalid package", "Enter one valid package name to remove.");
            return;
        }
        if (QMessageBox::warning(this, "Confirm removal",
            "Remove package '" + package + "'? Review DNF's proposed transaction carefully.",
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes)
            run_dnf({"remove", "-y", package}, true);
    });
    connect(update_, &QPushButton::clicked, this, [this] {
        if (QMessageBox::warning(this, "Confirm system update",
            "Check and apply available package updates? Review DNF's proposed transaction before accepting it.",
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes)
            run_dnf({"upgrade", "-y"}, true);
    });
    connect(cancel_, &QPushButton::clicked, this, [this] {
        if (process_->state() != QProcess::NotRunning) {
            output_->appendPlainText("\nCancellation requested…");
            process_->terminate();
            QTimer::singleShot(2500, process_, [this] {
                if (process_->state() != QProcess::NotRunning) process_->kill();
            });
        }
    });

    connect(process_, &QProcess::readyReadStandardOutput, this, [this] {
        const auto text = QString::fromLocal8Bit(process_->readAllStandardOutput());
        if (!text.isEmpty()) output_->insertPlainText(text);
    });
    connect(process_, &QProcess::readyReadStandardError, this, [this] {
        const auto text = QString::fromLocal8Bit(process_->readAllStandardError());
        if (!text.isEmpty()) output_->insertPlainText(text);
    });
    connect(process_, &QProcess::finished, this, [this](int exitCode, QProcess::ExitStatus status) {
        output_->appendPlainText(QString("\nProcess finished with exit code %1.").arg(exitCode));
        if (status != QProcess::NormalExit)
            output_->appendPlainText("The command did not exit normally.");
        set_busy(false);
    });
    connect(process_, &QProcess::errorOccurred, this, [this](QProcess::ProcessError error) {
        if (error == QProcess::FailedToStart) {
            output_->appendPlainText("Could not start the command. Check that dnf and, for privileged actions, pkexec are installed.");
            set_busy(false);
        }
    });
}

bool MainWindow::valid_package_input(const QString& value) const {
    static const QRegularExpression packagePattern(
        "^[A-Za-z0-9][A-Za-z0-9._+:-]{0,199}$");
    return packagePattern.match(value).hasMatch();
}

void MainWindow::set_busy(bool busy) {
    for (auto* button : {search_, info_, installed_, available_, install_, remove_, update_})
        button->setEnabled(!busy);
    query_->setEnabled(!busy);
    cancel_->setEnabled(busy);
    if (busy) progress_->setRange(0, 0);
    else {
        progress_->setRange(0, 1);
        progress_->setValue(0);
    }
}

void MainWindow::run_search() {
    const QString query = query_->text().trimmed();
    if (query.isEmpty()) {
        QMessageBox::information(this, "Search packages", "Enter a package name or search terms.");
        return;
    }
    run_dnf({"search", query});
}

void MainWindow::run_dnf(QStringList args, bool mutating) {
    if (process_->state() != QProcess::NotRunning) return;
    output_->clear();
    output_->appendPlainText((mutating ? "Requesting administrator authorization…\n" : "Running dnf…\n"));
    set_busy(true);
    if (mutating) {
        QStringList privilegedArgs{"dnf"};
        privilegedArgs.append(args);
        process_->start("pkexec", privilegedArgs);
    } else {
        process_->start("dnf", args);
    }
}
}
