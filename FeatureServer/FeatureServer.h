//
// Created by test2 on 19-5-14.
//

#ifndef MUDUO_FEATURESERVER_H
#define MUDUO_FEATURESERVER_H

#include <stdio.h>
#include <unistd.h>
#include <memory>

#include <muduo/base/Logging.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/TcpServer.h>

#include "Singleton.h"


using namespace muduo;
using namespace muduo::net;

class FeatureServer: public Singleton<FeatureServer>
{

public:
    FeatureServer();
    void Run();
    void OnConnection(const TcpConnectionPtr& conn);
    void onHighWaterMark(const TcpConnectionPtr& conn, size_t len);
    void OnMessage(const TcpConnectionPtr& conn, Buffer* msg, Timestamp timestamp);

private:
    void WriteFile(const char* filename, const std::string & filecontent);

private:


    std::unique_ptr<EventLoop> spEventLoop_;
    std::unique_ptr<TcpServer> spTcpServer_;

    uint16_t nPort_;
    std::string strSaveFilePath_;

};


#endif //MUDUO_FEATURESERVER_H
