#pragma once

#include <QMainWindow>
#include <QStringList>

class QLineEdit;
class QPlainTextEdit;
class QPushButton;
class QProcess;
class QProgressBar;

namespace fedora_pm::gui {
class MainWindow final : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
private:
    QLineEdit* query_;
    QPlainTextEdit* output_;
    QPushButton* search_;
    QPushButton* info_;
    QPushButton* installed_;
    QPushButton* available_;
    QPushButton* install_;
    QPushButton* remove_;
    QPushButton* update_;
    QPushButton* flatpakSearch_;
    QPushButton* flatpakInstalled_;
    QPushButton* flatpakInfo_;
    QPushButton* cancel_;
    QProgressBar* progress_;
    QProcess* process_;
    QStringList pendingMutationArgs_;
    QString processOutput_;
    bool previewingMutation_{false};
    bool cancellationRequested_{false};
    void run_search();
    void run_dnf(QStringList args, bool mutating = false);
    void run_flatpak(QStringList args);
    bool valid_package_input(const QString& value) const;
    void set_busy(bool busy);
};
}
