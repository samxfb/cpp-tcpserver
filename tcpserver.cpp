
#include "tcpserver.h"
#include <functional>
#include <iostream>

namespace cpptcp
{

void Session::close()
{
	if (m_socket.is_open()) {
		asio::error_code ec;
		m_socket.shutdown(asio::ip::tcp::socket::shutdown_both, ec);
		m_socket.close(ec);
		server.onClose(ip);
	}
}

const std::string & Session::address()
{
	if (ip.empty() && m_socket.is_open()) {
		asio::error_code ec;
		auto endpoint = m_socket.remote_endpoint(ec);
		if (!ec)
			ip = std::move(endpoint.address().to_string());
	}
	return ip;
}

void Session::async_send_msg(const std::string &message)
{
	auto self(this->shared_from_this());
	auto callback = [self, this](asio::error_code ec, std::size_t n){
		if (!ec) {
			server.onSend(self);
		} else {
			server.destroy(self);
		}
	};
	asio::async_write(m_socket, asio::buffer(message), callback);
}

void Session::Session::async_recv_msg()
{	
	std::shared_ptr<std::string> message = std::make_shared<std::string>();
	message->resize(64 * 1024);
	auto self(this->shared_from_this());
	auto callback = [self, this, message](asio::error_code ec, std::size_t n){
		if (!ec) {
			message->resize(n);
			server.onMessage(*message, self);
			async_recv_msg();
		} else {
			server.destroy(self);
		}
	};

	asio::async_read(m_socket, asio::buffer(*message), asio::transfer_at_least(1), callback);
}

void Session::set_keepalive()
{
#ifdef __linux__
	auto handle = m_socket.native_handle();
	int keepAlive = 1; 		// 开启keepalive属性
	int keepIdle = 60;		// 如该连接在60秒内没有任何数据往来,则进行探测 
	int keepInterval = 3; 	//探测时发包的时间间隔为3秒
	int keepCount = 3; 		//探测尝试的次数.如果第1次探测包就收到响应了,则后2次的不再发.
	setsockopt(handle, SOL_SOCKET, SO_KEEPALIVE, (void *)&keepAlive, sizeof(keepAlive));
	setsockopt(handle, SOL_TCP, TCP_KEEPIDLE, (void*)&keepIdle, sizeof(keepIdle));
	setsockopt(handle, SOL_TCP, TCP_KEEPINTVL, (void *)&keepInterval, sizeof(keepInterval));
	setsockopt(handle, SOL_TCP, TCP_KEEPCNT, (void *)&keepCount, sizeof(keepCount));
#endif
}

void TcpServer::start(uint16_t port)
{
	m_acceptor = asio::ip::tcp::acceptor(m_ioctx, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port));
	accept();
	while (1) {
		try {
			m_ioctx.run();
			break;
		} catch (const std::exception &e) {
			//std::cerr << "io_context run occur exception: " << e.what() << std::endl;
		}
	}
}

void TcpServer::accept()
{
	std::shared_ptr<Session> pSession = std::make_shared<Session>(*this, m_ioctx);
	if (!pSession)
		return;
	auto callback = [pSession, this](asio::error_code ec){
		if (!ec) {
			m_sessions.insert(pSession);
			pSession->set_keepalive();
			onConnect(pSession);
			pSession->async_recv_msg();
		}
		accept();
	};

	m_acceptor.async_accept(pSession->getSocket(), callback);
}

}
