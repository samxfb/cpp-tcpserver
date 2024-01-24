
#include "tcpserver.h"
#include <iostream>
namespace cpptcp
{

TcpServer::TcpServer(uint16_t port) 
: acceptor_(ioctx_, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port))
{

}

TcpServer::~TcpServer()
{
	ioctx_.stop();
}

void TcpServer::start()
{
	accept();
	while (1) {
		try {
			ioctx_.run();
			break;
		} catch (const std::exception &e) {
			//std::cerr << "io_context run occur exception: " << e.what() << std::endl;
		}
	}
}

void TcpServer::destroy(std::shared_ptr<Session> pSession)
{
	sessions_.erase(pSession);
}

void TcpServer::accept()
{
	std::shared_ptr<Session> pSession = std::make_shared<Session>(*this, ioctx_);
	if (!pSession) {
		return;
	}	
	auto callback = [pSession, this](asio::error_code ec){
		if (!ec) {
			sessions_.insert(pSession);
			pSession->set_keepalive();
			onConnect(pSession);
			pSession->async_recv_msg();
		}
		accept();
	};

	acceptor_.async_accept(pSession->getSocket(), callback);
}

Session::Session(TcpServer &srv, asio::io_context &ioctx) 
: server_(srv), socket_(ioctx) 
{

}

Session::~Session()
{ 
	close(); 
}

void Session::close()
{
	if (socket_.is_open()) {
		asio::error_code ec;
		socket_.shutdown(asio::ip::tcp::socket::shutdown_both, ec);
		socket_.close(ec);
		server_.onClose(ip_);
	}
}

const std::string& Session::address()
{
	if (ip_.empty() && socket_.is_open()) {
		asio::error_code ec;
		auto endpoint = socket_.remote_endpoint(ec);
		if (!ec) {
			ip_ = endpoint.address().to_string();
		}	
	}
	return ip_;
}

asio::ip::tcp::socket& Session::getSocket()
{
	return socket_;
}

void Session::async_send_msg(const std::string &message)
{
	auto self(this->shared_from_this());
	auto callback = [self, this](asio::error_code ec, std::size_t n){
		if (!ec) {
			server_.onSend(self);
		} else {
			server_.destroy(self);
		}
	};
	asio::async_write(socket_, asio::buffer(message), callback);
}

void Session::Session::async_recv_msg()
{	
	std::shared_ptr<std::string> message = std::make_shared<std::string>();
	message->resize(kMaxMessageSize);
	auto self(this->shared_from_this());
	auto callback = [self, this, message](asio::error_code ec, std::size_t n){
		if (!ec) {
			message->resize(n);
			server_.onMessage(*message, self);
			async_recv_msg();
		} else {
			server_.destroy(self);
		}
	};

	asio::async_read(socket_, asio::buffer(*message), asio::transfer_at_least(1), callback);
}

void Session::set_keepalive()
{
#ifdef __linux__
	auto handle = socket_.native_handle();
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

}
