//
// Created by test2 on 19-5-10.
//

#include <muduo/net/TcpClient.h>

#include <muduo/base/Logging.h>
#include <muduo/base/Thread.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/EventLoopThreadPool.h>
#include <muduo/net/InetAddress.h>

#include <utility>

#include <stdio.h>
#include <unistd.h>

using namespace muduo;
using namespace muduo::net;

class Client;

class Session : noncopyable
{
public:
    Session(EventLoop* loop,
            const InetAddress& serverAddr,
            const string& name,
            Client* owner)
            : client_(loop, serverAddr, name),
              owner_(owner),
              bytesRead_(0),
              bytesWritten_(0),
              messagesRead_(0)
    {
        client_.setConnectionCallback(
                std::bind(&Session::onConnection, this, _1));
        client_.setMessageCallback(
                std::bind(&Session::onMessage, this, _1, _2, _3));
    }

    void start()
    {
        client_.connect();
    }

    void stop()
    {
        client_.disconnect();
    }

    int64_t bytesRead() const
    {
        return bytesRead_;
    }

    int64_t messagesRead() const
    {
        return messagesRead_;
    }

private:

    void onConnection(const TcpConnectionPtr& conn);


    void onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp timestamp)
    {
        if( strlen(buf->peek()) < 1)
            return;

        LOG_WARN << "onMessage faceIDserver - MSG: " << buf->peek() << "  readable Bytes : " << buf->readableBytes()
                 << " timestamp: " << timestamp.toFormattedString(true);
        ++messagesRead_;
        bytesRead_ += buf->readableBytes();
        bytesWritten_ += buf->readableBytes();
        //conn->send(buf);
    }

    TcpClient client_;
    Client* owner_;
    int64_t bytesRead_;
    int64_t bytesWritten_;
    int64_t messagesRead_;
};

class Client : noncopyable
{
public:
    Client(EventLoop* loop,
           const InetAddress& serverAddr,
           int blockSize,
           int sessionCount,
           int timeout,
           int threadCount,
           std::string featureFilePath)
            : loop_(loop),
              threadPool_(loop, "faceID-client"),
              sessionCount_(sessionCount),
              timeout_(timeout),
              featureFilePath_(featureFilePath)
    {
        loop->runAfter(timeout, std::bind(&Client::handleTimeout, this));
        if (threadCount > 1)
        {
            threadPool_.setThreadNum(threadCount);
        }
        threadPool_.start();

        for (int i = 0; i < blockSize; ++i)
        {
            message_.push_back(static_cast<char>(i % 128));
        }

        for (int i = 0; i < sessionCount; ++i)
        {
            char buf[32];
            snprintf(buf, sizeof buf, "C%05d", i);
            Session* session = new Session(threadPool_.getNextLoop(), serverAddr, buf, this);
            session->start();
            sessions_.emplace_back(session);
        }
    }

    const string& message() const
    {
        return message_;
    }

    string featureFilePath() const
    {
        return featureFilePath_;
    }
    string readFile(const char* filename)
    {
        string content;
        FILE* fp = ::fopen(filename, "rb");
        if (fp)
        {
            // inefficient!!!
            const int kBufSize = 1024*1024;
            char iobuf[kBufSize];
            ::setbuffer(fp, iobuf, sizeof iobuf);

            char buf[kBufSize];
            size_t nread = 0;
            while ( (nread = ::fread(buf, 1, sizeof buf, fp)) > 0)
            {
                content.append(buf, nread);
            }
            ::fclose(fp);
        }
        return content;
    }

    void onConnect()
    {
        if (numConnected_.incrementAndGet() == sessionCount_)
        {
            LOG_WARN << "all connected";
        }
    }

    void onDisconnect(const TcpConnectionPtr& conn)
    {
        if (numConnected_.decrementAndGet() == 0)
        {
            LOG_WARN << "all disconnected";

            int64_t totalBytesRead = 0;
            int64_t totalMessagesRead = 0;
            for (const auto& session : sessions_)
            {
                totalBytesRead += session->bytesRead();
                totalMessagesRead += session->messagesRead();
            }
            LOG_WARN << totalBytesRead << " total bytes read";
            LOG_WARN << totalMessagesRead << " total messages read";
            LOG_WARN << static_cast<double>(totalBytesRead) / static_cast<double>(totalMessagesRead)
                     << " average message size";
            LOG_WARN << static_cast<double>(totalBytesRead) / (timeout_ * 1024 * 1024)
                     << " MiB/s throughput";
            conn->getLoop()->queueInLoop(std::bind(&Client::quit, this));
        }
    }

private:

    void quit()
    {
        loop_->queueInLoop(std::bind(&EventLoop::quit, loop_));
    }

    void handleTimeout()
    {
        LOG_WARN << "stop";
        for (auto& session : sessions_)
        {
            session->stop();
        }
    }

    EventLoop* loop_;
    EventLoopThreadPool threadPool_;
    int sessionCount_;
    int timeout_;
    std::vector<std::unique_ptr<Session>> sessions_;
    string message_;
    string featureFilePath_;
    AtomicInt32 numConnected_;
};

void Session::onConnection(const TcpConnectionPtr& conn)
{

    LOG_WARN << "faceIDclient - " << conn->peerAddress().toIpPort() << " -> "
             << conn->localAddress().toIpPort() << " is "
             << (conn->connected() ? "UP" : "DOWN");

    if (conn->connected())
    {
        conn->setTcpNoDelay(true);
        for(int i=0; i<10; i++ )
        {
            std::string featureFile = owner_->featureFilePath();
            featureFile += std::to_string(i);
            featureFile += ".bin";
            LOG_WARN << "Send message of file:  " << featureFile;
            std::string buf = owner_->readFile(featureFile.c_str());
            conn->send(buf);
        }
        owner_->onConnect();
    }
    else
    {
        owner_->onDisconnect(conn);
    }
}

int main(int argc, char* argv[])
{
    if (argc == 7)
    {
        fprintf(stderr, "Usage: client <host_ip> <port> <threads> <blocksize> ");
        fprintf(stderr, "<sessions> <time>\n");
    }
    else
    {
        LOG_INFO << "pid = " << getpid() << ", tid = " << CurrentThread::tid();
        Logger::setLogLevel(Logger::WARN);

        //const char* ip = argv[1];
        uint16_t port = static_cast<uint16_t>(atoi(argv[2]));
        int threadCount = atoi(argv[3]);
        int blockSize = atoi(argv[4]);
        int sessionCount = atoi(argv[5]);
        int timeout = atoi(argv[6]);

        std::string str_ip ="192.168.1.192";
        std::string featureFilePath = "/home/test2/scp/feature/eric/";
        port = 2021 ;
        threadCount = 1 ;
        blockSize = 64 ;
        sessionCount = 1 ;
        timeout = 10;   //5s 之后结束


        EventLoop loop;
        InetAddress serverAddr(str_ip.c_str(), port);

        Client client(&loop, serverAddr, blockSize, sessionCount, timeout, threadCount,featureFilePath);
        loop.loop();
    }
}

