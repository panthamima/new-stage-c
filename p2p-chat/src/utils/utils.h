#ifndef C_P2P_CHAT_UTILS_H
#define C_P2P_CHAT_UTILS_H

#include "../chat.h"

void parseConnectAddress(int argc, char *argv[], char** ip, int* port);
void parseSoursePort(int argc, char *argv[], int* port);
void parseName(int argc, char *argv[], char* name);
 
#endif