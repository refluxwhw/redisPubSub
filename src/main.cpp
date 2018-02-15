#include <iostream>

using namespace std;

#include "RedisPS.h"

bool run_pub(const std::string& e)
{
    RedisPS ps;
    if (!ps.init())
    {
        printf("pub init failed\n");
        return false;
    }

    if (!ps.connect("127.0.0.1"))
    {
        printf("pub connect failed\n");
        return false;
    }

    char buffer[1024];
    while (true)
    {
        printf("input pub message: ");
        fgets(buffer, 1023, stdin);
        string s = buffer;

        if (!ps.publish(e, s))
        {
            printf("publish failed\n");
            return false;
        }

        printf("publish success, message is: %s\n", s.c_str());
    }

    return false;
}

bool run_sub(const std::string& e)
{
    RedisPS ps;
    if (!ps.init())
    {
        printf("sub init failed\n");
        return false;
    }

    if (!ps.connect("127.0.0.1"))
    {
        printf("sub connect failed\n");
        return false;
    }

    ps.setSubCallBack([] (const std::string& eventName, const std::string& message) {
        printf("\nrecv sub event [%s], message is: %s\n",
               eventName.c_str(), message.c_str());
    });

    if (!ps.subscribe(e))
    {
        printf("\nsub event [%s] failed\n", e.c_str());
        return false;
    }

    printf("\nsub event [%s] success\n", e.c_str());

    char buffer[1024];
    while (true)
    {
        fgets(buffer, 1023, stdin);
        if (buffer[0] == 'q')
        {
            break;
        }
    }

    return false;
}


int main(int argc, char** argv)
{
    if (argc != 3)
    {
        printf("usage: %s <p|s> <event>\n", argv[0]);
        return -1;
    }

    string ps = argv[1];
    if (ps == "p")
    {
        run_pub(argv[2]);
    }
    else if (ps == "s")
    {
        run_sub(argv[2]);
    }
    else
    {
        printf("usage: %s <p|s> <event>\n", argv[0]);
        return -1;
    }

    return 0;
}
