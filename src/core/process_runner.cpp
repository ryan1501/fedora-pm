#include "core/process_runner.hpp"

#include <cerrno>
#include <csignal>
#include <cstring>
#include <fcntl.h>
#include <poll.h>
#include <stdexcept>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

namespace fedora_pm::core {
namespace {
void close_fd(int& fd) {
    if (fd >= 0) {
        ::close(fd);
        fd = -1;
    }
}

void close_pipes(int out_pipe[2], int err_pipe[2]) {
    close_fd(out_pipe[0]);
    close_fd(out_pipe[1]);
    close_fd(err_pipe[0]);
    close_fd(err_pipe[1]);
}

void set_nonblocking(int fd) {
    const int flags = ::fcntl(fd, F_GETFL, 0);
    if (flags < 0 || ::fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0)
        throw std::runtime_error("fcntl failed: " + std::string(std::strerror(errno)));
}

void drain_fd(int& fd, std::string& target, bool& open) {
    char buffer[4096];
    for (;;) {
        const auto n = ::read(fd, buffer, sizeof(buffer));
        if (n > 0) {
            target.append(buffer, static_cast<std::size_t>(n));
        } else if (n == 0) {
            close_fd(fd);
            open = false;
            return;
        } else if (errno == EINTR) {
            continue;
        } else if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return;
        } else {
            close_fd(fd);
            open = false;
            return;
        }
    }
}

void terminate_child(pid_t pid, int& status, bool& child_done) {
    if (child_done) return;
    (void)::kill(pid, SIGTERM);
    const auto grace = std::chrono::steady_clock::now() + std::chrono::milliseconds(250);
    while (std::chrono::steady_clock::now() < grace) {
        const pid_t waited = ::waitpid(pid, &status, WNOHANG);
        if (waited == pid || (waited < 0 && errno == ECHILD)) {
            child_done = true;
            return;
        }
        if (waited < 0 && errno != EINTR) break;
        (void)::usleep(10000);
    }
    (void)::kill(pid, SIGKILL);
    while (::waitpid(pid, &status, 0) < 0) {
        if (errno == EINTR) continue;
        if (errno == ECHILD) child_done = true;
        break;
    }
    child_done = true;
}
}

ProcessResult run_process(const std::string& program, const std::vector<std::string>& arguments,
                          const ProcessOptions& options) {
    const bool elevate = options.use_sudo && ::geteuid() != 0;
    const std::string executable = elevate ? "sudo" : program;
    std::vector<std::string> effective_arguments;
    if (elevate) {
        effective_arguments.reserve(arguments.size() + 2);
        effective_arguments.emplace_back("--non-interactive");
        effective_arguments.push_back(program);
        effective_arguments.insert(effective_arguments.end(), arguments.begin(), arguments.end());
    } else {
        effective_arguments = arguments;
    }
    int out_pipe[2]{-1, -1};
    int err_pipe[2]{-1, -1};
    if (::pipe(out_pipe) != 0)
        throw std::runtime_error("stdout pipe failed: " + std::string(std::strerror(errno)));
    if (::pipe(err_pipe) != 0) {
        const int saved_errno = errno;
        close_pipes(out_pipe, err_pipe);
        throw std::runtime_error("stderr pipe failed: " + std::string(std::strerror(saved_errno)));
    }

    const pid_t pid = ::fork();
    if (pid < 0) {
        const int saved_errno = errno;
        close_pipes(out_pipe, err_pipe);
        throw std::runtime_error("fork failed: " + std::string(std::strerror(saved_errno)));
    }
    if (pid == 0) {
        if (::dup2(out_pipe[1], STDOUT_FILENO) < 0 ||
            ::dup2(err_pipe[1], STDERR_FILENO) < 0) {
            const char message[] = "failed to redirect child process output\n";
            const auto written = ::write(STDERR_FILENO, message, sizeof(message) - 1);
            (void)written;
            _exit(126);
        }
        close_pipes(out_pipe, err_pipe);
        std::vector<char*> argv;
        argv.reserve(effective_arguments.size() + 2);
        argv.push_back(const_cast<char*>(executable.c_str()));
        for (const auto& arg : effective_arguments) argv.push_back(const_cast<char*>(arg.c_str()));
        argv.push_back(nullptr);
        ::execvp(executable.c_str(), argv.data());
        const std::string message = "exec failed: " + std::string(std::strerror(errno)) + "\n";
        const auto written = ::write(STDERR_FILENO, message.data(), message.size());
        (void)written;
        _exit(127);
    }

    close_fd(out_pipe[1]);
    close_fd(err_pipe[1]);
    try {
        set_nonblocking(out_pipe[0]);
        set_nonblocking(err_pipe[0]);
    } catch (...) {
        (void)::kill(pid, SIGKILL);
        int ignored_status = 0;
        while (::waitpid(pid, &ignored_status, 0) < 0 && errno == EINTR) {}
        close_pipes(out_pipe, err_pipe);
        throw;
    }

    ProcessResult result;
    bool out_open = true, err_open = true, child_done = false;
    int status = 0;
    const auto deadline = std::chrono::steady_clock::now() + options.timeout;
    while (out_open || err_open || !child_done) {
        if (!child_done) {
            const pid_t waited = ::waitpid(pid, &status, WNOHANG);
            if (waited == pid || (waited < 0 && errno == ECHILD)) {
                child_done = true;
            } else if (waited < 0 && errno != EINTR) {
                const int saved_errno = errno;
                terminate_child(pid, status, child_done);
                close_pipes(out_pipe, err_pipe);
                throw std::runtime_error("waitpid failed: " + std::string(std::strerror(saved_errno)));
            }
        }

        // Bound the entire operation, including pipes inherited by descendants
        // after the direct child has exited.
        if (std::chrono::steady_clock::now() >= deadline &&
            (!child_done || out_open || err_open)) {
            result.timed_out = true;
            terminate_child(pid, status, child_done);
            close_pipes(out_pipe, err_pipe);
            out_open = err_open = false;
            break;
        }

        pollfd fds[2]{{out_pipe[0], static_cast<short>(POLLIN | POLLHUP), 0},
                      {err_pipe[0], static_cast<short>(POLLIN | POLLHUP), 0}};
        (void)::poll(fds, 2, 20);
        if (out_open && (fds[0].revents || child_done)) drain_fd(out_pipe[0], result.standard_output, out_open);
        if (err_open && (fds[1].revents || child_done)) drain_fd(err_pipe[0], result.standard_error, err_open);
        if (child_done && !out_open && !err_open) break;
    }

    if (WIFEXITED(status)) result.exit_code = WEXITSTATUS(status);
    else if (WIFSIGNALED(status)) result.exit_code = 128 + WTERMSIG(status);
    if (options.require_success && (result.timed_out || result.exit_code != 0))
        throw std::runtime_error("process failed: " + program + (result.timed_out ? " (timed out)" : ""));
    return result;
}
}
