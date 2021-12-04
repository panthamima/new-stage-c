#ifndef C_P2P_CHAT_PACKET_H
#define C_P2P_CHAT_PACKET_H

#include "../chat.h"

#define PACKET_CONNECT_REQUES '0'
#define PACKET_CONNECT_ACCEPT '1'
#define PACKET_PING '3'
#define PACKET_TIMEOUT '4'
#define PACKET_REQUEST_USERS '5'
#define PACKET_LIST_USERS '6'
#define PACKET_SEND_MESSAGE '7'

int getPacketId(const char* data);
int createSimplePacket(char type, char* buf);
int createConnectRequestPacket(char* buf, const char* name);
int createConnectAcceptPacket(char* buf, const char* name);
int createMessagePacket(char* buf_send, char* buf_input, int len_msg);
int createListUsersPacket(char* buf);



#endif