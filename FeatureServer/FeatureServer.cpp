//
// Created by test2 on 19-5-14.
//

#include "FeatureServer.h"
#include "ConfigFileReader.h"

using namespace muduo;
using namespace muduo::net;

FeatureServer::FeatureServer()
{
    spEventLoop_.reset(new EventLoop());

    ConfigFileReader::GetInstance()->LoadFromFile("/home/test2/github/muduo/config.cfg");
    //std::string strPort = ConfigFileReader::GetInstance()->GetConfigName("listen_port");
    nPort_ = static_cast<uint16_t>(atoi(ConfigFileReader::GetInstance()->GetConfigName("listen_port")));

    InetAddress listenAddr(2021);
    spTcpServer_.reset(new TcpServer(spEventLoop_.get(), listenAddr, "FeatureServer"));

    spTcpServer_->setConnectionCallback(std::bind(&FeatureServer::OnConnection,this,_1) );
    spTcpServer_->setMessageCallback(std::bind(&FeatureServer::OnMessage,this,_1,_2,_3) );

    strSaveFilePath_ = ConfigFileReader::GetInstance()->GetConfigName("save_file_path");

    //connect to redies

    InetAddress serverAddr("127.0.0.1", 6379);

}
void FeatureServer::Run()
{
    spTcpServer_->start();
    spEventLoop_->loop();

}

void FeatureServer::OnConnection(const TcpConnectionPtr& conn)
{
    LOG_INFO << "onConnection faceIDserver - " << conn->peerAddress().toIpPort() << " -> "
             << conn->localAddress().toIpPort() << " is "
             << (conn->connected() ? "UP" : "DOWN");

    if (conn->connected())
    {
       // conn->setHighWaterMarkCallback(onHighWaterMark, 64*1024);
    }
}

void FeatureServer::onHighWaterMark(const TcpConnectionPtr& conn, size_t len)
{
    LOG_INFO << "HighWaterMark " << len;
}

void FeatureServer::OnMessage(const TcpConnectionPtr& conn, Buffer* msg, Timestamp timestamp)
{
    if( strlen(msg->peek()) < 1)
    {
        LOG_INFO  << "empty buffer readable Bytes : " << msg->readableBytes();
        return;
    }

    LOG_INFO << "readable  Bytes : " << msg->readableBytes()
             << "writeable Bytes : " << msg->writableBytes()
             << "timestamp : "<<timestamp.toFormattedString();

    while(msg->readableBytes()>=512)
    {
        std::string saveFile = strSaveFilePath_ + std::to_string(Timestamp::now().microSecondsSinceEpoch()) + ".bin";
        WriteFile(saveFile.c_str(),msg->retrieveAsString(512));
    }
}


void FeatureServer::WriteFile(const char* filename, const std::string & filecontent)
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