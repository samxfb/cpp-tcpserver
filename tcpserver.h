#pragma once

#include <string>
#include <set>
#include <memory>
#include "asio.hpp"
namespace cpptcp
{

class Session;
class TcpServer
{
public:
	TcpServer(uint16_t port);
	virtual ~TcpServer();

	TcpServer(const TcpServer&) = delete;
	TcpServer& operator=(const TcpServer&) = delete;
	
	//启动服务器,会阻塞
	void start();

	//销毁客户端session
	void destroy(std::shared_ptr<Session> pSession);

	//客户端连接之后调用
	virtual void onConnect(std::shared_ptr<Session> pSession) = 0;

	//客户端关闭之后调用
	virtual void onClose(const std::string& address) = 0;

	//读取到客户端数据之后调用
	virtual void onMessage(const std::string& message, std::shared_ptr<Session> pSession) = 0;
	
	//发送成功之后调用
	virtual void onSend(std::shared_ptr<Session> pSession) {}

private:
	//非阻塞接收客户端连接
	void accept();

private:
	asio::io_context ioctx_;
	asio::ip::tcp::acceptor acceptor_;
	std::set<std::shared_ptr<Session>> sessions_;
};

class Session : public std::enable_shared_from_this<Session>
{
public:
	Session(TcpServer &srv, asio::io_context &ioctx);

	~Session();

	// 关闭客户端会话
	void close();

	// 异步发送消息
	void async_send_msg(const std::string &msg);

	// 异步接收消息
	void async_recv_msg();

	// 获取客户端套接字
	asio::ip::tcp::socket& getSocket();

	// 获取客户端地址
	const std::string& address();

	// 设置keepalive心跳机制
	void set_keepalive();

private:
	const int kMaxMessageSize = 64 * 1024; // 64k

private:
	TcpServer &server_;
	asio::ip::tcp::socket socket_;
	std::string ip_;
};

}
