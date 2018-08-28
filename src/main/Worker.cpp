#include "Worker.hpp"

namespace alchemist {

Worker::Worker(MPI_Comm & _world, MPI_Comm & _peers, io_context & _io_context, const unsigned int port) :
		Worker(_world, _peers, _io_context, tcp::endpoint(tcp::v4(), port)) { }

Worker::Worker(MPI_Comm & _world, MPI_Comm & _peers, io_context & _io_context, const tcp::endpoint & endpoint) :
		Client(_io_context, endpoint), world(_world), peers(_peers), next_session_ID(1), current_alchemist_worker_ID(0),
		sessions_idle(0), num_alchemist_workers(0)
{
	int world_rank;
	MPI_Comm_rank(world, &world_rank);
	ID = world_rank;

	char buffer[12];
	sprintf(buffer, "worker-%03d", ID);

	log = start_log(string(buffer));

	wait_for_command();
}

Worker::~Worker() { }


int Worker::start()
{
	string message = "Worker ready\n";
	message += SPACE;
	message += "Running on {} {}";
	log->info(message.c_str(), hostname, address);

	MPI_Barrier(world);

	return 0;
}


void Worker::get_endpoint()
{
//	address = "0.0.0.0";
//	port = 24961;
}

// ===============================================================================================
// ====================================   UTILITY FUNCTIONS   ====================================

// ----------------------------------------   File I/O   ----------------------------------------

int Worker::read_HDF5() {

	log->info("Worker::read_HDF5 not yet implemented");

	return 0;
}

// ===============================================================================================
// ====================================   COMMAND FUNCTIONS   ====================================


void Worker::new_session()
{

//	WorkerSession_ptr session = std::make_shared<WorkerSession>(std::move(socket), *this, next_session_ID, log);
	ses = std::make_shared<WorkerSession>(std::move(socket), *this, current_alchemist_worker_ID, log);
	sessions.insert(std::make_pair(current_alchemist_worker_ID, ses));
	ses->start();
//	usleep(1000);
//	ses->start();
//	sessions[current_alchemist_worker_ID]->start();

//	std::make_shared<WorkerSession>(std::move(socket), *this, next_session_ID++, log)->start();
}

//void Worker::connect()
//{
//	string p = std::to_string(port);
//
//	log->info("Connecting to Alchemist at {}:{}", address, p);
//
//	tcp::resolver::query q(address.c_str(), p.c_str());
//	resolver.async_resolve(q, [this](const boost::system::error_code & ec, tcp::resolver::results_type endpoints)
//		{
//			if (!ec) Client::connect(endpoints);
//			else log->info("ERROR: Unknown error while connecting to Alchemist (error code {})", ec.value());
//		});
//}


//void Worker::connect(const tcp::resolver::results_type & endpoints)
//{
//	boost::asio::async_connect(socket, endpoints,
//						[this](const boost::system::error_code & ec, const tcp::endpoint &) {
//							if (!ec) {
//								new_session();
//								connected = true;
//							}
//							else {
//								if (ec.value() == 89) {
//									log->info("Retrying connection");
//									usleep(1000);
//									retry_connect();
//								}
//								else log->info("Unknown error while connecting");
//							}
//						});
//}
//
//void Worker::retry_connect()
//{
//	connect(endpoints);
//
//}

void Worker::add_session(WorkerSession_ptr session)
{
//	sessions.insert(std::make_pair(current_alchemist_worker_ID, session));
	connected = true;
}

void Worker::print_num_sessions()
{
	if (sessions.size() == 0)
		log->info("No active sessions");
	else if (sessions.size() == 1)
		log->info("1 active session");
	else
		log->info("{} active sessions", sessions.size());
}

void Worker::remove_session(WorkerSession_ptr session)
{
	Session_ID session_ID = session->get_ID();

	log->info("Session {} at {} has been removed", session->get_ID(), session->get_address().c_str());
	sessions.erase(session_ID);

	if (sessions.size() == 0) connected = false;

	print_num_sessions();
}

int Worker::wait_for_command()
{
	aci_command c;

	bool should_exit = false;
	int flag = 0;
	MPI_Request req = MPI_REQUEST_NULL;
	MPI_Status status;

	while (!should_exit) {

		MPI_Ibcast(&c, 1, MPI_UNSIGNED_CHAR, 0, world, &req);
		while (flag == 0) {
			MPI_Test(&req, &flag, &status);
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}

		handle_command(c);

		flag = 0;
		c = IDLE;
	}

	return 0;
}

void Worker::handle_command(aci_command c)
{
	switch (c) {
		case IDLE:
			break;
		case START:
			start();
			break;
		case SEND_INFO:
			send_info();
			break;
		case CONNECT_TO_ALCHEMIST:
			go();
//			connect_to_alchemist();
			break;
		case HANDSHAKES:
			boost::asio::post(ioc, [this]() { send_handshakes(); });
			ioc.run();
			ioc.restart();
			break;
		case SEND_TEST_STRINGS:
			boost::asio::post(ioc, [this]() { send_test_strings(); });
			ioc.run();
			ioc.restart();
			break;
		case REQUEST_TEST_STRINGS:
			boost::asio::post(ioc, [this]() { request_test_strings(); });
			ioc.run();
			ioc.restart();
			break;
		case ALCHEMIST_MATRIX_LAYOUT:
			boost::asio::post(ioc, [this]() { send_test_array(); });
			ioc.run();
			ioc.restart();
			break;
		case NEW_RANDOM_MATRIX:
			boost::asio::post(ioc, [this]() { new_random_matrix(); });
			ioc.run();
			ioc.restart();
			break;
		case SET_MATRIX_ID:
			boost::asio::post(ioc, [this]() { set_matrix_ID(); });
			ioc.run();
			ioc.restart();
			break;
		case SEND_MATRIX_DATA:
			boost::asio::post(ioc, [this]() { send_matrix_data(); });
			ioc.run();
			ioc.restart();
			break;
	}
}

void Worker::set_matrix_ID()
{
	MPI_Status status;

	MPI_Bcast(&current_matrix_ID, 1, MPI_UNSIGNED_SHORT, 0, world);

	MPI_Barrier(world);
}

void Worker::send_matrix_data()
{
	MPI_Status status;
	uint16_t num_alchemist_workers, alchemist_worker;
	uint32_t num_rows, max_num_rows;

	MPI_Recv(&num_alchemist_workers, 1, MPI_INTEGER2, 0, 0, world, &status);
	MPI_Recv(&max_num_rows, 1, MPI_INTEGER4, 0, 0, world, &status);

	uint32_t buffer[max_num_rows];

	row_layout = vector<vector<uint32_t>>(num_alchemist_workers, vector<uint32_t>());

	for (uint16_t j = 0; j < num_alchemist_workers; j++) {
		MPI_Recv(&alchemist_worker, 1, MPI_INTEGER2, 0, 0, world, &status);
		MPI_Recv(&num_rows, 1, MPI_INTEGER4, 0, 0, world, &status);
		row_layout[j].reserve(num_rows);
		MPI_Recv(&buffer[0], num_rows, MPI_INTEGER4, 0, 0, world, &status);
		for (uint64_t i = 0; i < num_rows; i++)
			row_layout[j].push_back(buffer[i]);
	}

	std::stringstream ss;
	for (auto i = 0; i < row_layout.size(); i++) {
		for (auto j = 0; j < row_layout[i].size(); j++)
			ss << row_layout[i][j] << " ";
		ss << std::endl;
	}
	log->info(ss.str());

	for (uint16_t j = 0; j < num_alchemist_workers; j++)
		sessions[j]->send_data(current_matrix_ID, data, row_layout[j]);
//
//	std::stringstream ss;
//
//	ss << std::endl;
//	for (uint16_t j = 0; j < num_alchemist_workers; j++) {
//		for (uint32_t k = 0; k < row_layout[j].size(); k++)
//			ss << row_layout[j][k] << " ";
//		ss << std::endl;
//	}
//	log->info(ss.str());
}

uint32_t Worker::local_row_index(uint32_t & global_row_index)
{
	uint32_t i;

	for (i = 0; i < row_indices.size(); i++)
		if (row_indices[i] == global_row_index) break;

	return i;
}

void Worker::new_random_matrix()
{
	uint32_t dims[2];
	uint32_t num_rows, num_cols, row, col;
	MPI_Status status;

	MPI_Recv(&dims, 2, MPI_INTEGER4, 0, 0, world, &status);

	num_rows = dims[0];
	num_cols = dims[1];

	row_indices = vector<uint32_t>(num_rows);
	col_indices = vector<uint32_t>(num_cols);

	MPI_Recv(&row_indices[0], num_rows, MPI_INTEGER4, 0, 0, world, &status);

	for (uint32_t j = 0; j < num_cols; j++)
		MPI_Recv(&col_indices[j], 1, MPI_INTEGER4, 0, 0, world, &status);

	data = vector<vector<float> >(num_rows, vector<float>(num_cols));

	float entry = ID*10000.0;

	for (auto i = 0; i < num_rows; i++)
		for (auto j = 0; j < num_cols; j++)
			data[i][j] = entry++;

	std::stringstream ss;

	ss << "MATRIX" << std::endl;

	for (auto i = 0; i < num_rows; i++) {
		for (auto j = 0; j < num_cols; j++) {
			data[i][j] = entry++;
			ss << data[i][j] << " ";
		}
		ss << std::endl;
	}

	log->info(ss.str());

	MPI_Barrier(world);
}

void Worker::send_test_array()
{
	log->info("Sending test array");

	unsigned short int num_alchemist_workers;
	float temp[2];
	MPI_Status status;

	MPI_Bcast(&num_alchemist_workers, 1, MPI_UNSIGNED_SHORT, 0, world);

	auto num_rows = row_end - row_start;
	auto num_cols = col_end - col_start;

	uint32_t dims[8];

	dims[0] = row_start;
	dims[1] = row_end;
	dims[2] = col_start;
	dims[3] = col_end;

	auto my_layout = std::vector<std::vector<float> >(num_alchemist_workers, std::vector<float>(2));

	for (auto i = 0; i < num_alchemist_workers; i++) {
		MPI_Recv(&temp, 2, MPI_FLOAT, 0, 0, world, &status);
		my_layout[i][0] = temp[0];
		my_layout[i][1] = temp[1];

		if (temp[0] < temp[1]) {
			dims[4] = row_start + std::ceil(temp[0]*num_rows);
			dims[5] = row_start + std::floor(temp[1]*num_rows);
			dims[6] = col_start;
			dims[7] = col_end;

//			log->info("eeeeee {} {} {} {}", dims[4], dims[5], dims[6], dims[7]);
			sessions[i]->send_data(current_matrix_ID, data, dims);
		}
	}

//	for (auto it = sessions.begin(); it != sessions.end(); it++) {
//		log->info("ffffff {} {} {}", it->first, it->second->get_address(), it->second->get_port());
//	}
//
//	for (auto i = 0; i < num_alchemist_workers; i++)
//		log->info("ggggggg {} {} {}", my_layout[i][0], my_layout[i][1], sessions.size());

//	MPI_Barrier(world);



}

void Worker::go()
{
	connect_to_alchemist();
//	some_threads.push_back(boost::thread(& Worker::connect_to_alchemist, this));
//
//	for (auto & t: some_threads) t.join();
}

void Worker::session_idle(Session_ID session_ID)
{
	sessions_idle += 1;

	if (sessions_idle == num_alchemist_workers) {
		sessions_idle = 0;
		MPI_Barrier(world);
	}
}

void Worker::send_handshakes()
{
//	for (auto it = alchemist_workers.begin(); it != alchemist_workers.end(); it++) {
//		current_alchemist_worker_ID = it->first;
//		sessions[current_alchemist_worker_ID]->start();
//	}

	ses->send_handshake();

//	log->info("Thread split");
//		boost::thread t = boost::thread(& Worker::go1, this);
////
//		t.join();
//		log->info("Thread joined");

//	for (auto it = sessions.begin(); it != sessions.end(); it++)
//		it->second->send_hand();
}

void Worker::go1()
{
	ses->send_handshake();
	ses->read_header();
}

void Worker::send_test_strings()
{
	char buffer[4];
	std::stringstream test_str;

	sprintf(buffer, "%03d", ID);
	test_str << "This is a test string from ACI worker " << buffer;

	log->info("Sending the following message to Alchemist: '{}'", test_str.str());

	ses->send_test_string(test_str.str());

//	for (auto it = sessions.begin(); it != sessions.end(); it++)
//		it->second->send_test_string(test_str.str());
}

void Worker::request_test_strings()
{
	ses->request_test_string();
//	for (auto it = sessions.begin(); it != sessions.end(); it++)
//		it->second->request_test_string();
}

Worker_ID Worker::get_ID()
{
	return ID;
}

void Worker::connect_to_alchemist()
{
	uint16_t pl, al, hl;


	// Receive Alchemist worker info from driver
	MPI_Bcast(&num_alchemist_workers, 1, MPI_UNSIGNED_SHORT, 0, world);

	for(int i = 0; i < num_alchemist_workers; i++) {
		MPI_Bcast(&hl, 1, MPI_UNSIGNED_SHORT, 0, world);
		char hostname[hl];
		MPI_Bcast(hostname, hl, MPI_CHAR, 0, world);
		MPI_Bcast(&al, 1, MPI_UNSIGNED_SHORT, 0, world);
		char address_loc[al];
		MPI_Bcast(address_loc, al, MPI_CHAR, 0, world);
		MPI_Bcast(&pl, 1, MPI_UNSIGNED_SHORT, 0, world);

		alchemist_workers[i] = WorkerInfo();
		alchemist_workers[i].hostname = string(hostname);
		alchemist_workers[i].address = string(address_loc);
		alchemist_workers[i].port = pl;

		log->info("Received Alchemist worker {} at {}:{}", string(hostname), string(address_loc), pl);
	}

	for (auto it = alchemist_workers.begin(); it != alchemist_workers.end(); it++) {
		log->info("Connecting to Alchemist worker {} at {}:{}", it->second.hostname, it->second.address, it->second.port);
		address = it->second.address;
		port = it->second.port;
		current_alchemist_worker_ID = it->first;
		open_connection(address, port);
		sessions[current_alchemist_worker_ID]->set_alchemist_worker(it->second);
	}
// std::to_string(port)
//	address = "0.0.0.0";
//		port = 24961;

//		boost::thread t = boost::thread(& Worker::open_connection, this);
//		//
//				t.join();
//				log->info("Thread joined");

//	open_connection();

//	MPI_Barrier(world);
}

void Worker::connect_workers()
{

}

int Worker::send_info()
{

//	log->info("Sending hostname and port to driver");
//
//	std::string address = acceptor.local_endpoint().address().to_string();
//
//	uint16_t hl = hostname.length()+1;
//	uint16_t al = address.length()+1;
//
//	MPI_Send(&hl, 1, MPI_UNSIGNED_SHORT, 0, 0, world);
//	MPI_Send(hostname.c_str(), hl, MPI_CHAR, 0, 0, world);
//	MPI_Send(&al, 1, MPI_UNSIGNED_SHORT, 0, 0, world);
//	MPI_Send(address.c_str(), al, MPI_CHAR, 0, 0, world);
//	MPI_Send(&port, 1, MPI_UNSIGNED_SHORT, 0, 0, world);

	return 0;
}

int Worker::receive_test_string(const WorkerSession_ptr session, const char * data, const uint32_t length) {

	string test_string(data, length);

	log->info("Test string from Session {} at {}:{}", session->get_ID(), session->get_address(), session->get_port());
	log->info(test_string);

	return 0;
}

int Worker::send_test_string(WorkerSession_ptr session) {

	char buffer[4];
	std::stringstream test_str;

	sprintf(buffer, "%03d", ID);
	test_str << "This is a test string from ACI worker " << buffer;

	log->info("Sending the following message to Alchemist: '{}'", test_str.str());
	session->send_test_string(test_str.str());

	return 0;
}

int Worker::run_task() {


	return 0;
}

// ----------------------------------------   Parameters   ---------------------------------------

int Worker::process_input_parameters(Parameters & input_parameters) {



	return 0;
}

int Worker::process_output_parameters(Parameters & output_parameters) {



	return 0;
}

// -----------------------------------------   Testing   -----------------------------------------


// ----------------------------------------   Matrices   -----------------------------------------

//MatrixHandle Worker::register_matrix(size_t num_rows, size_t num_cols) {
//
//	MatrixHandle handle{0};
//
//
//	return handle;
//}

int Worker::receive_new_matrix() {


	return 0;
}

int Worker::get_matrix_dimensions() {


	return 0;
}

int Worker::get_transpose() {



	return 0;
}

int Worker::matrix_multiply() {



	return 0;
}

int Worker::get_matrix_rows() {



	return 0;
}

// ===============================================================================================
// ===============================================================================================

} // namespace alchemist
