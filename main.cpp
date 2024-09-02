#include <unistd.h>
#include "server/webserver.hpp"

int main(){
    WebServer server(1316,3); /* 端口 ET模式 */
    server._Start();
    return 0;
}
