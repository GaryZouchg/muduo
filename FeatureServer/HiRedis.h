//
// Created by test2 on 19-5-15.
//

#ifndef MUDUO_HIREDIS_H
#define MUDUO_HIREDIS_H


#include <muduo/base/noncopyable.h>
#include <muduo/base/StringPiece.h>
#include <muduo/base/Types.h>
#include <muduo/net/Callbacks.h>
#include <muduo/net/InetAddress.h>

#include <hiredis/hiredis.h>

struct redisAsyncContext;

namespace muduo
{
    namespace net
    {
        class Channel;
        class EventLoop;
    }
}

class HiRedis : public std::enable_shared_from_this<HiRedis>,
                muduo::noncopyable
{
public:
    typedef std::function<void(HiRedis*, int)> ConnectCallback;
    typedef std::function<void(HiRedis*, int)> DisconnectCallback;
    typedef std::function<void(HiRedis*, redisReply*)> CommandCallback;

    HiRedis(muduo::net::EventLoop* loop, const muduo::net::InetAddress& serverAddr);
    ~HiRedis();

    const muduo::net::InetAddress& serverAddress() const { return serverAddr_; }
    // redisAsyncContext* context() { return context_; }
    bool connected() const;
    const char* errstr() const;

    void setConnectCallback(const ConnectCallback& cb) { connectCb_ = cb; }
    void setDisconnectCallback(const DisconnectCallback& cb) { disconnectCb_ = cb; }

    void connect();
    void disconnect();  // FIXME: implement this with redisAsyncDisconnect

    int command(const CommandCallback& cb, muduo::StringArg cmd, ...);

    int ping();

private:
    void handleRead(muduo::Timestamp receiveTime);
    void handleWrite();

    int fd() const;
    void logConnection(bool up) const;
    void setChannel();
    void removeChannel();

    void connectCallback(int status);
    void disconnectCallback(int status);
    void commandCallback(redisReply* reply, CommandCallback* privdata);

    static HiRedis* getHiredis(const redisAsyncContext* ac);

    static void connectCallback(const redisAsyncContext* ac, int status);
    static void disconnectCallback(const redisAsyncContext* ac, int status);
    // command callback
    static void commandCallback(redisAsyncContext* ac, void*, void*);

    static void addRead(void* privdata);
    static void delRead(void* privdata);
    static void addWrite(void* privdata);
    static void delWrite(void* privdata);
    static void cleanup(void* privdata);

    void pingCallback(HiRedis* me, redisReply* reply);

private:
    muduo::net::EventLoop* loop_;
    const muduo::net::InetAddress serverAddr_;
    redisAsyncContext* context_;
    std::shared_ptr<muduo::net::Channel> channel_;
    ConnectCallback connectCb_;
    DisconnectCallback disconnectCb_;
};

#endif //MUDUO_HIREDIS_H
