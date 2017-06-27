#include "server.h"
#include "commands.h"

#include <iostream>
#include <string>

#include <cstring>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <cstdio>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>

#define ASSERT(cond, msg) if(!(cond)) { std::cerr << msg << std::endl; exit(1); };
#define PORT "2909"

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

Server::Server(Backend* backend) : m_backend(backend) {}

void Server::listener() {
    CommandHandler cmd_handler(m_backend);
    int s; // success holder

    // === Setup ===
    ::fd_set master, read_fds;
    FD_ZERO(&master);
    FD_ZERO(&read_fds);

    struct ::addrinfo hints, *ai;
    std::memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    s = ::getaddrinfo(NULL, PORT, &hints, &ai);
    ASSERT(s == 0, gai_strerror(s));

    int listener;
    struct ::addrinfo* p;
    for(p = ai; p != NULL; p = p->ai_next) {
        listener = ::socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (listener < 0) continue;

        int yes = 1;
        ::setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

        s = ::bind(listener, p->ai_addr, p->ai_addrlen);
        if (s < 0) { ::close(listener); continue; }
        break;
    }
    ASSERT(p != NULL, "Failed to bind");

    ::freeaddrinfo(ai);

    s = ::listen(listener, 10);
    ASSERT(s != -1, "Error in listen");

    FD_SET(listener, &master);
    int fdmax = listener;

    // === Main loop ===
    while (m_listen) {
        read_fds = master;
        struct timeval timeout { .tv_sec = 1, .tv_usec = 0 };
        s = ::select(fdmax+1, &read_fds, NULL, NULL, &timeout);
        ASSERT(s != -1, "Error in select");

        for (int i=0; i <= fdmax; i++) {
            if (!FD_ISSET(i, &read_fds))
                continue;

            if (i == listener) {
                // handle new connections
                struct ::sockaddr_storage remoteaddr;
                ::socklen_t addrlen = sizeof remoteaddr;
                int newfd = ::accept(listener, (struct sockaddr*) &remoteaddr, &addrlen);
                ASSERT(newfd != -1, "Error in accept");

                FD_SET(newfd, &master);
                fdmax = newfd > fdmax ? newfd : fdmax;

                char remoteIP[INET6_ADDRSTRLEN];

                std::cout << "New connection from "
                    << ::inet_ntop(remoteaddr.ss_family,
                            get_in_addr((struct sockaddr*) &remoteaddr),
                            remoteIP, INET6_ADDRSTRLEN)
                    << " on socket " << newfd << std::endl;
            } else {
                // handle data from a client
                char buf[256];
                int nbytes = ::recv(i, buf, 256, 0);
                if (nbytes == 0)
                    std::cerr << "socket " << i << " hung up" << std::endl;
                else if (nbytes < 0)
                    std::perror("recv");
                if (nbytes <= 0) {
                    ::close(i);
                    FD_CLR(i, &master);
                    continue;
                }

                // we got some data from a client
                if (FD_ISSET(i, &master)){
                    /* if (std::strlen(buf) == 1 && (buf[0] == '\n' || buf[0] == '\r')) */
                    /*     std::memset(buf, 0, sizeof buf); */
                    /*     continue; */

                    while (buf[std::strlen(buf)-1] == '\n' || buf[std::strlen(buf)-1] == '\r')
                        buf[std::strlen(buf)-1] = 0;


                    if (std::strcmp(buf, "stop") == 0 || std::strcmp(buf, "quit") == 0) {
                        m_listen = false;
                        stopped = true;
                        ::send(i, "stopping...\n", 11, 0);
                        std::cout << "stopping..." << std::endl;
                        std::memset(buf, 0, sizeof buf);
                        continue;
                    }

#define HNDL_EXCPT() std::cout << e.what() << std::endl; \
                     response = std::string(e.what())+'\n';

                    std::string response;
                    try {
                        response = cmd_handler.run(std::string(buf))+'\n';
                    } catch (CommandHandler::CommandHandlerException& e) {
                        HNDL_EXCPT();
                    } catch (Settings::SettingsException& e) {
                        HNDL_EXCPT();
                    }
                    s = ::send(i, response.c_str(), response.length()-1, 0);
                    std::cout << response;

                    ASSERT(s != -1, "Error in send");
                }
                std::memset(buf, 0, sizeof buf);
            }
        }
    }
}

void Server::start() {
    m_listen = true;
    m_listener_thread = std::thread([&](){ listener(); });
}

void Server::stop() {
    m_listen = false;
    stopped = true;
    /* ::close(m_newsockfd); */
    /* ::close(m_sockfd); */
    /* m_listener_thread.join(); */
}
