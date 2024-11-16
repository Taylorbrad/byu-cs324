cmake_minimum_required(VERSION 3.29)
project(byu_cs324 C)

set(CMAKE_C_STANDARD 11)

include_directories(include)

add_executable(byu_cs324
    01c-hw-remote/hello.c
    01d-hw-strings-io-env/learn_c.c
    03-hw-fork-exec/exec.c
    03-hw-fork-exec/fork.c
    04-lab-shell-pipeline/mycat.c
    04-lab-shell-pipeline/mygrep.c
    04-lab-shell-pipeline/myintgroup.c
    04-lab-shell-pipeline/myppid.c
    04-lab-shell-pipeline/myspin.c
    04-lab-shell-pipeline/tsh.c
    05-hw-signals/killer.c
    05-hw-signals/signals.c
    06-lab-shell-signals/myint.c
    06-lab-shell-signals/myspin.c
    06-lab-shell-signals/mysplit.c
    06-lab-shell-signals/mystop.c
    06-lab-shell-signals/tsh.c
    07-hw-sockets/client.c
    07-hw-sockets/server.c
    08-lab-socket-hunt/treasure_hunter.c
    09b-hw-concurrency/echo.c
    09b-hw-concurrency/echo_cnt.c
    09b-hw-concurrency/echoserveri.c
    09b-hw-concurrency/echoserverp.c
    09b-hw-concurrency/echoservert.c
    09b-hw-concurrency/echoservert_pre.c
    10-lab-proxy-threadpool/www/cgi-bin/slow.c
    10-lab-proxy-threadpool/proxy.c
    11a-hw-openmp/mandelbrot.c
    11b-hw-iomultiplex/echoservere.c
    12-lab-proxy-iomultiplex/www/cgi-bin/slow.c
    12-lab-proxy-iomultiplex/proxy.c
    code/sbuf.c
    code/sockhelper.c
    include/sbuf.h
    include/sockhelper.h)
