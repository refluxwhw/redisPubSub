#include <iostream>
#include <getopt.h>
using namespace std;

#include "RedisPS.h"

#define PRINT_INVALID_ARGUMENT(app) printf("invalid arguments, try '%s --help' for more information.\n", app);

typedef struct {
    string host = "127.0.0.1";
    string user = "";
    uint16_t port = 6379;
    string passwd = "";
} AppArg;

AppArg g_arg;


bool run_pub(const std::string& e)
{
    RedisPS ps;
    if (!ps.init())
    {
        printf("pub init failed\n");
        return false;
    }

    if (!ps.connect(g_arg.host, g_arg.port))
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

    if (!ps.connect(g_arg.host, g_arg.port))
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

void printUsage(const char* app)
{
    printf("Usage: %s [options] <p|s> <event>\n"
           "\n"
           "  Options:\n"
           "  -H, --host=name      Redis host for connection, default host is localhost\n"
           "  -P, --port=#         Port number to use for connection, default port is 6379\n"
           "  -u, --user=name      User for auth\n"
           "  -p, --passwd[=name]  Password to use when connecting to server. If password is\n"
           "                       not given it's asked from the tty.\n"
           "\n"
           "      --help           display this help and exit\n"
           "      --version        output version information and exit\n",
           app);
}

void printVersion(const char* app)
{
    int majorV = 0;
    int minorV = 0;
    int fixedV = 0;
    printf("%s %d.%d.%d\n",
           app, majorV, minorV, fixedV);
}

void parseArgs(int argc, char** argv)
{
    static struct option long_options[] = {
        {"host",    required_argument, NULL, 'H'},
        {"port",    required_argument, NULL, 'P'},
        {"user",    required_argument, NULL, 'u'},
        {"passwd",  optional_argument, NULL, 'p'},
        {"help",    no_argument,       NULL, 'h'},
        {"version", no_argument,       NULL, 'v'},
        {NULL,      0,                 NULL, 0},
    };

    char opt;
    int option_index;
    while((opt = getopt_long_only(argc, argv, ":H:P:u:p::", long_options, &option_index))!= -1)
    {
        // printf("opt = %c\t\t", opt);
        // printf("optarg = %s\t\t",optarg);
        // printf("optind = %d\t\t",optind);
        // printf("argv[optind] =%s\t\t", argv[optind]);
        // printf("option_index = %d\n",option_index);

        switch (opt) {
        case 'H': // host
            g_arg.host = optarg;
            break;

        case 'P': // port
            g_arg.port = (uint16_t)atoi(optarg);
            break;

        case 'u': // user
            g_arg.user = optarg;
            break;

        case 'p': // passwd
            if (nullptr == optarg) // no passwd, please input
            {
#define READ_LINE_SIZE 64
                char buf[READ_LINE_SIZE] = {};
                fgets(buf, READ_LINE_SIZE-1, stdin);
                g_arg.passwd = buf;
            }
            else
            {
                g_arg.passwd = optarg;
            }
            break;

        case 'h': // help
            printUsage(argv[0]);
            exit(0);

        case 'v': // version
            printVersion(argv[0]);
            exit(0);

        case ':': // miss argument
            PRINT_INVALID_ARGUMENT(argv[0]);
            exit(1);

        default:
            break;
        }
    }
}

int main(int argc, char** argv)
{
    parseArgs(argc, argv);

    if (argc != optind + 2)
    {
        PRINT_INVALID_ARGUMENT(argv[0]);
        exit(1);
    }

    string ps = argv[optind];   // publish or subscrib
    string ev = argv[optind+1]; // event name
    if ("p" == ps)
    {
        run_pub(ev);
    }
    else if ("s" == ps)
    {
        run_sub(ev);
    }
    else
    {
        PRINT_INVALID_ARGUMENT(argv[0]);
        exit(1);
    }

    return 0;
}
