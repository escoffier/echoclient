#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <signal.h>
#ifndef _WIN32
#include <netinet/in.h>
#ifdef _XOPEN_SOURCE_EXTENDED
#include <arpa/inet.h>
#endif
#include <sys/socket.h>
#endif
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/listener.h>
#include <event2/util.h>
#include <event2/event.h>
#include <event2/event_struct.h>
#include <iostream>
#include <unistd.h>
#include <memory>

#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/daily_file_sink.h"

const int PORT = 9995;
const char *SERVER = "192.168.0.105";

//#include "spdlog/sinks/basic_file_sink.h"
void initLog()
{
    //auto console = spdlog::stdout_color_mt("console");
    //console->info("Welcome to spdlog!");
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    console_sink->set_level(spdlog::level::debug);
    //console_sink->set_pattern("[echoserver] [%^%l%$] %v");
    //console_sink->set_pattern("[%u @ %j:%#] %v");

    auto file_sink = std::make_shared<spdlog::sinks::daily_file_sink_mt>("logs/echoserver.log", 3, 46, true);
    // auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("logs/multisink.txt",true);
    file_sink->set_level(spdlog::level::trace);

    //spdlog::logger logger("multi_sink", {console_sink, file_sink});
    std::vector<spdlog::sink_ptr> sinks{console_sink, file_sink};

    auto logger = std::make_shared<spdlog::logger>("echoclient", sinks.begin(), sinks.end());
    logger->set_level(spdlog::level::trace);
    spdlog::register_logger(logger);
    logger->info("Welcome to spdlog!");
    logger->flush();
}

static void readcb(struct bufferevent *bev, void *ctx)
{
    char data[1024] = {0};
    struct evbuffer *input = bufferevent_get_input(bev);
    bufferevent_read(bev, data, evbuffer_get_length(input));
    std::cout << "receivce data:" << data;
}

static void event_cb(struct bufferevent *bev, short events, void *ptr)
{
    if (events & BEV_EVENT_CONNECTED)
    {
        auto fd = bufferevent_getfd(bev);
        spdlog::get("echoclient")->info("BEV_EVENT_CONNECTED fd: {}", fd);
    }
    else if(events & BEV_EVENT_EOF)
    {
        spdlog::get("echoclient")->info("peer closed");
    }
}

static void
read_stdin(struct bufferevent *bev, void *user_data)
{
    char data[1024] = {0};
    struct evbuffer *input = bufferevent_get_input(bev);
    bufferevent_read(bev, data, evbuffer_get_length(input));
    std::cout << "stdin receivce data:" << data;

    bufferevent *sockbev = (bufferevent*)user_data;

    bufferevent_write(sockbev, data, strlen(data));
}

static void
signal_cb(evutil_socket_t sig, short events, void *user_data)
{
    struct event_base *base = (event_base *)user_data;
    struct timeval delay = {1, 0};

    spdlog::get("echoclient")->info("Caught an interrupt signal [{}]; exiting cleanly in 1 seconds.", sig);
    //std::cout << "Caught an interrupt signal; exiting cleanly in 1 seconds.\n";
    event_base_loopexit(base, &delay);
}

int main(int argc, char const *argv[])
{
    /* code */
    initLog();
    auto logger = spdlog::get("echoclient");
    struct sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_port = htons(PORT);
    //sin.sin_addr.s_addr = htonl(INADDR_ANY);
    if (inet_pton(AF_INET, SERVER, &sin.sin_addr) < 0)
    {
        logger->error("inet_pton error");
        return 1;
    };

    struct event_base *base = event_base_new();

    if (base == nullptr)
    {
        return 1;
    }

    struct event *signalEvent = evsignal_new(base, SIGINT, signal_cb, base);

    if (!signalEvent)
    {
        logger->error("Could not create singal event");
        return 1;
    }

    if (event_add(signalEvent, nullptr) < 0)
    {
        logger->error("Could not add a singal event");
        return 1;
    }

    //event *inEv = event_new(base, 0 /*stdin*/, EV_READ | EV_PERSIST, read_stdin, base);

    bufferevent *bev = bufferevent_socket_new(base, -1, BEV_OPT_CLOSE_ON_FREE);
    bufferevent_setcb(bev, readcb, nullptr, event_cb, nullptr);
    bufferevent_enable(bev, EV_READ | EV_WRITE);

    bufferevent *inbev = bufferevent_socket_new(base, STDIN_FILENO, 0);
    bufferevent_setcb(inbev, read_stdin, nullptr, event_cb, bev);
    bufferevent_enable(inbev, EV_READ | EV_WRITE);

    if (bufferevent_socket_connect(bev, (sockaddr *)&sin, sizeof(sin)) != 0)
    {
        return 1;
    }

    spdlog::get("echoclient")->info("connected to {}", SERVER);

    event_base_dispatch(base);

    return 0;
}
