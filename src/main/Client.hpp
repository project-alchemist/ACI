#ifndef ACI__CLIENT_HPP
#define ACI__CLIENT_HPP


#include "ACI.hpp"

namespace alchemist {

using boost::asio::ip::tcp;


class Client : public std::enable_shared_from_this<Client>
{
public:
	Client(io_context & _io_context);
	Client(io_context & _io_context, const tcp::endpoint & endpoint);
	virtual ~Client() { }

//	void deliver(const Session_ptr session, const Message & msg);

//	void write(const Message & msg);

	void start();

	void close();

	std::map<Worker_ID, WorkerInfo> alchemist_workers;

protected:
	string hostname = "";
	string address = "";
	uint16_t port = 0;

	bool connected;

	virtual void get_endpoint() = 0;

	string make_daytime_string();

//	static _io_context io_context;

	io_context & ioc;
	Log_ptr log;

	tcp::resolver::results_type endpoints;
	tcp::socket socket;
	tcp::resolver resolver;

	virtual void new_session() = 0;

	void open_connection();
	void open_connection(string address, uint16_t port);
	void connect();
	void connect1();
	void connect(string address, uint16_t port);
	void connect(const tcp::resolver::results_type & endpoints);

	static void connect_handler(const boost::system::error_code & ec, const tcp::endpoint & endpoint);

	void set_connected();

//	void read_header();
//	void read_body();
	void display_message();
//	void write();

	void set_log(Log_ptr _log);
	int get_num_sessions();

	void disconnect();
	void list_connections();

	void shut_down();
	void close_client();

	void print_IP();
};

typedef std::shared_ptr<Client> Client_ptr;

}

#endif
