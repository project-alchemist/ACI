// =============================================================================================
//                                         DriverSession
// =============================================================================================

#include "DriverSession.hpp"

namespace alchemist {

DriverSession::DriverSession(tcp::socket _socket, Driver & _driver) :
		Session(std::move(_socket)), admin_privilege(false), driver(_driver) { }

DriverSession::DriverSession(tcp::socket _socket, Driver & _driver, uint16_t _ID) :
		Session(std::move(_socket), _ID), admin_privilege(false), driver(_driver) { }

DriverSession::DriverSession(tcp::socket _socket, Driver & _driver, uint16_t _ID, Log_ptr & _log) :
		Session(std::move(_socket), _ID, _log), admin_privilege(false), driver(_driver) { }

void DriverSession::start()
{
	driver.add_session(shared_from_this());
	send_handshake();
}

void DriverSession::send_info(string & log_dir)
{
	write_msg.add_string(log_dir);
	flush();

	receive_response();
}

void DriverSession::remove_session()
{
	driver.remove_session();
}

void DriverSession::set_admin_privilege(bool privilege)
{
	admin_privilege = privilege;
}

bool DriverSession::get_admin_privilege() const
{
	return admin_privilege;
}

int DriverSession::handle_message()
{
//	log->info("Received message from Session {} at {}", get_ID(), get_address().c_str());
	log->info("{}", read_msg.to_string());

	client_command command = read_msg.cc;

	switch (command) {
		case SHUT_DOWN:
//			shut_down();
			break;
		case HANDSHAKE:
			if (check_handshake()) {
				log->info("Handshake successful");
				send_info();
			}
			else log->info("Handshake not successful");

			break;
		case CLIENT_INFO:
			log->info(read_msg.read_string());
			driver.print_menu();
			break;
		case REQUEST_TEST_STRING:
			log->info(read_msg.read_string());
			driver.print_menu();
			break;
		case SEND_TEST_STRING:
			log->info(read_msg.read_string());
			driver.print_menu();
			break;
		case REQUEST_WORKERS:
			read_alchemist_workers();
			driver.print_menu();
			break;
		case YIELD_WORKERS:
			log->info(read_msg.read_string());
			driver.print_menu();
			break;
		case SEND_ASSIGNED_WORKERS_INFO:
//			send_assigned_workers_info();
			driver.print_menu();
			break;
		case MATRIX_INFO:
			get_matrix_info();
			break;
		case LIST_ALL_WORKERS:
			driver.list_all_alchemist_workers(read_msg.read_string());
			driver.print_menu();
			break;
		case LIST_ACTIVE_WORKERS:
			driver.list_active_alchemist_workers(read_msg.read_string());
			driver.print_menu();
			break;
		case LIST_INACTIVE_WORKERS:
			driver.list_inactive_alchemist_workers(read_msg.read_string());
			driver.print_menu();
			break;
		case LIST_ASSIGNED_WORKERS:
			driver.list_assigned_alchemist_workers(read_msg.read_string());
			driver.print_menu();
			break;
		case LOAD_LIBRARY:
			driver.print_menu();
			break;
	}

	return 0;
}

void DriverSession::send_matrix_info(unsigned char type, unsigned char layout, uint32_t num_rows, uint32_t num_cols)
{
	log->info("Sending matrix info to Alchemist");

	write_msg.start(MATRIX_INFO);
	write_msg.add_unsigned_char(type);
	write_msg.add_unsigned_char(layout);
	write_msg.add_uint32(num_rows);
	write_msg.add_uint32(num_cols);
	flush();

	read_header();
}

void DriverSession::get_matrix_info()
{
	Matrix_ID matrix_ID = read_msg.read_uint16();

	log->info("Received matrix ID from Alchemist");

	driver.set_matrix_ID(matrix_ID);

	Worker_ID worker_ID;
	uint32_t num_rows = read_msg.read_uint32();
	uint32_t worker_num_rows, row;

	vector<uint16_t> alchemist_row_assignments = vector<uint16_t>(num_rows);

	for (uint32_t row = 0; row < num_rows; row++)
		alchemist_row_assignments[row] = read_msg.read_uint16();

	log->info("Received matrix row assignments from Alchemist");

	driver.set_matrix_row_assignments(matrix_ID, alchemist_row_assignments);
}

void DriverSession::send_info()
{
	log->info("Sending info to Alchemist");

	uint16_t num_workers = driver.get_num_workers();
	string log_dir = "lalalal";

	write_msg.start(CLIENT_INFO);
	write_msg.add_uint16(num_workers);
	write_msg.add_string(log_dir);
	flush();

	receive_response();
}

void DriverSession::load_library(string library_name, string library_path)
{
	log->info("Telling Alchemist to load library {} at {}", library_name, library_path);

	write_msg.start(LOAD_LIBRARY);
	write_msg.add_string(library_name);
	write_msg.add_string(library_path);
	flush();

	receive_response();

}

void DriverSession::extract_layout()
{
	// Get layout
	uint16_t num_alchemist_workers = read_msg.read_uint16();
	uint16_t num_client_workers = read_msg.read_uint16();

	vector<vector<vector<float> > > layout_rr = vector<vector<vector<float> > >(num_client_workers, vector<vector<float> >(num_alchemist_workers, vector<float>(2)));

	for (int i = 0; i < num_client_workers; i++) {
		for (int j = 0; j < num_alchemist_workers; j++) {
			layout_rr[i][j][0] = read_msg.read_float();
			layout_rr[i][j][1] = read_msg.read_float();
		}
	}

	driver.set_layout_matrix(layout_rr);
}

bool DriverSession::read_alchemist_workers()
{
	log->info("Reading in Alchemist workers");

	uint16_t num_workers = read_msg.read_uint16();

	for (uint16_t i = 0; i < num_workers; i++) {
		WorkerInfo alchemist_worker = WorkerInfo();

		alchemist_worker.id = read_msg.read_uint16();
		alchemist_worker.hostname = read_msg.read_string();
		alchemist_worker.address = read_msg.read_string();
		alchemist_worker.port = read_msg.read_uint16();

		driver.add_alchemist_worker(alchemist_worker);

		alchemist_workers.insert(std::make_pair(alchemist_worker.id, alchemist_worker));
	}

	driver.list_assigned_alchemist_workers();
	driver.connect_workers();

	return true;
}

void DriverSession::list_assigned_alchemist_workers()
{
	std::stringstream list_of_alchemist_workers;
	list_of_alchemist_workers << "List of assigned Alchemist workers:" << std::endl;

	uint16_t num_workers = alchemist_workers.size();

	if (num_workers == 0)
	{
		list_of_alchemist_workers << SPACE;
		list_of_alchemist_workers << "    No workers" << std::endl;
	}
	else
	{
		char buffer[4];

		for (auto it = alchemist_workers.begin(); it != alchemist_workers.end(); it++) {
			sprintf(buffer, "%03d", it->first);
			list_of_alchemist_workers << SPACE;
			list_of_alchemist_workers << "    Worker-" << string(buffer) << " running on " << it->second.hostname << " ";
			list_of_alchemist_workers << it->second.address << ":" << it->second.port << std::endl;
		}
	}

	log->info(list_of_alchemist_workers.str());
}

void DriverSession::send_test_strings_to_workers()
{
	driver.send_test_strings_to_workers();
}

void DriverSession::request_test_strings_from_workers()
{
	driver.request_test_strings_from_workers();
}

bool DriverSession::request_list_all_alchemist_workers()
{
	log->info("Requesting list of all Alchemist workers");

	write_msg.start(LIST_ALL_WORKERS);
	write_msg.add_string(" ");
	flush();

	read_header();

	return true;
}

bool DriverSession::request_list_active_alchemist_workers()
{
	log->info("Requesting list of active Alchemist workers");

	write_msg.start(LIST_ACTIVE_WORKERS);
	write_msg.add_string(" ");
	flush();

	read_header();

	return true;
}

bool DriverSession::request_list_inactive_alchemist_workers()
{
	log->info("Requesting list of inactive Alchemist workers");

	write_msg.start(LIST_INACTIVE_WORKERS);
	write_msg.add_string(" ");
	flush();

	read_header();

	return true;
}

bool DriverSession::request_list_assigned_alchemist_workers()
{
	log->info("Requesting list of assigned Alchemist workers");

	write_msg.start(LIST_ASSIGNED_WORKERS);
	write_msg.add_string(" ");
	flush();

	read_header();

	return true;
}

bool DriverSession::request_workers(uint16_t num_workers)
{
	if (num_workers == 1)
		log->info("Requesting 1 worker from Alchemist");
	else
		log->info("Requesting {} workers from Alchemist", num_workers);

	write_msg.start(REQUEST_WORKERS);
	write_msg.add_uint16(num_workers);
	flush();

	read_header();

	return true;
}

bool DriverSession::yield_workers()
{
	uint16_t num_workers;

	alchemist_workers.clear();

	write_msg.start(YIELD_WORKERS);
	write_msg.add_string(" ");
	flush();

	read_header();

	return true;
}

}   // namespace alchemist
