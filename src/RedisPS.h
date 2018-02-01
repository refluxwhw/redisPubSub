#ifndef REDISPS_H
#define REDISPS_H

#include <stdlib.h>
#include <string>
#include <vector>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <functional>

#include <async.h>
#include <adapters/libevent.h>

typedef std::function<void (const std::string& eventName, const std::string& message)> SubCallback;

class RedisPS
{
public:
    RedisPS();

    bool init();
    bool uninit();
    bool connect(const std::string& host, int port = 6379);
    bool disconnect();

    bool publish(const std::string &eventName,
                 const std::string &message);

    bool subscribe(const std::string &eventName);

    void setSubCallBack(SubCallback cb);

private:
    // 下面三个回调函数供redis服务调用
    // 连接回调
    static void connect_callback(const redisAsyncContext *redis_context,
                                 int status);

    // 断开连接的回调
    static void disconnect_callback(const redisAsyncContext *redis_context,
                                    int status);

    // 执行命令回调
    static void command_callback(redisAsyncContext *redis_context,
                                 void *reply, void *privdata);

    // 事件分发线程函数
    static void *event_thread(void *data);
    void *event_proc();

private:
    // libevent事件对象
    event_base *_event_base = nullptr;
    // 事件线程ID
    pthread_t _event_thread = 0;
    // 事件线程的信号量
    sem_t _event_sem;
    // hiredis异步对象
    redisAsyncContext *_redis_context = nullptr;


    SubCallback _cb_fn = nullptr;
};

#endif // REDISPS_H
