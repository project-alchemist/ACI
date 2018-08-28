#ifndef ACI__DRIVERSESSION_HPP
#define ACI__DRIVERSESSION_HPP


#include "ACI.hpp"
#include "Message.hpp"
//#include "data_stream.hpp"
#include "Session.hpp"
#include "Driver.hpp"


namespace alchemist {

using boost::asio::ip::tcp;
using std::string;

typedef uint16_t Session_ID;
typedef std::deque<Message> Message_queue;

class Driver;

class DriverSession : public Session
{
public:
	DriverSession(tcp::socket, Driver &);
	DriverSession(tcp::socket, Driver &, uint16_t);
	DriverSession(tcp::socket _socket, Driver & _driver, uint16_t _ID, Log_ptr & _log);

	std::map<Worker_ID, WorkerInfo> workers;

	void start();

	void remove_session();

	void load_library(string lib_name, string lib_location);
	void send_info(string & log_dir);

	void send_test_strings_to_workers();
	void request_test_strings_from_workers();

	void list_assigned_alchemist_workers();

	bool request_list_all_alchemist_workers();
	bool request_list_active_alchemist_workers();
	bool request_list_inactive_alchemist_workers();
	bool request_list_assigned_alchemist_workers();

	bool read_alchemist_workers();
	bool request_workers(uint16_t num_workers);
	bool yield_workers();

	void send_info();

	void list_alchemist_workers();

	void extract_layout();

	void set_admin_privilege(bool privilege);
	bool get_admin_privilege() const;

	int handle_message();

	void get_matrix_info();
	void send_matrix_info(unsigned char type, unsigned char layout, uint32_t num_rows, uint32_t num_cols);

	std::shared_ptr<DriverSession> shared_from_this()
	{
		return std::static_pointer_cast<DriverSession>(Session::shared_from_this());
	}
private:

	bool admin_privilege;

	Driver & driver;
};

typedef std::shared_ptr<DriverSession> DriverSession_ptr;

}			// namespace alchemist

#endif		// ACI__DRIVERSESSION_HPP
