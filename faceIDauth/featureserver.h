//
// Created by test2 on 19-5-14.
//

#ifndef MUDUO_FEATURESERVER_H
#define MUDUO_FEATURESERVER_H

#include "singleton.h"


class FeatureServer: public Singleton<FeatureServer>
{

public:
    FeatureServer();
    void Run();
    void OnConnection(const TcpConnectionPtr& conn);
    void onMessage(const TcpConnectionPtr& conn, Buffer* msg, Timestamp timestamp);


private:
    void writeFile(const char* filename, const std::string & filecontent);

private:


    EventLoop loop;
    InetAddress listenAddr;
    TcpServer server;
};


#endif //MUDUO_FEATURESERVER_H
