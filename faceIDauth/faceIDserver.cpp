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



void writeFile(const char* filename, const std::string & filecontent)
{
    string content;
    FILE* fp = ::fopen(filename, "w+");
    if (fp)
    {
        // inefficient!!!
        const int kBufSize = 1024*1024;
        char iobuf[kBufSize];
        ::setbuffer(fp, iobuf, sizeof iobuf);

        size_t nread = 0;

        nread = ::fwrite(filecontent.c_str(), 1, filecontent.size(), fp);

        if(nread> 0)
        {
            LOG_INFO << "write file nreaed: " << nread << "  string length of filecontent: " << filecontent.size();
        }

        ::fclose(fp);
    }

}

void onHighWaterMark(const TcpConnectionPtr& conn, size_t len)
{
    LOG_INFO << "HighWaterMark " << len;
}

void onConnection(const TcpConnectionPtr& conn)
{
    LOG_INFO << "onConnection faceIDserver - " << conn->peerAddress().toIpPort() << " -> "
             << conn->localAddress().toIpPort() << " is "
             << (conn->connected() ? "UP" : "DOWN");

//    char buf[24]="123abcdefghijklmn";
    if (conn->connected())
    {
        conn->setHighWaterMarkCallback(onHighWaterMark, 64*1024);
    }
}

void onMessage(const TcpConnectionPtr& conn, Buffer* msg, Timestamp timestamp)
{

    if( strlen(msg->peek()) < 1)
    {
        LOG_INFO  << "empty buffer readable Bytes : " << msg->readableBytes();
        return;

    }

    std::string saveFilePath("/home/test2/scp/received-feature/");

    LOG_INFO << "readable  Bytes : " << msg->readableBytes()
             << "writeable Bytes : " << msg->writableBytes();
    //LOG_INFO << "write to  file  : " << saveFile;

    while(msg->readableBytes()>=512)
    {
        std::string saveFile = saveFilePath + std::to_string(Timestamp::now().microSecondsSinceEpoch()) + ".bin";

        writeFile(saveFile.c_str(),msg->retrieveAsString(512));

    }

}

int main(int argc, char* argv[])
{
    LOG_INFO << "pid = " << getpid();
    if (argc > 0)
    {
        //std::string saveFilePath = argv[1];

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

