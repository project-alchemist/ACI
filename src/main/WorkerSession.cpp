// =============================================================================================
//                                         WorkerSession
// =============================================================================================

#include "WorkerSession.hpp"

namespace alchemist {

WorkerSession::WorkerSession(tcp::socket _socket, Worker & _worker) :
		Session(std::move(_socket)), worker(_worker) { }

WorkerSession::WorkerSession(tcp::socket _socket, Worker & _worker, Session_ID _ID) :
		Session(std::move(_socket), _ID), worker(_worker) { }

WorkerSession::WorkerSession(tcp::socket _socket, Worker & _worker, Session_ID _ID, Log_ptr & _log) :
		Session(std::move(_socket), _ID, _log), worker(_worker) { }

void WorkerSession::start()
{
//	worker.add_session(shared_from_this());
	send_handshake();
}

void WorkerSession::set_alchemist_worker(WorkerInfo _alchemist_worker)
{
	alchemist_worker = _alchemist_worker;
	address = alchemist_worker.address;
	port = alchemist_worker.port;
}

void WorkerSession::remove_session()
{
	worker.remove_session(shared_from_this());
}

void WorkerSession::send_data(Matrix_ID matrix_ID, vector<vector<float>> & data, vector<uint32_t> & layout)
{
	log->info("Sending matrix data");

	std::stringstream ss;
	for (auto i = 0; i < layout.size(); i++) ss << layout[i] << " ";
	log->info("{}", ss.str());

	uint32_t num_blocks = 0;

	for (auto i = 1; i < layout.size(); i++)
		if (layout[i] > layout[i-1]+1) num_blocks++;
	num_blocks++;

	write_msg.start(MATRIX_BLOCK);
	write_msg.add_uint16(matrix_ID);
	write_msg.add_uint32(num_blocks);

	uint32_t row_start = 0, row_end = 0;
	uint32_t col_start = 0, col_end = data[0].size()-1;

	for (auto block = 0; block < num_blocks; block++) {

		// Find end row of current block
		for (auto i = row_start+1; i < layout.size(); i++)
			if (layout[i] > layout[i-1]+1) {
				row_end = i-1;
				break;
			}

		if (block == num_blocks-1) row_end = layout.size()-1;

		write_msg.add_uint32(layout[row_start]);
		write_msg.add_uint32(layout[row_end]);
		write_msg.add_uint32(col_start);
		write_msg.add_uint32(col_end);

		for (uint32_t i = row_start; i <= row_end; i++)
			for (uint32_t j = col_start; j <= col_end; j++)
				write_msg.add_float(data[worker.local_row_index(layout[i])][j]);

		row_start = row_end+1;
	}

	flush();

	receive_response();
}

void WorkerSession::send_data(Matrix_ID matrix_ID, vector<vector<float> > & data, uint32_t dims[8])
{
	uint32_t row_start      = dims[0];
	uint32_t row_end        = dims[1];
	uint32_t col_start      = dims[2];
	uint32_t col_end        = dims[3];
	uint32_t row_send_start = dims[4];
	uint32_t row_send_end   = dims[5];
	uint32_t col_send_start = dims[6];
	uint32_t col_send_end   = dims[7];

	write_msg.start(MATRIX_BLOCK);

	write_msg.add_uint32(row_send_start);
	write_msg.add_uint32(row_send_end);
	write_msg.add_uint32(col_start);
	write_msg.add_uint32(col_end);

//	log->info("Dimensions: {} {} {} {}", row_send_start, row_send_end, col_send_start, col_send_end);

	for (auto i = row_send_start - row_start; i <= row_send_end - row_start; i++)
		for (auto j = col_send_start - col_start; j <= col_send_end - col_start; j++)
			write_msg.add_float(data[i][j]);

	flush();

	receive_response();
}

int WorkerSession::handle_message()
{
	client_command command = read_msg.cc;

//	log->info("Received message {} from Session {} at {}", get_command_name(command), get_ID(), get_address().c_str());

	switch (command) {
		case SHUT_DOWN:
//			shut_down();
			break;
		case HANDSHAKE:
			check_handshake() ? log->info("Handshake successful") : log->info("Handshake not successful");
			worker.session_idle(ID);
			break;
		case SEND_TEST_STRING:
			log->info(read_msg.read_string());
			worker.session_idle(ID);
			break;
		case REQUEST_TEST_STRING:
			log->info(read_msg.read_string());
			worker.session_idle(ID);
			break;
		case MATRIX_BLOCK:
			log->info(read_msg.read_string());
			worker.session_idle(ID);
			break;
	}

	return 0;
}

}			// namespace alchemist
