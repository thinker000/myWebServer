#include <unistd.h>
#include "server/webserver.hpp"

int main(int argc,char* argv[]){
    int port = std::stoi(argv[1]);
    WebServer server(port,3); /* 端口 ET模式 */
    server._Start();
    return 0;
}
