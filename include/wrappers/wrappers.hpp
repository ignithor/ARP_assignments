#ifndef WRAPPERS_H
#define WRAPPERS_H
#include "utility/utility.hpp"
#include <sys/stat.h>
#include <sys/select.h>
#include <unistd.h>
#include <sstream>             
#include <iostream>             
#include <string>               
#include <thread>           
#include <chrono>   
#include <cstring>  

int Read(int fd, void *buf, size_t nbytes);
int Write(int fd, void *buf, size_t nbytes);
int Select_wmask(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
                 struct timeval *timeout);
int Close(int fd);
void Kill(int pid, int signal);
#endif // !WRAPPERS_H
