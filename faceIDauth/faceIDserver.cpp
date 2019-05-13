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



void writeFile(const char* filename, const char*  filecontent)
{
    string content;
    FILE* fp = ::fopen(filename, "w+");
    if (fp)
    {
        // inefficient!!!
        const int kBufSize = 1024*1024;
        char iobuf[kBufSize];
        ::setbuffer(fp, iobuf, sizeof iobuf);

        char buf[kBufSize];
        size_t nread = 0;
//        while ( (nread = ::fwrite(filecontent, 1, 512, fp)) > 0)
//        {
//            LOG_INFO << "in while loop  nreaed: " << nread ;
//        }
        nread = ::fwrite(filecontent, 1, sizeof buf, fp);

        if(nread> 0)
        {
            LOG_INFO << "write file nreaed: " << nread ;
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
//        LOG_INFO << "faceIDserver - Sending buf " << buf
//                 << " to " << conn->peerAddress().toIpPort();
        conn->setHighWaterMarkCallback(onHighWaterMark, 64*1024);

       // conn->send(buf);
        //conn->shutdown();
        //LOG_INFO << "faceIDserver - done";
    }
}

void onMessage(const TcpConnectionPtr& conn, Buffer* msg, Timestamp timestamp)
{

    if( strlen(msg->peek()) < 1)
    {
        LOG_INFO  << "empty buffer readable Bytes : " << msg->readableBytes();
        return;

    }



    std::string saveFile("/home/test2/scp/received-feature/");
    saveFile += std::to_string(timestamp.microSecondsSinceEpoch());
    saveFile += ".bin";

    LOG_INFO << "readable Bytes : " << msg->readableBytes()
             << "  file : " << saveFile;

    writeFile(saveFile.c_str(),msg->peek());
    //conn->send(msg);
    //sleep(1);
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

