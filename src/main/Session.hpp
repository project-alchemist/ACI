#ifndef ACI__SESSION_HPP
#define ACI__SESSION_HPP


#include "ACI.hpp"
#include "Message.hpp"
#include "Parameters.hpp"
#include "utility/logging.hpp"


namespace alchemist {

using boost::asio::ip::tcp;
using std::string;

typedef uint16_t Session_ID;
typedef std::deque<Message> Message_queue;


class Session : public std::enable_shared_from_this<Session>
{
public:
	Session(tcp::socket);
	Session(tcp::socket, Session_ID);
	Session(tcp::socket, Session_ID, Log_ptr &);
	virtual ~Session() {}

	virtual void start() = 0;
	virtual void remove_session() = 0;
	virtual int handle_message() = 0;

	void set_log(Log_ptr _log);
	void set_ID(Session_ID _ID);

	void send_handshake();
	bool check_handshake();

	void send_test_string(string test_str);
	void request_test_string();
	void receive_response();

	void read_header();
	void read_body();
	void flush();


	Session_ID get_ID() const;
	string get_address() const;
	uint16_t get_port() const;

	void write_string(const string & data);
	void write_unsigned_char(const unsigned char & data);
	void write_uint16(const uint16_t & data);
	void write_uint32(const uint32_t & data);

	void write(const char * data, std::size_t length, datatype dt);

	Message & new_message();

	std::map<Worker_ID, WorkerInfo> alchemist_workers;

protected:
	Log_ptr log;


	tcp::socket socket;

	Message read_msg;
	Message write_msg;
	Message_queue write_msgs;

	Session_ID ID;
	string address;
	uint16_t port;
private:
	void set_address();
	void set_port();
};

typedef std::shared_ptr<Session> Session_ptr;

}			// namespace alchemist

#endif		// ALCHEMIST__SESSION_HPP
