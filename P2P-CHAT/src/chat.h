#ifndef C_P2P_CHAT_CHAT_H
#define C_P2P_CHAT_CHAT_H

#include <ncursesw/curses.h> 
#include <locale.h> 
#include <stdio.h>  
#include <stdlib.h> 
#include <string.h> 
#include <unistd.h> 
#include <fcntl.h>  
#include <arpa/inet.h> 
#include <sys/ioctl.h> 

#include "config.h"
#include "clients.h"
#include "network/packet.h"
#include "network/socket.h"
#include "utils/interface.h"

void escape(const char* error);
void connectToClient(int sockfd, const struct sockaddr_in* addr, const char* name);
void sendPacket(int sockfd, const char* buf, int buf_size);

#endif