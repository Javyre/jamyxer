#ifndef MAIN_H
#define MAIN_H

#include "backend.h"
#include "server.h"
#include <string>
#include <map>

int main(int argc, char** argv);

void cmd_loop(Backend* backend, Server* server);

#endif
