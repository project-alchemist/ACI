#include "Driver.hpp"

namespace alchemist {

// ===============================================================================================
// =======================================   CONSTRUCTOR   =======================================


Driver::Driver(MPI_Comm & _world, MPI_Comm & _peers, io_context & _io_context, const unsigned int port) :
		Driver(_world, _peers, _io_context, tcp::endpoint(tcp::v4(), port)) { }

Driver::Driver(MPI_Comm & _world, MPI_Comm & _peers, io_context & _io_context, const tcp::endpoint & endpoint) :
		Client(_io_context, endpoint), world(_world), peers(_peers), next_matrix_ID(1), workers_assigned(false),
		workers_connected(false), layout_ready(false)
{
	log = start_log("driver", "[%Y-%m-%d %H:%M:%S.%e] [%n] [%l]     %v");

	int world_size;
	MPI_Comm_size(world, &world_size);

	num_workers = world_size - 1;

	print_welcome_message();

	log->info("Starting workers");
	start_workers();

	print_ready_message();
	print_menu();
}

Driver::~Driver() { }

int Driver::start()
{
//	aci_command command = ACCEPT_CONNECTION;
//	MPI_Bcast(&command, 1, MPI_UNSIGNED_CHAR, 0, world);

	return 0;
}


void Driver::new_session()
{
	session = std::make_shared<DriverSession>(std::move(socket), *this, 0, log);
	session->start();
	send_info();
}

void Driver::send_info()
{
//	string log_dir = "hylt";
//	session->send_info(log_dir);
}

// ===============================================================================================
// ====================================   UTILITY FUNCTIONS   ====================================

// ----------------------------------------   Printing   -----------------------------------------

void Driver::print_welcome_message()
{
	string message = "==============================================\n";
	message += SPACE;
	message +=       "Starting Alchemist Client Interface {}\n";
	message += SPACE;
	message += "----------------------------------------------\n";
	message += SPACE;
	#ifndef ASIO_STANDALONE
	message += "Using Boost.Asio {}\n";
	message += SPACE;
	#endif
	message += "Running on {} {}";

	#ifndef ASIO_STANDALONE
	log->info(message.c_str(), get_ACI_version(), get_Boost_version(), hostname, address, port);
	#else
	log->info(message.c_str(), get_ACI_version(), hostname, address, port);
	#endif
}

void Driver::print_ready_message()
{
	string message = "==============================================\n";
	message += SPACE;
	message += "Alchemist Client Interface is ready\n";
	message += SPACE;
	message += "----------------------------------------------\n";
	log->info(message.c_str());
}

void Driver::connect_alchemist_workers(std::map<Worker_ID, WorkerInfo> _alchemist_workers)
{
	alchemist_workers = _alchemist_workers;
}

void Driver::print_menu()
{
	string choice;
	string dummy;

//	while (true) {
		if (!connected) {
			string message = "\n";
			message += SPACE;
			message += "Choose an option:\n";
			message += SPACE;
			message += "----------------------------------------------\n";

			message += SPACE;
			message += " (1) Connect to Alchemist\n";
			message += SPACE;
			message += " (2) Exit\n";

			log->info(message.c_str());
			usleep(10);
			std::cout << SPACE;
			std::cout << "Enter choice: ";
			std::cin >> choice;
			std::cin.ignore();

			std::stringstream cs(choice);

			int choice_int = 0;
			cs >> choice_int;

			switch (choice_int) {
				case 1:
					open_connection();
					break;
				case 2:
					close_client();
					break;
				default:
					log->info("Invalid choice");
					print_menu();
					break;
			}
		}
		else {
			workers_assigned = !(alchemist_workers.size() == 0);

			usleep(400);
			log->info("<Press 'return' to show menu>");

			std::getline(std::cin, dummy);

			string message = "Choose an option:\n";
			message += SPACE;
			message += "----------------------------------------------\n";

			message += SPACE;
			message += " (2) Disconnect from Alchemist\n";
			message += SPACE;
			message += " (3) List current connections\n";
			message += SPACE;
			message += "\n";
			message += SPACE;
			message += "(11) Handshake\n";
			message += SPACE;
			message += "(12) Send test string\n";
			message += SPACE;
			message += "(13) Request test string\n";
			if (workers_assigned)  {
				message += "\n";
				message += SPACE;
				message += "(14) Yield assigned Alchemist workers\n";
				message += SPACE;
				message += "(15) List assigned Alchemist workers\n";
			}
			else {
				message += "\n";
				message += SPACE;
				message += "(14) Request Alchemist workers\n";
			}
			if (workers_connected) {
				message += "\n";
				message += SPACE;
				message += "(16) Handshake with Alchemist workers\n";
				message += SPACE;
				message += "(17) Send test strings to Alchemist workers\n";
				message += SPACE;
				message += "(18) Request test strings from Alchemist workers\n";
			}
			if (workers_assigned)  {
				message += "\n";
				message += SPACE;
				message += "(21) Load library\n";
				message += "\n";
				message += SPACE;
				message += "(31) Send test matrix (Elemental)\n";
				message += SPACE;
				message += "(32) Send test matrix (ScaLAPACK)\n";
			}
			message += "\n";
			message += SPACE;
			message += "(41) List all Alchemist workers\n";
			message += SPACE;
			message += "(42) List active Alchemist workers\n";
			message += SPACE;
			message += "(43) List inactive Alchemist workers\n";
			message += SPACE;
			message += "\n";
			message += SPACE;
			message += "(90) Shut down Alchemist\n";
			message += SPACE;
			message += "(91) Exit\n";

			log->info(message.c_str());
			usleep(400);
			std::cout << SPACE;
			std::cout << "Enter choice: ";
			std::cin >> choice;
			std::cin.ignore();

			std::stringstream cs(choice);

			int choice_int = 0;
			cs >> choice_int;

			switch (choice_int) {
				case 2:
		//				disconnect();
					break;
				case 3:
		//				list_connections();
					break;
				case 11:
					send_handshake();
					break;
				case 12:
					send_test_string();
					break;
				case 13:
					request_test_string();
					break;
				case 14:
					if (workers_assigned) {
						session->yield_workers();
						workers_connected = false;
					}
					else {
						uint16_t num_workers;

						log->info(" ");
						usleep(100);
						std::cout << SPACE;
						std::cout << "Requested number of workers: ";
						std::cin >> num_workers;
						std::cin.ignore();

						session->request_workers(num_workers);
					}

					break;
				case 15:
					if (workers_assigned) {
						list_assigned_alchemist_workers();
						print_menu();
					}
					else
						invalid_menu_choice();
					break;
				case 16:
					if (workers_connected) {
						handshake_with_workers();
						print_menu();
					}
					else
						invalid_menu_choice();
					break;
				case 17:
					if (workers_connected) {
						send_test_strings_to_workers();
						print_menu();
					}
					else
						invalid_menu_choice();
					break;
				case 18:
					if (workers_connected) {
						request_test_strings_from_workers();
						print_menu();
					}
					else
						invalid_menu_choice();
					break;
				case 21:
					load_library();
					break;
				case 31:
					if (workers_connected) {
						send_Elemental_test_matrix();
					}
					else
						invalid_menu_choice();
					break;
				case 32:
					if (workers_connected) {
						send_ScaLAPACK_test_matrix();
						print_menu();
					}
					else
						invalid_menu_choice();
					break;
				case 41:
					session->request_list_all_alchemist_workers();
					break;
				case 42:
					session->request_list_active_alchemist_workers();
					break;
				case 43:
					session->request_list_inactive_alchemist_workers();
					break;
				case 90:
		//				shut_down();
		//				keep_going = false;
					break;
				case 91:
		//				close_client();
		//				keep_going = false;
					break;
				default:
					invalid_menu_choice();

					break;
				}
		}
//	}

}

void Driver::set_layout_matrix(vector<vector<vector<float> > > & _layout_rr)
{
	layout_rr = _layout_rr;
}

void Driver::print_layout_matrix()
{
	auto num_client_workers = layout_rr.size();
	auto num_alchemist_workers = layout_rr[0].size();

	std::stringstream ss;
	std::string space = "                                              ";

	ss << "Data layout matrix (row-major), ACI workers along rows, Alchemist workers along columns" << std::endl;
	ss << space << "    |";
	for (int j = 0; j < num_alchemist_workers; j++)
		ss << "   " << j << "  ";
	ss << std::endl;
	ss << space << "----|";
	for (int j = 0; j < num_alchemist_workers; j++)
		ss << "------";
	ss << std::endl;

	for (int i = 0; i < num_client_workers; i++) {
		ss << space << " " << i << "  |  ";
		for (int j = 0; j < num_alchemist_workers; j++)
			ss << layout_rr[i][j][0] << "," << layout_rr[i][j][1] << "  ";
		ss << std::endl;
	}

	log->info(ss.str());
}

void Driver::prepare_data_layout_table()
{
	auto num_alchemist_workers = alchemist_workers.size();
	auto num_client_workers = num_workers;

	auto data_ratio = float(num_alchemist_workers)/float(num_client_workers);

	layout_rr = vector<vector<vector<float> > >(num_client_workers, vector<vector<float> >(num_alchemist_workers, vector<float>(2)));

//	float mat[num_client_workers][num_alchemist_workers];

	for (int i = 0; i < num_client_workers; i++)
		for (int j = 0; j < num_alchemist_workers; j++) {
			layout_rr[i][j][0] = 0.0;
			layout_rr[i][j][1] = 0.0;
		}

	int j = 0;
	float diff, col_sum;

	for (int i = 0; i < num_client_workers; i++) {
		auto dr = data_ratio;
		for (; j < num_alchemist_workers; j++) {
			col_sum = 0.0;
			for (int k = 0; k < i; k++)
				col_sum += layout_rr[k][j][1];

			if (i > 0) diff = 1.0 - col_sum;
			else diff = 1.0;

			if (dr >= diff) {
				layout_rr[i][j][1] = layout_rr[i][j][0] + diff;
				dr -= diff;
			}
			else {
				layout_rr[i][j][1] = layout_rr[i][j][0] + dr;
				break;
			}
		}
	}

	for (int i = 0; i < num_client_workers; i++)
		for (int j = 0; j < num_alchemist_workers; j++) {
			layout_rr[i][j][0] /= data_ratio;
			layout_rr[i][j][1] /= data_ratio;
			if (j > 0) {
				layout_rr[i][j][0] += layout_rr[i][j-1][1];
				layout_rr[i][j][1] += layout_rr[i][j-1][1];
				if (layout_rr[i][j][1] >= 0.99) break;
			}
		}
}

uint16_t Driver::get_num_workers()
{
	return num_workers;
}

void Driver::load_library()
{
	string library_name;
	string library_path;
//
//	std::cout << SPACE;
//	std::cout << "Enter library name: ";
//	std::cin >> lib_name;
//	std::cout << SPACE;
//	std::cout << "Enter library location: ";\
//	std::cin >> lib_location;

	library_name = "AlLib";
	library_path = "/Users/kai/Projects/AlLib/target/allib.dylib";

	session->load_library(library_name, library_path);
}

void Driver::set_matrix_ID(Matrix_ID & matrix_ID)
{
	matrices.insert(std::make_pair(matrix_ID, MatrixInfo(matrix_ID, current_num_rows, current_num_cols)));

	aci_command command = SET_MATRIX_ID;

	MPI_Request req;
	MPI_Status status;
	MPI_Ibcast(&command, 1, MPI_UNSIGNED_CHAR, 0, world, &req);
	MPI_Wait(&req, &status);

	MPI_Bcast(&matrix_ID, 1, MPI_UNSIGNED_SHORT, 0, world);

	MPI_Barrier(world);
}

void Driver::set_matrix_layout(vector<vector<uint32_t> > & _rows_on_alchemist_workers)
{
	rows_on_alchemist_workers = _rows_on_alchemist_workers;
	layout_ready = true;
}

void Driver::set_matrix_row_assignments(Matrix_ID & _matrix_ID, vector<uint16_t> & _alchemist_row_assignments)
{
	alchemist_row_assignments = _alchemist_row_assignments;

	prepare_layout_lists();
	send_matrix_data();
}

void Driver::prepare_layout_lists()
{
	log->info("Preparing matrix layout");

	uint32_t num_rows = client_row_assignments.size();

	auto num_alchemist_workers = alchemist_workers.size();
	auto num_client_workers = num_workers;

	row_layout = vector<vector<vector<uint32_t>>>(num_client_workers, vector<vector<uint32_t>>(num_alchemist_workers, vector<uint32_t>()));

	for (uint32_t row = 0; row < num_rows; row++)
		row_layout[client_row_assignments[row]][alchemist_row_assignments[row]].push_back(row);

//	std::stringstream ss1;
//
//	for (uint16_t i = 0; i < num_client_workers; i++) {
//		ss1 << std::endl << i << " | ";
//		for (uint16_t j = 0; j < num_alchemist_workers; j++) {
//			for (uint32_t k = 0; k < row_layout[i][j].size(); k++)
//				ss1 << row_layout[i][j][k] << "  ";
//			ss1 << " | ";
//		}
//		ss1 << std::endl;
//	}
//	log->info(ss1.str());
}

void Driver::send_Elemental_test_matrix()
{
	uint32_t num_rows, num_cols;
	uint32_t worker_num_cols, last_row_index = 0;

	get_matrix_dimensions(num_rows, num_cols);

	client_row_assignments = vector<Worker_ID>(num_rows);
	rows_on_client_workers = vector<vector<uint32_t> >(num_workers, vector<uint32_t>());
	Worker_ID worker = 0;
	uint32_t num_rows_remaining = num_rows;

	uint32_t worker_num_rows = std::ceil(num_rows/num_workers);

	for (uint32_t row = 0; row < num_rows; row++) {
		client_row_assignments[row] = worker;
		rows_on_client_workers[worker].push_back(row);
		worker_num_rows--;
		if (worker_num_rows == 0) worker_num_rows = std::ceil((num_rows - row)/(num_workers-++worker));
	}

	worker_num_cols = num_cols;

	uint32_t dims[2];

	aci_command command = NEW_RANDOM_MATRIX;

	MPI_Request req;
	MPI_Status status;
	MPI_Ibcast(&command, 1, MPI_UNSIGNED_CHAR, 0, world, &req);
	MPI_Wait(&req, &status);

	for (uint16_t id = 1; id <= num_workers; id++) {
		dims[0] = rows_on_client_workers[id-1].size();
		dims[1] = worker_num_cols;
		MPI_Send(&dims, 2, MPI_INTEGER4, id, 0, world);

		MPI_Send(&rows_on_client_workers[id-1][0], dims[0], MPI_INTEGER4, id, 0, world);

		for (uint32_t j = 0; j < dims[1]; j++)
			MPI_Send(&j, 1, MPI_INTEGER4, id, 0, world);
	}

	MPI_Barrier(world);

//	std::stringstream ss;
//
//	ss << std::endl << "Row | Worker " << std::endl;
//	for (uint32_t row = 0; row < num_rows; row++)
//		ss << row << " | " << client_row_assignments[row] << std::endl;
//	log->info(ss.str());

	session->send_matrix_info(0, 0, num_rows, num_cols);
}

void Driver::send_matrix_data()
{
	uint32_t num_rows, max_num_rows = 0;

	aci_command command = SEND_MATRIX_DATA;

	MPI_Request req;
	MPI_Status status;
	MPI_Ibcast(&command, 1, MPI_UNSIGNED_CHAR, 0, world, &req);
	MPI_Wait(&req, &status);

	auto num_alchemist_workers = alchemist_workers.size();

	for (uint16_t id = 1; id <= num_workers; id++) {
		MPI_Send(&num_alchemist_workers, 1, MPI_INTEGER2, id, 0, world);
		for (uint16_t j = 0; j < num_alchemist_workers; j++) {
			num_rows = row_layout[id-1][j].size();
			if (max_num_rows < num_rows) max_num_rows = num_rows;
		}
		MPI_Send(&max_num_rows, 1, MPI_INTEGER4, id, 0, world);
		for (uint16_t j = 0; j < num_alchemist_workers; j++) {
			num_rows = row_layout[id-1][j].size();
			MPI_Send(&j, 1, MPI_INTEGER2, id, 0, world);
			MPI_Send(&num_rows, 1, MPI_INTEGER4, id, 0, world);
			MPI_Send(&row_layout[id-1][j][0], num_rows, MPI_INTEGER4, id, 0, world);
		}
	}

	MPI_Barrier(world);

	print_menu();
}

void Driver::send_layout_to_workers()
{
	auto num_client_workers = layout_rr.size();
	auto num_alchemist_workers = layout_rr[0].size();

	aci_command command = ALCHEMIST_MATRIX_LAYOUT;

	MPI_Request req;
	MPI_Status status;
	MPI_Ibcast(&command, 1, MPI_UNSIGNED_CHAR, 0, world, &req);
	MPI_Wait(&req, &status);
	MPI_Bcast(&num_alchemist_workers, 1, MPI_UNSIGNED_SHORT, 0, world);

	float temp[2];

	for (auto i = 1; i <= num_client_workers; i++)
		for (auto j = 0; j < num_alchemist_workers; j++) {
			temp[0] = layout_rr[i-1][j][0];
			temp[1] = layout_rr[i-1][j][1];
			MPI_Send(&temp, 2, MPI_FLOAT, i, 0, world);
		}

	MPI_Barrier(world);
}

void Driver::get_matrix_dimensions(uint32_t & num_rows, uint32_t & num_cols)
{
	num_rows = 20;
	num_cols = 10;
}

void Driver::send_ScaLAPACK_test_matrix()
{
	auto num_alchemist_workers = alchemist_workers.size();

	aci_command command = ALCHEMIST_MATRIX_LAYOUT;

	MPI_Request req;
	MPI_Status status;
	MPI_Ibcast(&command, 1, MPI_UNSIGNED_CHAR, 0, world, &req);
	MPI_Wait(&req, &status);
	MPI_Bcast(&num_alchemist_workers, 1, MPI_UNSIGNED_SHORT, 0, world);

	float temp[2];

	for (auto i = 1; i <= num_workers; i++)
		for (auto j = 0; j < num_alchemist_workers; j++) {
			temp[0] = layout_rr[i-1][j][0];
			temp[1] = layout_rr[i-1][j][1];
			MPI_Send(&temp, 2, MPI_FLOAT, i, 0, world);
		}

	MPI_Barrier(world);
}

void Driver::invalid_menu_choice()
{
	log->info("Invalid menu choice");
	print_menu();
}

void Driver::send_handshake()
{
	session->send_handshake();
}

void Driver::send_test_string()
{
	session->send_test_string("This is a test string from ACI driver");
}

void Driver::request_test_string()
{
	session->request_test_string();
}

// -----------------------------------------   Workers   -----------------------------------------


void Driver::get_endpoint()
{
	std::ifstream in("connection.info");

	if(!in) log->info("Cannot open 'connection.info' file");
	else {
		in >> address;
		in >> port;

		in.close();
	}

//	address = "0.0.0.0";
//	port = 24960;

	//	log->info(" ");
	//	std::cout << SPACE;
	//	std::cout << "Enter Alchemist address: ";
	//	std::cin >> address;
	//	std::cout << SPACE;
	//	std::cout << "Enter Alchemist port:    ";
	//	std::cin >> port;
}



void Driver::add_alchemist_worker(WorkerInfo aw)
{
	alchemist_workers.insert(std::make_pair(aw.id, aw));
}

void Driver::start_workers()
{
	aci_command command = START;

	log->info("Sending command {} to ACI workers", get_command_name(command));

	MPI_Request req;
	MPI_Status status;
	MPI_Ibcast(&command, 1, MPI_UNSIGNED_CHAR, 0, world, &req);
	MPI_Wait(&req, &status);

	MPI_Barrier(world);
}

void Driver::connect_workers()
{
	prepare_data_layout_table();

	aci_command command = CONNECT_TO_ALCHEMIST;

	log->info("Sending command {} to ACI workers", get_command_name(command));

	std::string address, hostname;
	uint16_t port, al, hl;
	int num_alchemist_workers = alchemist_workers.size();

	MPI_Request req;
	MPI_Status status;
	MPI_Ibcast(&command, 1, MPI_UNSIGNED_CHAR, 0, world, &req);
	MPI_Wait(&req, &status);

	log->info("Connecting workers");

	// Send Alchemist worker info to all workers
	MPI_Bcast(&num_alchemist_workers, 1, MPI_UNSIGNED_SHORT, 0, world);
	for(auto it = alchemist_workers.begin(); it != alchemist_workers.end(); it++) {
		hostname = alchemist_workers[it->first].hostname;
		hl = hostname.length()+1;
		address = alchemist_workers[it->first].address;
		al = address.length()+1;
		port = it->second.port;

		MPI_Bcast(&hl, 1, MPI_UNSIGNED_SHORT, 0, world);
		MPI_Bcast((void*) hostname.c_str(), hl, MPI_CHAR, 0, world);
		MPI_Bcast(&al, 1, MPI_UNSIGNED_SHORT, 0, world);
		MPI_Bcast((void*) address.c_str(), al, MPI_CHAR, 0, world);
		MPI_Bcast(&port, 1, MPI_UNSIGNED_SHORT, 0, world);
	}

	MPI_Barrier(world);

	workers_connected = true;
}

void Driver::handshake_with_workers()
{
	aci_command command = HANDSHAKES;

	log->info("Sending command {} to ACI workers", get_command_name(command));

	MPI_Request req;
	MPI_Status status;
	MPI_Ibcast(&command, 1, MPI_UNSIGNED_CHAR, 0, world, &req);
	MPI_Wait(&req, &status);

	MPI_Barrier(world);
}

void Driver::send_test_strings_to_workers()
{
	aci_command command = SEND_TEST_STRINGS;

	log->info("Sending command {} to ACI workers", get_command_name(command));

	MPI_Request req;
	MPI_Status status;
	MPI_Ibcast(&command, 1, MPI_UNSIGNED_CHAR, 0, world, &req);
	MPI_Wait(&req, &status);

	MPI_Barrier(world);
}

void Driver::request_test_strings_from_workers()
{
	aci_command command = REQUEST_TEST_STRINGS;

	log->info("Sending command {} to ACI workers", get_command_name(command));

	MPI_Request req;
	MPI_Status status;
	MPI_Ibcast(&command, 1, MPI_UNSIGNED_CHAR, 0, world, &req);
	MPI_Wait(&req, &status);

	MPI_Barrier(world);
}

void Driver::list_all_alchemist_workers(string list)
{
	log->info(list);
}

void Driver::list_active_alchemist_workers(string list)
{
	log->info(list);
}

void Driver::list_inactive_alchemist_workers(string list)
{
	log->info(list);
}

void Driver::list_assigned_alchemist_workers(string list)
{
	log->info(list);
}

void Driver::list_assigned_alchemist_workers()
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

void Driver::print_workers()
{
	char buffer[4];

	for (Worker_ID id = 1; id <= num_workers; ++id) {
		sprintf(buffer, "%03d", id);
		log->info("    Worker {} running on {} {}:{}", string(buffer), workers[id].hostname, workers[id].address, workers[id].port);
	}

}

// ----------------------------------------   File I/O   ----------------------------------------

int Driver::read_HDF5() {

	log->info("Driver::read_HDF5 not yet implemented");



	return 0;
}

// ===============================================================================================
// ====================================   COMMAND FUNCTIONS   ====================================

int Driver::receive_test_string(const DriverSession_ptr session, const char * data, const uint32_t length) {

	string test_string(data, length);

	log->info("Test string from Session {} at {}", session->get_ID(), session->get_address());
	log->info(test_string);

	return 0;
}

int Driver::send_test_string(DriverSession_ptr session) {

	session->send_test_string("This is a test string from Alchemist driver");

	return 0;
}

int Driver::shut_down()
{
	log->info("Shutting down");

	return 0;
}

int Driver::assign_workers()
{


	send_assigned_workers_info();

	return 0;
}

int Driver::send_assigned_workers_info()
{
//	log->info("    Sending worker hostnames and ports to Session {}");
//	auto num_workers = world.size() - 1;
//	output.write_int(num_workers);
//
//	for(auto id = 0; id < num_workers; ++id) {
//		output.write_string(workers[id].hostname);
//		output.write_int(workers[id].port);
//	}
//
//	output.flush();

	return 0;
}

int Driver::handle_command(int command) {


	return 0;
}

int Driver::run_task() {

//	string args = input.read_string();
//
//	MPI_Bcast(args.c_str(), args.length(), MPI_CHAR, 0, world);
//	boost::mpi::broadcast(world, args, 0);
//
//	Parameters output_parameters = Parameters();
//
//	int status = Executor::run_task(args, output_parameters);
//
//	log->info("    Output: {}", output_parameters.to_string());
//
//	if (status != 0) {
//		output.write_int(0x0);
//		return 1;
//	}
//
//	output.write_int(0x1);
//	output.write_string(output_parameters.to_string());

	return 0;
}

// ---------------------------------------   Information   ---------------------------------------

void Driver::add_session(DriverSession_ptr _session)
{
//	session = _session;

	connected = true;
}

void Driver::remove_session()
{
	session.reset();

	connected = false;
}

// ----------------------------------------   Parameters   ---------------------------------------


int Driver::process_input_parameters(Parameters & input_parameters) {


	return 0;
}


int Driver::process_output_parameters(Parameters & output_parameters) {



	return 0;
}

// -----------------------------------------   Testing   -----------------------------------------



// ----------------------------------------   Matrices   -----------------------------------------


//MatrixHandle Driver::register_matrix(size_t num_rows, size_t num_cols) {
//
//	MatrixHandle handle{next_matrix_ID++};
//
//	return handle;
//}


int Driver::receive_new_matrix() {


	return 0;
}


int Driver::get_matrix_dimensions() {


	return 0;
}


int Driver::get_transpose() {


	return 0;
}


int Driver::matrix_multiply() {


	return 0;
}


int Driver::get_matrix_rows() {



	return 0;
}

// ===============================================================================================
// ===============================================================================================

} // namespace alchemist



