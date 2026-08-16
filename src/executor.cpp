#include "hackforge/executor.hpp"
#include <unistd.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <fcntl.h>
#include <signal.h>
#include <chrono>
#include <thread>
#include <cstring>
#include <cstdio>
#include <cerrno>

namespace hackforge {

ExecutionResult Executor::run(const std::string& target_path, const std::string& input, double timeout_ms) {
    ExecutionResult result;
    result.launched = false;
    result.timed_out = false;
    result.crashed = false;
    result.verdict = "OK"; 

    // Avoid Pipe Deadlocks: Write input to a temporary file instead of an unbounded pipe buffer.
    // The OS automatically cleans up tmpfile() when all file descriptors point to it are closed.
    FILE* tmp_in = tmpfile();
    if (!tmp_in) {
        result.error = "Failed to create temp file for stdin";
        return result;
    }
    if (!input.empty()) {
        fwrite(input.data(), 1, input.size(), tmp_in);
        fflush(tmp_in);
        rewind(tmp_in);
    }

    // Pipe used to cleanly catch and bubble up an exec() failure to the parent.
    int exec_pipe[2];
    if (pipe(exec_pipe) != 0) {
        result.error = "Failed to create exec pipe";
        fclose(tmp_in);
        return result;
    }
    
    // Automatically close the pipe in the child when exec succeeds
    fcntl(exec_pipe[1], F_SETFD, FD_CLOEXEC);

    auto start_time = std::chrono::steady_clock::now();

    pid_t pid = fork();
    if (pid < 0) {
        result.error = "fork() failed: " + std::string(strerror(errno));
        close(exec_pipe[0]);
        close(exec_pipe[1]);
        fclose(tmp_in);
        return result;
    }

    if (pid == 0) {
        // --- CHILD PROCESS ---
        close(exec_pipe[0]);

        // Route tmp file to stdin
        dup2(fileno(tmp_in), STDIN_FILENO);

        // Discard stdout/stderr to /dev/null
        int dev_null = open("/dev/null", O_WRONLY);
        if (dev_null != -1) {
            dup2(dev_null, STDOUT_FILENO);
            dup2(dev_null, STDERR_FILENO);
            close(dev_null);
        }

        execl(target_path.c_str(), target_path.c_str(), nullptr);

        // Reaches here ONLY if execl fails. Write errno to parent and exit.
        int err = errno;
        if (write(exec_pipe[1], &err, sizeof(err)) < 0) { /* suppress warn */ }
        _exit(127);
    }

    // --- PARENT PROCESS ---
    close(exec_pipe[1]); 

    // Read the error pipe to see if exec() failed
    int err = 0;
    if (read(exec_pipe[0], &err, sizeof(err)) == sizeof(err)) {
        result.launched = false;
        result.error = "exec() failed: " + std::string(strerror(err));
        waitpid(pid, nullptr, 0); // Reap failed child
        close(exec_pipe[0]);
        fclose(tmp_in);
        return result;
    }
    close(exec_pipe[0]);

    result.launched = true;

    int status = 0;
    struct rusage ru;
    pid_t wpid = 0;
    bool done = false;

    // Timeout loop checking wait4() non-blocking
    while (!done) {
        wpid = wait4(pid, &status, WNOHANG, &ru);
        if (wpid > 0) {
            done = true;
        } else if (wpid == -1) {
            if (errno != EINTR) done = true;
        } else {
            auto now = std::chrono::steady_clock::now();
            auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time).count();
            
            if (elapsed_ms >= timeout_ms) {
                result.timed_out = true;
                result.verdict = "TLE";
                kill(pid, SIGKILL);
                wait4(pid, &status, 0, &ru); // Ensure child is reaped completely
                done = true;
            } else {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        }
    }

    auto end_time = std::chrono::steady_clock::now();
    result.wall_seconds = std::chrono::duration<double>(end_time - start_time).count();

    // Sum User and System time
    result.cpu_seconds = (ru.ru_utime.tv_sec + ru.ru_utime.tv_usec / 1e6) +
                         (ru.ru_stime.tv_sec + ru.ru_stime.tv_usec / 1e6);

#ifdef __APPLE__
    result.max_rss_kb = ru.ru_maxrss / 1024; // macOS rusage returns bytes
#else
    result.max_rss_kb = ru.ru_maxrss;        // Linux rusage returns kilobytes
#endif

    fclose(tmp_in);

    if (result.timed_out) {
        return result;
    }

    if (WIFEXITED(status)) {
        result.exit_code = WEXITSTATUS(status);
        if (result.exit_code != 0) {
            result.crashed = true;
            result.verdict = "CRASH";
        }
    } else if (WIFSIGNALED(status)) {
        result.crashed = true;
        result.signal = WTERMSIG(status);
        result.verdict = "CRASH";
    }

    return result;
}

} // namespace hackforge