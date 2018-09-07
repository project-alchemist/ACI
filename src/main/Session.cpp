#include "Session.hpp"

namespace alchemist {

Session::Session(tcp::socket _socket)
    : socket(std::move(_socket)), ID(0)
{
	set_address();
}

Session::Session(tcp::socket _socket, uint16_t _ID)
    : socket(std::move(_socket)), ID(_ID)
{
	set_address();
}

Session::Session(tcp::socket _socket, uint16_t _ID, Log_ptr & _log)
    : socket(std::move(_socket)), ID(_ID), log(_log)
{
	set_address();
}

void Session::set_address()
{
	address = socket.remote_endpoint().address().to_string();
}

void Session::set_port()
{
	port = socket.remote_endpoint().port();
}

void Session::set_log(Log_ptr _log)
{
	log = _log;
}

void Session::set_ID(Session_ID _ID)
{
	ID = _ID;
}

Session_ID Session::get_ID() const
{
	return ID;
}

string Session::get_address() const
{
	return address;
}

uint16_t Session::get_port() const
{
	return port;
}

bool Session::check_handshake()
{
	if (read_msg.next_datatype() == UINT16_T && read_msg.next_data_length() == 1 && read_msg.read_uint16() == 4321) {
		if (read_msg.next_datatype() == STRING && read_msg.next_data_length() == 4 && read_msg.read_string().compare("DCBA") == 0) {
			return true;
		}
	}

	return false;
}

void Session::send_handshake()
{
	write_msg.start(HANDSHAKE);
	write_msg.add_uint16(1234);
	write_msg.add_string("ABCD");
	flush();

	receive_response();
}

Message & Session::new_message()
{
	write_msg.clear();

	return write_msg;
}

void Session::request_test_string()
{
	log->info("Requesting test string from Alchemist");

	write_msg.start(REQUEST_TEST_STRING);
	write_msg.add_string(" ");
	flush();

	receive_response();
}

void Session::send_test_string(string test_str)
{
	log->info("Sending test string to Alchemist");

	write_msg.start(SEND_TEST_STRING);
	write_msg.add_string(test_str);
	flush();

	receive_response();
}

void Session::receive_response()
{
	read_header();
}

void Session::write_string(const string & data)
{
	write_msg.add_string(data);
}

void Session::write_unsigned_char(const unsigned char & data)
{
	write_msg.add_unsigned_char(data);
}

void Session::write_uint16(const uint16_t & data)
{
	write_msg.add_uint16(data);
}

void Session::write_uint32(const uint32_t & data)
{
	write_msg.add_uint32(data);
}

void Session::write(const char * data, std::size_t length, datatype dt)
{
	write_msg.add(data, length, dt);
}

void Session::read_header()
{
	read_msg.clear();
	auto self(shared_from_this());
	asio::async_read(socket,
					asio::buffer(read_msg.header(), Message::header_length),
					[this, self](error_code ec, std::size_t) {
						if (!ec && read_msg.decode_header()) read_body();
						else remove_session();
					});
}

void Session::read_body()
{
	auto self(shared_from_this());
	asio::async_read(socket,
					asio::buffer(read_msg.body(), read_msg.body_length),
					[this, self](error_code ec, std::size_t) {
						if (!ec) handle_message();
						else remove_session();
					});
}

void Session::flush()
{
//	log->info("{}", write_msg.to_string());
	auto self(shared_from_this());
	asio::async_write(socket,
					asio::buffer(write_msg.data, write_msg.length()),
					[this, self](error_code ec, std::size_t) {
						if (!ec) write_msg.clear();
						else remove_session();
					});
}


} // namespace alchemist
