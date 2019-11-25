#pragma once

#include <unistd.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstdio>
#include <cstring>
#include <errno.h>
#include <list>
#include <iostream>
#include <signal.h>
#include <queue>

#include "package.hpp"

#define MAX_CONTENT_SIZE 1024

struct Package {
    package::header head;
    char content[MAX_CONTENT_SIZE];
};

struct Connection {
    int sockfd;
    std::queue<Package> wqueue;
    std::queue<Package> rqueue;
    int devid;
    int devno;
    int ram;
    int request_num;
    int response_num;
    int tty_num;
    int tty_num_received;

    Connection() {
        tty_num = 0;
        tty_num_received = 0;
        request_num = 0;
        response_num = 0;
    }
};