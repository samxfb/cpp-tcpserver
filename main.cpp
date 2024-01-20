#include "tcpserver.h"
#include <iostream>

using namespace cpptcp;

class MyTcpServer : public TcpServer
{
public:
    MyTcpServer() {}
    ~MyTcpServer() {}

	virtual void onConnect(std::shared_ptr<Session> pSession) override {
        std::cout << "client " << pSession->address() << " is connect" << std::endl;
    }

	virtual void onClose(const std::string & address) override {
        std::cout << "client " << address << " is closed" << std::endl;
    }

	virtual void onMessage(const std::string &message, std::shared_ptr<Session> pSession) override {
        std::cout << "Recv client " << pSession->address() << " message:" << message << std::endl;
        pSession->async_send_msg(message);
    }

};

int main()
{
    MyTcpServer server;
    server.start(8000);
    
    return 0;
}