#include "RedisPS.h"
#include <string.h>


RedisPS::RedisPS()
{

}

bool RedisPS::init()
{
    // initialize the event
    _event_base = event_base_new();    // 创建libevent对象
    if (NULL == _event_base)
    {
        printf(": Create redis event failed.\n");
        return false;
    }

    memset(&_event_sem, 0, sizeof(_event_sem));
    int ret = sem_init(&_event_sem, 0, 0);
    if (ret != 0)
    {
        printf(": Init sem failed.\n");
        return false;
    }

    return true;
}

bool RedisPS::uninit()
{
    _event_base = NULL;

    sem_destroy(&_event_sem);
    return true;
}

bool RedisPS::connect(const std::string &host, int port)
{
    // connect redis
    _redis_context = redisAsyncConnect(host.c_str(), port);// 异步连接到redis服务器上
    if (NULL == _redis_context)
    {
        printf(": Connect redis failed.\n");
        return false;
    }

    if (_redis_context->err)
    {
        printf(": Connect redis error: %d, %s\n",
               _redis_context->err, _redis_context->errstr);// 输出错误信息
        return false;
    }

    // attach the event
    // 将事件绑定到redis context上，使设置给redis的回调跟事件关联
    redisLibeventAttach(_redis_context, _event_base);

    // 创建事件处理线程
    int ret = pthread_create(&_event_thread, 0, &RedisPS::event_thread, this);
    if (ret != 0)
    {
        printf(": create event thread failed.\n");
        disconnect();
        return false;
    }

    // 设置连接回调，当异步调用连接后，服务器处理连接请求结束后调用，通知调用者连接的状态
    redisAsyncSetConnectCallback(_redis_context, &RedisPS::connect_callback);

    // 设置断开连接回调，当服务器断开连接后，通知调用者连接断开，调用者可以利用这个函数实现重连
    redisAsyncSetDisconnectCallback(_redis_context,
                                    &RedisPS::disconnect_callback);

    // 启动事件线程
    sem_post(&_event_sem);
    return true;
}

bool RedisPS::disconnect()
{
    if (_redis_context)
    {
        redisAsyncDisconnect(_redis_context);
        redisAsyncFree(_redis_context);
        _redis_context = NULL;
    }

    return true;
}

bool RedisPS::publish(const std::string &eventName, const std::string &message)
{
    int ret = redisAsyncCommand(_redis_context,
                                &RedisPS::command_callback, this, "PUBLISH %s %s",
                                eventName.c_str(), message.c_str());
    if (REDIS_ERR == ret)
    {
        printf("Publish command failed: %d\n", ret);
        return false;
    }

    return true;
}

bool RedisPS::subscribe(const std::string &eventName)
{
    int ret = redisAsyncCommand(_redis_context,
                                &RedisPS::command_callback, this, "SUBSCRIBE %s",
                                eventName.c_str());
    if (REDIS_ERR == ret)
    {
        printf("Subscribe command failed: %d\n", ret);
        return false;
    }

    printf(": Subscribe success: %s\n", eventName.c_str());
    return true;
}

void RedisPS::setSubCallBack(SubCallback cb)
{
    _cb_fn = cb;
}

void RedisPS::connect_callback(const redisAsyncContext *redis_context, int status)
{
    if (status != REDIS_OK)
    {
        printf(": Error: %s\n", redis_context->errstr);
    }
    else
    {
        printf(": Redis connected!\n");
    }
}

void RedisPS::disconnect_callback(const redisAsyncContext *redis_context, int status)
{
    if (status != REDIS_OK)
    {
        // 这里异常退出，可以尝试重连
        printf(": Error: %s\n", redis_context->errstr);
    }
}

void RedisPS::command_callback(redisAsyncContext __attribute__((unused)) *redis_context, void *reply, void *privdata)
{
    printf("command callback.\n");

    if (NULL == reply || NULL == privdata) {
        return ;
    }

    // 静态函数中，要使用类的成员变量，把当前的this指针传进来，用this指针间接访问
    //RedisPS *self_this = reinterpret_cast<RedisPS *>(privdata);
    redisReply *redis_reply = reinterpret_cast<redisReply *>(reply);

    // 订阅接收到的消息是一个带三元素的数组
    if (redis_reply->type == REDIS_REPLY_ARRAY &&
            redis_reply->elements == 3)
    {
        printf( ": Recieve message:%s:%zu:%s:%zu:%s:%zu\n",
                redis_reply->element[0]->str, redis_reply->element[0]->len,
                redis_reply->element[1]->str, redis_reply->element[1]->len,
                redis_reply->element[2]->str, redis_reply->element[2]->len);

        // 调用函数对象把消息通知给外层
        //self_this->notify_message_fn(redis_reply->element[1]->str,
        //        redis_reply->element[2]->str, redis_reply->element[2]->len);
        //self_this->_cb_fn();
    }
}

void *RedisPS::event_thread(void *data)
{
    if (NULL == data)
    {
        printf(": Error!\n");
        return NULL;
    }

    RedisPS *self_this = reinterpret_cast<RedisPS *>(data);
    return self_this->event_proc();
}

void *RedisPS::event_proc()
{
    sem_wait(&_event_sem);

    // 开启事件分发，event_base_dispatch会阻塞
    event_base_dispatch(_event_base);

    return NULL;
}
