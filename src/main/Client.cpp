#include "Client.hpp"
#include <boost/lexical_cast.hpp>

namespace alchemist {

Client::Client(io_context & _io_context) :
		ioc(_io_context), socket(_io_context), resolver(_io_context), connected(false)
{
//	socket = socket(io_context);
//	resolver = resolver(io_context);

	hostname = boost::asio::ip::host_name();

	address = boost::asio::ip::address().to_string();
//	port = socket.local_endpoint().port();


//	std::thread t([this](){ io_context.run(); });
//	connect();
//	std::cout << "Close" << std::endl;
//	close();
//
//	t.join();

//	tcp::resolver _resolver(io_context);
//	auto endpoints = _resolver.resolve("0.0.0.0", "24960");
//
////	boost::asio::async_connect(socket, endpoints, connect_handler);


//	connect();
//	display_menu();




//		boost::asio::async_connect(socket, endpoints,
//							[this](const boost::system::error_code & ec, const tcp::endpoint &) {
//			std::cout << "Connect " << ec << std::endl;
//	//							if (!ec) {
//	//								new_session();
//	//								display_menu();
//	//							}
//							});


//	 connect(endpoints);
}

Client::Client(io_context & _io_context, const tcp::endpoint & endpoint) :
		ioc(_io_context), socket(_io_context), resolver(_io_context), connected(false)
{
	socket.open(boost::asio::ip::tcp::v4());
	boost::system::error_code ec;
	socket.bind(endpoint, ec);

//	auto bb = socket.local_endpoint();

	hostname = boost::asio::ip::host_name();

	address = boost::asio::ip::address().to_string();
}

//Client::Client(boost::asio::io_context & _io_context) : io_context(_io_context),
//		socket(_io_context), resolver(_io_context), next_session_ID(1), connected(false)
//{
//
//	connect();
//
////	tcp::resolver _resolver(io_context);
////	auto endpoints = _resolver.resolve("0.0.0.0", "24960");
////
//////	boost::asio::async_connect(socket, endpoints, connect_handler);
//
//
////	connect();
////	display_menu();
//
//
//
//
////		boost::asio::async_connect(socket, endpoints,
////							[this](const boost::system::error_code & ec, const tcp::endpoint &) {
////			std::cout << "Connect " << ec << std::endl;
////	//							if (!ec) {
////	//								new_session();
////	//								display_menu();
////	//							}
////							});
//
//
////	 connect(endpoints);
//}
//
//
//Client::Client(boost::asio::io_context & _io_context, const tcp::resolver::results_type & endpoints) : io_context(io_context),
//		socket(_io_context), resolver(_io_context), next_session_ID(1), connected(false)
//{
//
//
//
////	start();
////	endpoints = resolver.resolve("0.0.0.0", "24960");
//	connect(endpoints);
//}

void Client::connect_handler(const boost::system::error_code & ec, const boost::asio::ip::tcp::endpoint &)
{
	std::cout << "Connect " << ec << std::endl;
}

//void Client::write(const Message & msg)
//{
//	boost::asio::post(io_context, [this, msg]() {
//		bool write_in_progress = !write_msgs.empty();
//		write_msgs.push_back(msg);
//		if (!write_in_progress) write();
//	});
//}

void Client::start()
{
//
//	char line[Message::max_body_length + 1];

//	connect();
//	display_menu();

//    char line[Message::max_body_length + 1];
//    std::cout << "Starting ... " << std::endl;
//    std::cin.getline(line, Message::max_body_length + 1);


//	std::cout << "Starting ..." << std::endl;
//	open_connection();
////
//	usleep(1000);
//
//	std::cout << "Go" << std::endl;

//	endpoints = resolver.resolve("0.0.0.0", "24960");
//			connect(endpoints);

//	while (keep_going)
//	{
//
//		std::cout << "Connected " << connected << std::endl;
//		keep_going = (connected) ? display_menu() : connect();
//		std::cout << "Enter message: ";
//		std::cin.getline(line, Message::max_body_length + 1);
//		if (strcmp(line, "exit") == 0)
//			keep_going = false;
//		else {
//			Message msg;
//			msg.body_length(std::strlen(line));
//			std::memcpy(msg.body(), line, msg.body_length());
////			msg.encode_header();
////			ac.write(msg);
//		}
//	}
}


void Client::open_connection()
{
	boost::asio::post(ioc, [this]() { connect(); });
	ioc.run();
	ioc.restart();
}

void Client::open_connection(string address, uint16_t port)
{
	boost::asio::post(ioc, [this]() { connect1(); });
	ioc.run();
	ioc.restart();
}

string Client::make_daytime_string()
{
	std::time_t now = std::time(0);
	return std::ctime(&now);
}

void Client::connect()
{
	get_endpoint();
	connect(address, port);
}

void Client::connect1()
{
	connect(address, port);
}

void Client::connect(string address, uint16_t port)
{
	string p = std::to_string(port);

	log->info("Connecting to {}:{}", address, p);

	tcp::resolver::query q(address.c_str(), p.c_str());
	resolver.async_resolve(q, [this](const boost::system::error_code & ec, tcp::resolver::results_type endpoints)
		{
			if (!ec) connect(endpoints);
			else log->info("ERROR: Unknown error while connecting to Alchemist (error code {})", ec.value());
		});
}

void Client::connect(const tcp::resolver::results_type & endpoints)
{
	boost::asio::async_connect(socket, endpoints,
						[this](const boost::system::error_code & ec, const boost::asio::ip::tcp::endpoint &) {
							if (!ec) {
								new_session();
								connected = true;
							}
							else {
								if (ec.value() == 89) {
									log->info("Retrying connection");
									usleep(1000);
									connect();
								}
								else if (ec.value() == 61) {
									log->info("ERROR: Invalid address for Alchemist");
								}
								else {
									log->info("ERROR: Unknown error while connecting to Alchemist (error code {})", ec.value());
								}
							}
						});
}

void Client::disconnect()
{
	list_connections();
	std::cout << "Disconnect the following (space-separated list)" << std::endl;
}

void Client::list_connections()
{
	std::cout << "Listing connections" << std::endl;
}

void Client::shut_down()
{
	std::cout << "shut_down" << std::endl;
}

void Client::close_client()
{
	log->info("Closing client");
}



//bool Client::connect(tcp::resolver::results_type endpoints)
//{
//	std::cout << "    Enter port 1:     " << std::endl;
//	boost::asio::async_connect(socket, endpoints, connect_handler);
//
//	std::cout << "conn " << connected << std::endl;
//	print_num_sessions();
//
//	return true;
//}
//
//void Client::connect_handler(const boost::system::error_code& ec, const tcp::endpoint& endpoint)
//{
//	std::cout << "    Enter port 2:     " << std::endl;
//						set_connected();
//						if (!ec) {
//							std::make_shared<Session>(std::move(socket), *this, next_session_ID++)->start();
//						}
////							else return false;
//}

void Client::set_connected()
{
	connected = true;
}

void Client::close()
{

//	boost::asio::post(io_context, [this]() { socket.close(); });
}


//
//void Client::read_header()
//{
//	boost::asio::async_read(socket,
//				boost::asio::buffer(read_msg.body(), Message::header_length),
//						[this](boost::system::error_code ec, std::size_t /*length*/) {
//							if (!ec && read_msg.decode_header()) read_body();
//							else {
//								std::cout << "Lost connection to Alchemist server 3" << std::endl;
//								socket.close();
//							}
//						});
//}
//
//void Client::read_body()
//{
//	boost::asio::async_read(socket, boost::asio::buffer(read_msg.body(), read_msg.body_length),
//						[this](boost::system::error_code ec, std::size_t /*length*/) {
//							if (!ec) {
//								display_message();
//								read_header();
//							}
//							else {
//								std::cout << "Lost connection to Alchemist server 2" << std::endl;
//								socket.close();
//							}
//						});
//}

void Client::display_message()
{

}

//void Client::write()
//{
//	boost::asio::async_write(socket, boost::asio::buffer(write_msgs.front().body(), write_msgs.front().body_length),
//						[this](boost::system::error_code ec, std::size_t /*length*/) {
//							if (!ec) {
//								write_msgs.pop_front();
//								if (!write_msgs.empty()) write();
//							}
//							else {
//								std::cout << "Lost connection to Alchemist server 1" << std::endl;
//								socket.close();
//							}
//						});
//}


void Client::print_IP()
{
	// Outdated
	int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int rv;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC; // use AF_INET6 to force IPv6
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // use my IP address

	if ((rv = getaddrinfo(NULL, "24960", &hints, &servinfo)) != 0) {
	    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
	    exit(1);
	}

	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		char hostname[NI_MAXHOST];
		getnameinfo(p->ai_addr, p->ai_addrlen, hostname, NI_MAXHOST, NULL, 0, 0);
		std::string ip(hostname);
		std::cout << "IP: " << hostname << std::endl;
	}

	freeaddrinfo(servinfo); // all done with this structure
}

// ===============================================================================================
// ===============================================================================================


} // namespace alchemist



