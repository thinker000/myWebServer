#include <unistd.h>
#include "server/webserver.hpp"

int main(){
    WebServer server(1316,3,6); /* 端口 ET模式 线程池数量 */
    server._Start();
    return 0;
}
