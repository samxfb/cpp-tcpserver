#pragma once

#include <string>
#include <set>
#include <functional>
#include <memory>
#include <queue>
#include <bitset>
#include <random>
#include "asio.hpp"

namespace cpptcp
{

class TcpServer;

class Session : public std::enable_shared_from_this<Session>
{
public:
	Session(TcpServer &srv, asio::io_context &ioctx) : server(srv), m_socket(ioctx) {}

	~Session() { close(); }

	//关闭客户端会话
	void close();

	//异步发送消息
	void async_send_msg(const std::string &msg);

	//异步接收消息
	void async_recv_msg();

	asio::ip::tcp::socket & getSocket() {return m_socket;}

	const std::string & address();

	//设置keepalive心跳机制
	void set_keepalive();

private:

private:
	TcpServer &server;
	asio::ip::tcp::socket m_socket;
	std::string ip;
};

class TcpServer
{
public:
	TcpServer() : m_acceptor(m_ioctx) {}

	virtual ~TcpServer() {m_ioctx.stop();}
	
	//启动服务器,会阻塞
	void start(uint16_t port);

	//销毁客户端session
	void destroy(std::shared_ptr<Session> pSession) {m_sessions.erase(pSession);}

	//客户端连接之后调用
	virtual void onConnect(std::shared_ptr<Session> pSession) = 0;
	//客户端关闭之后调用
	virtual void onClose(const std::string & address) = 0;
	//读取到客户端数据之后调用
	virtual void onMessage(const std::string &message, std::shared_ptr<Session> pSession) = 0;
	//发送成功之后调用
	virtual void onSend(std::shared_ptr<Session> pSession) {}

private:
	asio::io_context m_ioctx;
	asio::ip::tcp::acceptor m_acceptor;
	std::set<std::shared_ptr<Session>> m_sessions;

	void accept();	//非阻塞接收客户端连接
};

}

