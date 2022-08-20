/*
 * Copyright (C) 2013 Cloudius Systems, Ltd.
 *
 * This work is open source software, licensed under the terms of the
 * BSD license as described in the LICENSE file in the top-level directory.
 */

#include <sys/socket.h>
#include <sys/poll.h>
#include <sys/uio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>

#include <string>
#include <thread>
#include <iostream>
#include <vector>

static int tests = 0, fails = 0;

static void report(bool ok, std::string msg)
{
    ++tests;
    fails += !ok;
    std::cout << (ok ? "PASS" : "FAIL") << ": " << msg << "\n";
}

int main(int ac, char** av)
{
    // OSv doesn't support SIGPIPE generation, but Linux does, and we want
    // to disable it to check pipe errors where they occur, instead of through
    // a signal handler.
    signal(SIGPIPE, SIG_IGN);

    int s[2];

    int r = pipe(s);
    report(r == 0, "pipe call");


    char msg[] = "hello", reply[] = "wrong";
    r = write(s[1], msg, 5);
    report(r == 5, "write to empty socket");
    r = read(s[0], reply, 5);
    report(r == 5 && memcmp(msg, reply, 5) == 0, "read after write");

    r = write(s[0], msg, 5);
    report(r == -1 && errno == EBADF, "write to read end should fail");
    r = read(s[1], reply, 5);
    report(r == -1 && errno == EBADF, "read to write end should fail");

    memcpy(msg, "snafu", 5);
    memset(reply, 0, 5);
    int r2;
    std::thread t1([&] {
        r2 = read(s[0], reply, 5);
        report(r2 == 5 && memcmp(msg, reply, 5) == 0, "read before write");
    });
    sleep(1);
    r = write(s[1], msg, 5);
    t1.join();


    memcpy(msg, "fooba", 5);
    memset(reply, 0, 5);
    pollfd poller = { s[0], POLLIN, 0 };
    r = poll(&poller, 1, 0);
    report(r == 0, "poll() (immediate, no events)");
    r = poll(&poller, 1, 1);
    report(r == 0, "poll() (timeout, no events)");
    r2 = write(s[1], msg, 5);
    r = poll(&poller, 1, 0);
    report(r2 == 5 && r == 1 && poller.revents == POLLIN, "poll() (immediate, has event)");
    r2 = read(s[0], reply, 5);
    report(r2 == 5 && memcmp(msg, reply, 5) == 0, "read after poll");


    memcpy(msg, "smeg!", 5);
    memset(reply, 0, 5);
    std::thread t2([&] {
        poller.revents = 0;
        r2 = poll(&poller, 1, 5000);
        report(r2 == 1 && poller.revents == POLLIN, "waiting poll");
        r2 = read(s[0], reply, 5);
        report(r2 == 5 && memcmp(msg, reply, 5) == 0, "read after waiting poll");
    });
    sleep(1);
    r = write(s[1], msg, 5);
    t2.join();
    report(r == 5, "write to polling socket");

    poller = { s[0], POLLOUT, 0 };
    r = poll(&poller, 1, 0);
    report(r == 0, "poll() (no output on read end)");
    poller = { s[1], POLLIN, 0 };
    r = poll(&poller, 1, 0);
    report(r == 0, "poll() (no input on write end)");


    // test atomic writes.
#ifdef __OSV__
    // Assumes our af_local_buffer size is 8192 bytes -
    // if this changes we need to change this test!
#define PIPE_BUFFER_SIZE 8192
#else
    // The pipe buffer size since Linux 2.6.11 was dramatically increased to 64K.
#define PIPE_BUFFER_SIZE 65536
#endif
#define TSTBUFSIZE PIPE_BUFFER_SIZE*3
    char *buf1 = (char *)calloc(1,TSTBUFSIZE);
    char *buf2 = (char *)calloc(1,TSTBUFSIZE);
    std::thread t3([&] {
        r = write(s[1], buf1, PIPE_BUFFER_SIZE-100);
        report(r == PIPE_BUFFER_SIZE-100, "write size-100 bytes to empty pipe");
        // this write() should block, not partially succeed:
        r = write(s[1], buf1, 400);
        report(r == 400, "write 400 bytes is atomic");
        // this write() should block until the whole buffer is written,
        // even if it takes several waits
        r = write(s[1], buf1, 20000);
        report(r == 20000, "write 20000 bytes until complete");
        // this write() should block until the whole buffer is written,
        // even if it takes several waits

    });
    sleep(1);
    r2 = read(s[0], buf2, PIPE_BUFFER_SIZE-100); // 1 second should be enough for this to be available
    report(r2 == PIPE_BUFFER_SIZE-100, "read size-100 bytes from pipe");
    r2 = read(s[0], buf2, 400);
    report(r2 == 400, "read 400 bytes written atomicly");
    int count=20000;
    while(count>0) {
        r2 = read(s[0], buf2, count);
        report (r2>0, "partial read of the 20000 bytes");
        std::cout << "(read " << r2 << ")\n";
        count -= r2;
    }
    t3.join();
    free(buf1);
    free(buf2);

    // test writev
    struct iovec iov[3];
    char b1[3], b2[2], bn[0];
    char bout[5];
    memcpy(b1, "hel", 3);
    memcpy(b2, "lo", 2);
    iov[0].iov_base = b1;
    iov[0].iov_len = 3;
    iov[1].iov_base = bn;
    iov[1].iov_len = 0;
    iov[2].iov_base = b2;
    iov[2].iov_len = 2;
    r = writev(s[1], iov, sizeof(iov)/sizeof(iov[0]));
    report(r == 5, "writev");
    if (r == 5) {
        r = read(s[0], bout, 5);
        report(r == 5 && memcmp(bout, "hello", 5) == 0, "read after writev");
    }

    // test readv
    b1[0] = b2[0] = '\0';
    r = write(s[1], "mieuw", 5);
    report(r == 5, "write for readv");
    r = readv(s[0], iov, sizeof(iov)/sizeof(iov[0]));
    report(r == 5 && memcmp(b1,"mie",3)==0 && memcmp(b2,"uw",2)==0, "readv");

    // larger, more extensive test, with writev
#define LARGE1 1234567
#define LARGE2 2345678
    buf1 = (char *)calloc(1, LARGE1);
    buf2 = (char *)calloc(1, LARGE2);
    char *buf3 = (char *)calloc(1, LARGE1 + LARGE2);
    iov[0].iov_base = buf1;
    iov[0].iov_len = LARGE1;
    iov[1].iov_base = buf2;
    iov[1].iov_len = LARGE2;
    char c = 0;
    for (int i = 0; i < LARGE1; i++)
        buf1[i] = c++;
    for (int i = 0; i < LARGE2; i++)
        buf2[i] = c++;
    std::thread t4([&] {
        r = writev(s[1], iov, 2);
        report(r == LARGE1+LARGE2, "large writev");;
    });
    c = 0;
    count = LARGE1 + LARGE2;
    while(count > 0) {
        r2 = read(s[0], buf3, count);
        if (r2 <= 0)
            break;
        count -= r2;
        for (int i = 0; i < r2; i++) {
            if (buf3[i] != c++)
                report(false, "correct data read from large writev");
        }
    }
    report (count == 0, "read of large writev");
    t4.join();
    free(buf1);
    free(buf2);
    free(buf3);

    // test closing
    r = close(s[0]);
    report(r == 0, "close read side first");
    r = write(s[1], msg, 4);
    report(r == -1 && errno == EPIPE, "write, other side closed");
    r = close(s[1]);
    report(r == 0, "close also write side");

    r = pipe(s);
    report(r == 0, "pipe call");
    r = close(s[1]);
    report(r == 0, "close write side first");
    r = read(s[0], msg, 5);
    report(r == 0, "read, other side closed");
    r = close(s[0]);
    report(r == 0, "close also read side");

    // test nonblocking
    r = pipe(s);
    report(r == 0, "pipe call");
    r = fcntl(s[0], F_SETFL, O_NONBLOCK);
    report(r == 0, "set read side to nonblocking");
    memcpy(msg, "yoyoy", 5);
    memset(reply, 0, 5);
    r = read(s[0], reply, 5);
    report(r == -1 && errno == EAGAIN, "read from empty nonblocking pipe");
    r = write(s[1], msg, 3);
    report(r == 3, "small write to empty pipe");
    r = read(s[0], reply, 5);
    report(r == 3, "partial read from nonblocking pipe");
    r = fcntl(s[1], F_SETFL, O_NONBLOCK);
    report(r == 0, "set write side to nonblocking");
    r = write(s[1], msg, 5);
    report(r == 5, "small write to empty nonblocking pipe");
    r = write(s[1], msg, 5);
    report(r == 5, "small write to non-empty nonblocking pipe");
    buf1 = (char*) calloc(1, TSTBUFSIZE);
    r = write(s[1], buf1, TSTBUFSIZE);
    report(r < PIPE_BUFFER_SIZE, "partial write to nonblocking pipe");
    auto written = 5 + 5 + r;
    r = write(s[1], buf1, TSTBUFSIZE);
    report(r == -1 && errno == EAGAIN, "write to full nonblocking pipe");
    r = read(s[0], buf1, TSTBUFSIZE);
    report(r == written, "read entire nonblocking pipe");
    free(buf1);
    r = close(s[0]);
    report(r == 0, "close read side");
    r = close(s[1]);
    report(r == 0, "close write side");


    std::vector<int> fds;
    while (pipe(s) == 0) {
        fds.push_back(s[0]);
        fds.push_back(s[1]);
    }
    auto n = fds.size();
    for (int fd : fds) {
        close(fd);
    }
    std::cout << "n=" << n << "\n";
    report(n > 100, "create many pipes");


    // Writing to a pipe, closing the write end, and then read from the pipe.
    // The read would normally poll_wake on the write end of the pipe (in case
    // anybody is waiting to write), but in this case it is closed and should
    // be ignored.
    r = pipe(s);
    report(r == 0, "pipe call");
    r = write(s[1], msg, 5);
    report(r == 5, "write to empty socket");
    r = close(s[1]);
    report(r == 0, "close write side with non-empty buffer");
    r = read(s[0], reply, 5);
    report(r == 5 && memcmp(msg, reply, 5) == 0, "read after write side closed");
    r = close(s[0]);
    report(r == 0, "close also read side");

    // A similar test but have the reply read blocking in a second thread,
    // so it wakes up after the sender was closed - and shouldn't poll_wake
    // the no-longer existing write-side file descriptor.
    r = pipe(s);
    report(r == 0, "pipe call");
    std::thread t5([&] {
        r2 = read(s[0], reply, 5);
        report(r2 == 5 && memcmp(msg, reply, 5) == 0, "blocking read, may wake up with write side closed");
        r2 = close(s[0]);
        report(r2 == 0, "close also read side");
    });
    sleep(1);
    r = write(s[1], msg, 5);
    report(r == 5, "write to empty socket");
    r = close(s[1]);
    report(r == 0, "close write side with non-empty buffer");
    t5.join();

    // Test waiting poll() on read side, when write side closes (POLLHUP expected)
    r = pipe(s);
    report(r == 0, "pipe call");
    std::thread t6([&] {
        sleep(1);
        r2 = close(s[1]);
        report(r2 == 0, "close write side");
    });
    poller = { s[0], POLLIN, 0 };
    r = poll(&poller, 1, 5000);
    report(r==1, "wait for pipe to be readable");
    report(poller.revents & POLLHUP, "POLLHUP signaled"); // POLLHUP|POLLIN or POLLHUP are fine.
    t6.join();
    r = close(s[0]);
    report(r == 0, "close also read side");

    // Test poll() on read side, when write side is already closed (POLLHUP expected)
    r = pipe(s);
    report(r == 0, "pipe call");
    r = close(s[1]);
    report(r == 0, "close write side");
    poller = { s[0], POLLIN, 0 };
    r = poll(&poller, 1, 0);
    report(r==1, "pipe is readable");
    report(poller.revents & POLLHUP, "POLLHUP signaled");
    r = close(s[0]);
    report(r == 0, "close also read side");

    // Test waiting poll() on write side, when read side closes (POLLERR
    // expected, as Linux does, though POLLRDHUP would also make sense...)
    r = pipe(s);
    report(r == 0, "pipe call");
    // fill the write buffer of the pipe, and check it is no longer writable
    poller = { s[1], POLLOUT, 0 };
    r = poll(&poller, 1, 0);
    report(r==1 && poller.revents == POLLOUT, "empty pipe is ready for write");
    r = fcntl(s[1], F_SETFL, O_NONBLOCK);
    report(r == 0, "set write side to nonblocking");
    buf1 = (char*) calloc(1, TSTBUFSIZE);
    r = write(s[1], buf1, TSTBUFSIZE);
    free(buf1);
    report(r > 0, "large write to fill pipe");
    r = fcntl(s[1], F_SETFL, 0);
    report(r == 0, "set write side to blocking");
    r = poll(&poller, 1, 0);
    report(r==0, "full pipe is not ready for write");

    std::thread t7([&] {
        sleep(1);
        r2 = close(s[0]);
        report(r2 == 0, "close read side");
    });
    r = poll(&poller, 1, 5000);
    report(r==1, "wait for pipe to be ready for write");
    report(poller.revents & POLLERR, "POLLERR signaled"); // Following Linux, we return POLLERR|POLLOUT.
    t7.join();
    r = close(s[1]);
    report(r == 0, "close also write side");

    // Test poll() on write side, when read side is already closed (POLLERR expected)
    r = pipe(s);
    report(r == 0, "pipe call");
    r = close(s[0]);
    report(r == 0, "close read side");
    poller = { s[1], POLLOUT, 0 };
    r = poll(&poller, 1, 0);
    report(r==1, "closed pipe is ready for write");
    report(poller.revents & POLLERR, "POLLERR signaled");
    // See that we can get this POLLERR even if we don't specify any events
    // to poll for/ This agrees with the SUS definition of poll(), and Linux's
    // behavior in practice.
    poller = { s[1], 0, 0 };
    r = poll(&poller, 1, 0);
    report(r==1, "closed pipe has a non-user-requested event");
    report(poller.revents & POLLERR, "POLLERR signaled");
    r = close(s[1]);
    report(r == 0, "close also write side");


    std::cout << "SUMMARY: " << tests << " tests, " << fails << " failures\n";
    return fails == 0 ? 0 : 1;
}


