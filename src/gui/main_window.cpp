#include "gui/main_window.hpp"
#include "system/history_manager.hpp"

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

#include <string>
#include <vector>

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

    auto* systemActions = new QHBoxLayout;
    history_ = new QPushButton("DNF history", central);
    repositories_ = new QPushButton("Repositories", central);
    securityUpdates_ = new QPushButton("Security updates", central);
    systemActions->addWidget(history_);
    systemActions->addWidget(repositories_);
    systemActions->addWidget(securityUpdates_);
    systemActions->addStretch(1);
    layout->addLayout(systemActions);

    auto* flatpakActions = new QHBoxLayout;
    flatpakSearch_ = new QPushButton("Flatpak search", central);
    flatpakInstalled_ = new QPushButton("Installed Flatpaks", central);
    flatpakInfo_ = new QPushButton("Flatpak info", central);
    flatpakInstall_ = new QPushButton("Install Flatpak…", central);
    flatpakRemove_ = new QPushButton("Remove Flatpak…", central);
    flatpakUpdate_ = new QPushButton("Update Flatpaks…", central);
    flatpakActions->addWidget(flatpakSearch_);
    flatpakActions->addWidget(flatpakInstalled_);
    flatpakActions->addWidget(flatpakInfo_);
    flatpakActions->addWidget(flatpakInstall_);
    flatpakActions->addWidget(flatpakRemove_);
    flatpakActions->addWidget(flatpakUpdate_);
    flatpakActions->addStretch(1);
    layout->addLayout(flatpakActions);

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
        run_dnf({"install", package}, true);
    });
    connect(remove_, &QPushButton::clicked, this, [this] {
        const auto package = query_->text().trimmed();
        if (!valid_package_input(package)) {
            QMessageBox::warning(this, "Invalid package", "Enter one valid package name to remove.");
            return;
        }
        run_dnf({"remove", package}, true);
    });
    connect(update_, &QPushButton::clicked, this, [this] {
        run_dnf({"upgrade"}, true);
    });
    connect(history_, &QPushButton::clicked, this, [this] {
        run_dnf({"history", "list"});
    });
    connect(repositories_, &QPushButton::clicked, this, [this] {
        run_dnf({"repolist", "--all"});
    });
    connect(securityUpdates_, &QPushButton::clicked, this, [this] {
        run_dnf({"updateinfo", "list", "--security"});
    });
    connect(flatpakSearch_, &QPushButton::clicked, this, [this] {
        const auto query = query_->text().trimmed();
        if (query.isEmpty()) {
            QMessageBox::information(this, "Flatpak search", "Enter search terms first.");
            return;
        }
        run_flatpak({"search", query});
    });
    connect(flatpakInstalled_, &QPushButton::clicked, this, [this] {
        run_flatpak({"list", "--app"});
    });
    connect(flatpakInfo_, &QPushButton::clicked, this, [this] {
        const auto appId = query_->text().trimmed();
        if (!valid_flatpak_app_id(appId)) {
            QMessageBox::warning(this, "Invalid Flatpak ID", "Enter a dotted Flatpak application ID, such as org.example.App.");
            return;
        }
        run_flatpak({"info", appId});
    });
    connect(flatpakInstall_, &QPushButton::clicked, this, [this] {
        const auto appId = query_->text().trimmed();
        if (!valid_flatpak_app_id(appId)) {
            QMessageBox::warning(this, "Invalid Flatpak ID", "Enter a dotted Flatpak application ID, such as org.example.App.");
            return;
        }
        if (QMessageBox::question(this, "Confirm Flatpak installation",
                                  "Install Flatpak application " + appId + "?") == QMessageBox::Yes)
            run_flatpak({"install", "--assumeyes", appId});
    });
    connect(flatpakRemove_, &QPushButton::clicked, this, [this] {
        const auto appId = query_->text().trimmed();
        if (!valid_flatpak_app_id(appId)) {
            QMessageBox::warning(this, "Invalid Flatpak ID", "Enter a dotted Flatpak application ID, such as org.example.App.");
            return;
        }
        if (QMessageBox::question(this, "Confirm Flatpak removal",
                                  "Remove Flatpak application " + appId + "?") == QMessageBox::Yes)
            run_flatpak({"uninstall", "--assumeyes", appId});
    });
    connect(flatpakUpdate_, &QPushButton::clicked, this, [this] {
        if (QMessageBox::question(this, "Confirm Flatpak updates",
                                  "Apply all available Flatpak updates?") == QMessageBox::Yes)
            run_flatpak({"update", "--assumeyes"});
    });
    connect(cancel_, &QPushButton::clicked, this, [this] {
        if (process_->state() != QProcess::NotRunning) {
            cancellationRequested_ = true;
            output_->appendPlainText("\nCancellation requested…");
            process_->terminate();
            QTimer::singleShot(2500, process_, [this] {
                if (process_->state() != QProcess::NotRunning) process_->kill();
            });
        }
    });

    connect(process_, &QProcess::readyReadStandardOutput, this, [this] {
        const auto text = QString::fromLocal8Bit(process_->readAllStandardOutput());
        if (!text.isEmpty()) {
            processOutput_ += text;
            output_->insertPlainText(text);
        }
    });
    connect(process_, &QProcess::readyReadStandardError, this, [this] {
        const auto text = QString::fromLocal8Bit(process_->readAllStandardError());
        if (!text.isEmpty()) {
            processOutput_ += text;
            output_->insertPlainText(text);
        }
    });
    connect(process_, &QProcess::finished, this, [this](int exitCode, QProcess::ExitStatus status) {
        if (previewingMutation_) {
            previewingMutation_ = false;
            if (cancellationRequested_) {
                output_->appendPlainText("\nTransaction preview cancelled; no changes were applied.");
                pendingMutationArgs_.clear();
                set_busy(false);
                return;
            }
            const bool hasTransaction = processOutput_.contains("Transaction Summary", Qt::CaseInsensitive)
                || processOutput_.contains("Dependencies resolved", Qt::CaseInsensitive)
                || processOutput_.contains("Nothing to do", Qt::CaseInsensitive);
            if (status == QProcess::NormalExit && hasTransaction) {
                QMessageBox confirm(this);
                confirm.setIcon(QMessageBox::Warning);
                confirm.setWindowTitle("Review DNF transaction");
                confirm.setText("Review the proposed package changes before applying them.");
                confirm.setInformativeText(processOutput_.left(1800));
                confirm.setDetailedText(processOutput_);
                auto* applyButton = confirm.addButton("Apply transaction", QMessageBox::AcceptRole);
                confirm.addButton("Cancel", QMessageBox::RejectRole);
                confirm.exec();
                if (confirm.clickedButton() == applyButton) {
                    QStringList privilegedArgs{"dnf"};
                    privilegedArgs.append(pendingMutationArgs_);
                    privilegedArgs.append("-y");
                    processOutput_.clear();
                    output_->appendPlainText("\nApplying confirmed transaction…\n");
                    previewingMutation_ = false;
                    applyingMutation_ = true;
                    process_->start("pkexec", privilegedArgs);
                    return;
                }
                output_->appendPlainText("\nTransaction cancelled; no package changes were requested.");
            } else {
                output_->appendPlainText(QString("\nCould not verify a DNF transaction preview (exit code %1). No changes were applied.").arg(exitCode));
                if (!processOutput_.isEmpty())
                    output_->appendPlainText(processOutput_);
            }
            pendingMutationArgs_.clear();
            set_busy(false);
            return;
        }
        if (applyingMutation_) {
            if (status == QProcess::NormalExit && exitCode == 0) {
                QString action = pendingMutationArgs_.value(0);
                const bool flatpakAction = action.startsWith("flatpak-");
                if (flatpakAction) {
                    const QString operation = action.mid(QString("flatpak-").size());
                    if (operation == "install") action = "flatpak_install";
                    else if (operation == "uninstall") action = "flatpak_remove";
                    else if (operation == "update") action = "flatpak_update";
                } else if (action == "upgrade") {
                    action = "update";
                }
                std::vector<std::string> items;
                for (qsizetype i = 1; i < pendingMutationArgs_.size(); ++i) {
                    const auto item = pendingMutationArgs_.at(i);
                    if (!item.startsWith("--")) items.push_back(item.toStdString());
                }
                if (items.empty())
                    items.push_back(action == "update" || action == "flatpak_update"
                        ? "system" : flatpakAction ? "flatpak" : "dnf");
                const auto logged = fedora_pm::system::HistoryManager{}.log(action.toStdString(), items);
                if (!logged.success)
                    output_->appendPlainText(QString("\nWarning: transaction succeeded but local history logging failed: %1").arg(QString::fromStdString(logged.error)));
            }
            applyingMutation_ = false;
            pendingMutationArgs_.clear();
        }
        output_->appendPlainText(QString("\nProcess finished with exit code %1.").arg(exitCode));
        if (status != QProcess::NormalExit)
            output_->appendPlainText("The command did not exit normally.");
        set_busy(false);
    });
    connect(process_, &QProcess::errorOccurred, this, [this](QProcess::ProcessError error) {
        if (error == QProcess::FailedToStart) {
            output_->appendPlainText("Could not start the requested command. Check that the required executable is installed and available in PATH.");
            applyingMutation_ = false;
            pendingMutationArgs_.clear();
            set_busy(false);
        }
    });
}

bool MainWindow::valid_package_input(const QString& value) const {
    static const QRegularExpression packagePattern(
        "^[A-Za-z0-9][A-Za-z0-9._+:-]{0,199}$");
    return packagePattern.match(value).hasMatch();
}

bool MainWindow::valid_flatpak_app_id(const QString& value) const {
    static const QRegularExpression appIdPattern(
        "^[A-Za-z][A-Za-z0-9_-]*(\\.[A-Za-z][A-Za-z0-9_-]*)+$");
    return appIdPattern.match(value).hasMatch();
}

void MainWindow::set_busy(bool busy) {
    for (auto* button : {search_, info_, installed_, available_, install_, remove_, update_, history_, repositories_, securityUpdates_, flatpakSearch_, flatpakInstalled_, flatpakInfo_, flatpakInstall_, flatpakRemove_, flatpakUpdate_})
        button->setEnabled(!busy);
    query_->setEnabled(!busy);
    cancel_->setEnabled(busy);
    if (busy) progress_->setRange(0, 0);
    else {
        progress_->setRange(0, 1);
        progress_->setValue(0);
    }
}

void MainWindow::run_flatpak(QStringList args) {
    if (process_->state() != QProcess::NotRunning) return;
    output_->clear();
    processOutput_.clear();
    pendingMutationArgs_.clear();
    previewingMutation_ = false;
    cancellationRequested_ = false;
    const QString operation = args.value(0);
    applyingMutation_ = operation == "install" || operation == "uninstall" || operation == "update";
    if (applyingMutation_) {
        pendingMutationArgs_ = args;
        pendingMutationArgs_[0] = "flatpak-" + operation;
    }
    output_->appendPlainText("Running flatpak…\\n");
    set_busy(true);
    process_->start("flatpak", args);
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
    processOutput_.clear();
    pendingMutationArgs_.clear();
    applyingMutation_ = false;
    previewingMutation_ = mutating;
    cancellationRequested_ = false;
    output_->appendPlainText((mutating ? "Resolving proposed DNF transaction (no changes will be made)…\n" : "Running dnf…\n"));
    set_busy(true);
    if (mutating) {
        pendingMutationArgs_ = args;
        QStringList privilegedArgs{"dnf"};
        privilegedArgs.append(args);
        privilegedArgs.append("--assumeno");
        process_->start("pkexec", privilegedArgs);
    } else {
        process_->start("dnf", args);
    }
}
}
