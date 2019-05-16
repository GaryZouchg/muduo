//
// Created by test2 on 19-5-15.
//

#include "HiRedis.h"

#include <muduo/base/Logging.h>
#include <muduo/net/Channel.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/SocketsOps.h>

#include <hiredis/async.h>

using namespace muduo;
using namespace muduo::net;

static void dummy(const std::shared_ptr<Channel>&)
{
}

HiRedis::HiRedis(EventLoop* loop, const InetAddress& serverAddr)
        : loop_(loop),
          serverAddr_(serverAddr),
          context_(NULL)
{
}

HiRedis::~HiRedis()
{
    LOG_DEBUG << this;
    assert(!channel_ || channel_->isNoneEvent());
    ::redisAsyncFree(context_);
}

bool HiRedis::connected() const
{
    return channel_ && context_ && (context_->c.flags & REDIS_CONNECTED);
}

const char* HiRedis::errstr() const
{
    assert(context_ != NULL);
    return context_->errstr;
}

void HiRedis::connect()
{
    assert(!context_);

    context_ = ::redisAsyncConnect(serverAddr_.toIp().c_str(), serverAddr_.toPort());

    context_->ev.addRead = addRead;
    context_->ev.delRead = delRead;
    context_->ev.addWrite = addWrite;
    context_->ev.delWrite = delWrite;
    context_->ev.cleanup = cleanup;
    context_->ev.data = this;

    setChannel();

    assert(context_->onConnect == NULL);
    assert(context_->onDisconnect == NULL);
    ::redisAsyncSetConnectCallback(context_, connectCallback);
    ::redisAsyncSetDisconnectCallback(context_, disconnectCallback);
}

void HiRedis::disconnect()
{
    if (connected())
    {
        LOG_DEBUG << this;
        ::redisAsyncDisconnect(context_);
    }
}

int HiRedis::fd() const
{
    assert(context_);
    return context_->c.fd;
}

void HiRedis::setChannel()
{
    LOG_DEBUG << this;
    assert(!channel_);
    channel_.reset(new Channel(loop_, fd()));
    channel_->setReadCallback(std::bind(&HiRedis::handleRead, this, _1));
    channel_->setWriteCallback(std::bind(&HiRedis::handleWrite, this));
}

void HiRedis::removeChannel()
{
    LOG_DEBUG << this;
    channel_->disableAll();
    channel_->remove();
    loop_->queueInLoop(std::bind(dummy, channel_));
    channel_.reset();
}

void HiRedis::handleRead(muduo::Timestamp receiveTime)
{
    LOG_TRACE << "receiveTime = " << receiveTime.toString();
    ::redisAsyncHandleRead(context_);
}

void HiRedis::handleWrite()
{
    if (!(context_->c.flags & REDIS_CONNECTED))
    {
        removeChannel();
    }
    ::redisAsyncHandleWrite(context_);
}

/* static */ HiRedis* HiRedis::getHiredis(const redisAsyncContext* ac)
{
    HiRedis* hiredis = static_cast<HiRedis*>(ac->ev.data);
    assert(hiredis->context_ == ac);
    return hiredis;
}

void HiRedis::logConnection(bool up) const
{
    InetAddress localAddr(sockets::getLocalAddr(fd()));
    InetAddress peerAddr(sockets::getPeerAddr(fd()));

    LOG_INFO << localAddr.toIpPort() << " -> "
             << peerAddr.toIpPort() << " redis  is "
             << (up ? "UP" : "DOWN");
}

/* static */ void HiRedis::connectCallback(const redisAsyncContext* ac, int status)
{
    LOG_TRACE;
    getHiredis(ac)->connectCallback(status);
}

void HiRedis::connectCallback(int status)
{
    if (status != REDIS_OK)
    {
        LOG_ERROR << context_->errstr << " failed to connect to " << serverAddr_.toIpPort();
    }
    else
    {
        logConnection(true);
        setChannel();
    }

    if (connectCb_)
    {
        connectCb_(this, status);
    }
}

/* static */ void HiRedis::disconnectCallback(const redisAsyncContext* ac, int status)
{
    LOG_TRACE;
    getHiredis(ac)->disconnectCallback(status);
}

void HiRedis::disconnectCallback(int status)
{
    logConnection(false);
    removeChannel();

    if (disconnectCb_)
    {
        disconnectCb_(this, status);
    }
}

void HiRedis::addRead(void* privdata)
{
    LOG_TRACE;
    HiRedis* hiredis = static_cast<HiRedis*>(privdata);
    hiredis->channel_->enableReading();
}

void HiRedis::delRead(void* privdata)
{
    LOG_TRACE;
    HiRedis* hiredis = static_cast<HiRedis*>(privdata);
    hiredis->channel_->disableReading();
}

void HiRedis::addWrite(void* privdata)
{
    LOG_TRACE;
    HiRedis* hiredis = static_cast<HiRedis*>(privdata);
    hiredis->channel_->enableWriting();
}

void HiRedis::delWrite(void* privdata)
{
    LOG_TRACE;
    HiRedis* hiredis = static_cast<HiRedis*>(privdata);
    hiredis->channel_->disableWriting();
}

void HiRedis::cleanup(void* privdata)
{
    HiRedis* hiredis = static_cast<HiRedis*>(privdata);
    LOG_DEBUG << hiredis;
}

int HiRedis::command(const CommandCallback& cb, muduo::StringArg cmd, ...)
{
    if (!connected()) return REDIS_ERR;

    LOG_TRACE;
    CommandCallback* p = new CommandCallback(cb);
    va_list args;
    va_start(args, cmd);
    int ret = ::redisvAsyncCommand(context_, commandCallback, p, cmd.c_str(), args);
    va_end(args);
    return ret;
}

/* static */ void HiRedis::commandCallback(redisAsyncContext* ac, void* r, void* privdata)
{
    redisReply* reply = static_cast<redisReply*>(r);
    CommandCallback* cb = static_cast<CommandCallback*>(privdata);
    getHiredis(ac)->commandCallback(reply, cb);
}

void HiRedis::commandCallback(redisReply* reply, CommandCallback* cb)
{
    (*cb)(this, reply);
    delete cb;
}

int HiRedis::ping()
{
    return command(std::bind(&HiRedis::pingCallback, this, _1, _2), "PING");
}

void HiRedis::pingCallback(HiRedis* me, redisReply* reply)
{
    assert(this == me);
    LOG_DEBUG << reply->str;
}
