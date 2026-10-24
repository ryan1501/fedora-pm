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
void set_nonblocking(int fd) {
    const int flags = ::fcntl(fd, F_GETFL, 0);
    if (flags >= 0) ::fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}
void drain_fd(int fd, std::string& target, bool& open) {
    char buffer[4096];
    for (;;) {
        const auto n = ::read(fd, buffer, sizeof(buffer));
        if (n > 0) target.append(buffer, static_cast<std::size_t>(n));
        else if (n == 0) { ::close(fd); open = false; return; }
        else if (errno == EINTR) continue;
        else if (errno == EAGAIN || errno == EWOULDBLOCK) return;
        else { ::close(fd); open = false; return; }
    }
}
}

ProcessResult run_process(const std::string& program, const std::vector<std::string>& arguments,
                          const ProcessOptions& options) {
    int out_pipe[2], err_pipe[2];
    if (::pipe(out_pipe) != 0 || ::pipe(err_pipe) != 0)
        throw std::runtime_error("pipe failed: " + std::string(std::strerror(errno)));

    const pid_t pid = ::fork();
    if (pid < 0) {
        ::close(out_pipe[0]); ::close(out_pipe[1]); ::close(err_pipe[0]); ::close(err_pipe[1]);
        throw std::runtime_error("fork failed: " + std::string(std::strerror(errno)));
    }
    if (pid == 0) {
        ::dup2(out_pipe[1], STDOUT_FILENO);
        ::dup2(err_pipe[1], STDERR_FILENO);
        ::close(out_pipe[0]); ::close(out_pipe[1]); ::close(err_pipe[0]); ::close(err_pipe[1]);
        std::vector<char*> argv;
        argv.reserve(arguments.size() + 2);
        argv.push_back(const_cast<char*>(program.c_str()));
        for (const auto& arg : arguments) argv.push_back(const_cast<char*>(arg.c_str()));
        argv.push_back(nullptr);
        ::execvp(program.c_str(), argv.data());
        const std::string message = "exec failed: " + std::string(std::strerror(errno)) + "\n";
        (void)::write(STDERR_FILENO, message.data(), message.size());
        _exit(127);
    }
    ::close(out_pipe[1]); ::close(err_pipe[1]);
    set_nonblocking(out_pipe[0]); set_nonblocking(err_pipe[0]);
    ProcessResult result;
    bool out_open = true, err_open = true, child_done = false;
    int status = 0;
    const auto deadline = std::chrono::steady_clock::now() + options.timeout;
    while (out_open || err_open || !child_done) {
        if (!child_done) {
            const pid_t waited = ::waitpid(pid, &status, WNOHANG);
            if (waited == pid) child_done = true;
        }
        if (!child_done && std::chrono::steady_clock::now() >= deadline) {
            result.timed_out = true;
            ::kill(pid, SIGTERM);
            const auto grace = std::chrono::steady_clock::now() + std::chrono::milliseconds(250);
            while (::waitpid(pid, &status, WNOHANG) == 0 && std::chrono::steady_clock::now() < grace)
                ::usleep(10000);
            if (::waitpid(pid, &status, WNOHANG) == 0) { ::kill(pid, SIGKILL); ::waitpid(pid, &status, 0); }
            child_done = true;
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
