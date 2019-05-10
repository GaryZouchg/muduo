//
// Created by test2 on 19-5-10.
//

#include "faceIDserver.h"
#include <muduo/base/Logging.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/TcpServer.h>

#include <stdio.h>
#include <unistd.h>

using namespace muduo;
using namespace muduo::net;

const char* g_file = NULL;

void onHighWaterMark(const TcpConnectionPtr& conn, size_t len)
{
    LOG_INFO << "HighWaterMark " << len;
}

void onConnection(const TcpConnectionPtr& conn)
{
    LOG_INFO << "onConnection faceIDserver - " << conn->peerAddress().toIpPort() << " -> "
             << conn->localAddress().toIpPort() << " is "
             << (conn->connected() ? "UP" : "DOWN");

    char buf[24]="123abcdefghijklmn";
    if (conn->connected())
    {
        LOG_INFO << "faceIDserver - Sending buf " << buf
                 << " to " << conn->peerAddress().toIpPort();
        conn->setHighWaterMarkCallback(onHighWaterMark, 64*1024);

        conn->send(buf);
        //conn->shutdown();
        //LOG_INFO << "faceIDserver - done";
    }
}

void onMessage(const TcpConnectionPtr& conn, Buffer* msg, Timestamp timestamp)
{
    LOG_INFO << "onMessage faceIDserver - MSG: " << msg->toStringPiece()
             << "  timestamp: " << timestamp.toFormattedString(false);
    conn->send(msg);
}


int main(int argc, char* argv[])
{
    LOG_INFO << "pid = " << getpid();
    if (argc > 0)
    {
        g_file = argv[1];

        EventLoop loop;
        InetAddress listenAddr(2021);
        TcpServer server(&loop, listenAddr, "faceIDserver");
        server.setConnectionCallback(onConnection);
        server.setMessageCallback(onMessage);
        server.start();
        loop.loop();
    }
    else
    {
        fprintf(stderr, "Usage: %s faceIDserver\n", argv[0]);
    }
}

