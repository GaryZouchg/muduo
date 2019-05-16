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

    //Tcp server
    InetAddress listenAddr(2021);
    spTcpServer_.reset(new TcpServer(spEventLoop_.get(), listenAddr, "FeatureServer"));
    spTcpServer_->setConnectionCallback(std::bind(&FeatureServer::OnConnection,this,_1) );
    spTcpServer_->setMessageCallback(std::bind(&FeatureServer::OnMessage,this,_1,_2,_3) );


    //connect to redies

    InetAddress redisAddr("127.0.0.1", 6379);
    spHiRedis_.reset(new HiRedis(spEventLoop_.get(), redisAddr));


    strSaveFilePath_ = ConfigFileReader::GetInstance()->GetConfigName("save_file_path");

}
void FeatureServer::Run()
{

    spHiRedis_->setConnectCallback(FeatureServer::OnRedisConnect);
    spHiRedis_->setDisconnectCallback(FeatureServer::OnRedisDisconnect);

    spHiRedis_->connect();


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

    LOG_INFO << "On message readable  Bytes : " << msg->readableBytes()
             << "  writeable Bytes : " << msg->writableBytes()
             << "  timestamp : "<<timestamp.toFormattedString();

    while(msg->readableBytes()>=512)
    {

        std::string saveFile = strSaveFilePath_ + std::to_string(Timestamp::now().microSecondsSinceEpoch()) + ".bin";
        WriteFile(saveFile.c_str(),msg->retrieveAsString(512));
    }

    spHiRedis_->command(FeatureServer::OnCMDdbsize,"dbsize");
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

        size_t nwrite = ::fwrite(filecontent.c_str(), 1, filecontent.size(), fp);
        LOG_INFO << "write file: " << filename <<"  Byte of fwrite: "<< nwrite << " length of filecontent: " << filecontent.size();

        ::fclose(fp);
    }
    else
    {
        LOG_ERROR<< filename << " open failed   ";
    }
}


void FeatureServer::OnRedisConnect(HiRedis* c,int status)
{
    if (status != REDIS_OK)
    {
        LOG_ERROR << "connectCallback Error:" << c->errstr();
    }
    else
    {
        LOG_INFO << "Redis Connected...";
    }
}

void FeatureServer::OnRedisDisconnect(HiRedis* c,int status)
{
    if (status != REDIS_OK)
    {
        LOG_ERROR << "disconnectCallback Error:" << c->errstr();
    }
    else
    {
        LOG_INFO << "Redis Disconnected...";
    }
}

string FeatureServer::ToString(long long value)
{
    char buf[32];
    snprintf(buf, sizeof buf, "%lld", value);
    return buf;
}

string FeatureServer::RedisReplyToString(const redisReply* reply)
{
    static const char* const types[] = { "",
                                         "REDIS_REPLY_STRING", "REDIS_REPLY_ARRAY",
                                         "REDIS_REPLY_INTEGER", "REDIS_REPLY_NIL",
                                         "REDIS_REPLY_STATUS", "REDIS_REPLY_ERROR" };
    string str;
    if (!reply) return str;

    str += types[reply->type] + string("(") + ToString(reply->type) + ") ";

    str += "{ ";
    if (reply->type == REDIS_REPLY_STRING ||
        reply->type == REDIS_REPLY_STATUS ||
        reply->type == REDIS_REPLY_ERROR)
    {
        str += '"' + string(reply->str, reply->len) + '"';
    }
    else if (reply->type == REDIS_REPLY_INTEGER)
    {
        str += ToString(reply->integer);
    }
    else if (reply->type == REDIS_REPLY_ARRAY)
    {
        str += ToString(reply->elements) + " ";
        for (size_t i = 0; i < reply->elements; i++)
        {
            str += " " + RedisReplyToString(reply->element[i]);
        }
    }
    str += " }";

    return str;
}


void FeatureServer::OnCMDdbsize(HiRedis* c, redisReply* reply)
{
    LOG_INFO << "dbsize " << RedisReplyToString(reply);
}