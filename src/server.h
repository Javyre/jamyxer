#ifndef SERVER_H
#define SERVER_H

#include "backend.h"
#include <thread>

class Server {
    private:
        Backend* m_backend;
        int m_sockfd = 0;
        int m_newsockfd = 0;

        bool m_listen;

        void listener();
    public:
        bool stopped = false;
        Server(Backend* backend);
        std::thread m_listener_thread;

        void start();
        void stop();
};

#endif
