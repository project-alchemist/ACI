#ifndef ACI__WORKER_HPP
#define ACI__WORKER_HPP

#include "Client.hpp"
#include "WorkerSession.hpp"

namespace alchemist {

class WorkerSession;

typedef std::shared_ptr<WorkerSession> WorkerSession_ptr;

class Worker : public Client, public std::enable_shared_from_this<Worker>
{
public:

	Worker(MPI_Comm & _world, MPI_Comm & _peers, io_context & _io_context, const unsigned int port);
	Worker(MPI_Comm & _world, MPI_Comm & _peers, io_context & _io_context, const tcp::endpoint & endpoint);
	~Worker();

	int start();

	void new_session();
	void add_session(WorkerSession_ptr session);
	void remove_session(WorkerSession_ptr session);

	void print_num_sessions();

	Worker_ID get_ID();

	void session_idle(Session_ID session_ID);

	void go();
	void go1();

	void send_test_array();
	void new_random_matrix();
	void send_matrix_data();

	uint32_t row_start, row_end, col_start, col_end;

	vector<vector<float> > data;
	vector<vector<uint32_t>> row_layout;

	uint32_t local_row_index(uint32_t & global_row_index);

private:
	MPI_Comm & world;
	MPI_Comm & peers;

	uint16_t num_alchemist_workers;

	vector<uint32_t> row_indices;
	vector<uint32_t> col_indices;

	Worker_ID ID;
	Matrix_ID current_matrix_ID;

	void set_matrix_ID();

	Worker_ID current_alchemist_worker_ID;
	void handle_command(aci_command c);

	Session_ID next_session_ID;

	WorkerSession_ptr ses;
	std::map<Worker_ID, WorkerSession_ptr> sessions;

	vector<Worker_ID> assigned_Alchemist_workers;
	vector<Worker_ID> target_percentages;

	// ====================================   UTILITY FUNCTIONS   ====================================

	// ----------------------------------------   File I/O   ----------------------------------------

	int read_HDF5();

	// ====================================   COMMAND FUNCTIONS   ====================================

	int wait_for_command();

	int send_info();
	int run_task();

	int sessions_idle;

	void get_endpoint();

	void send_handshakes();
	void send_test_strings();
	void request_test_strings();

	void connect();

	void connect_to_alchemist();

	void connect_workers();

	// ---------------------------------------   Information   ---------------------------------------

	// ----------------------------------------   Parameters   ---------------------------------------

	int process_input_parameters(Parameters & input_parameters);
	int process_output_parameters(Parameters & output_parameters);

	// -----------------------------------------   Testing   -----------------------------------------

	int receive_test_string(const WorkerSession_ptr, const char *, const uint32_t);
	int send_test_string(WorkerSession_ptr);

	// -----------------------------------------   Matrices   ----------------------------------------

//	MatrixHandle register_matrix(size_t num_rows, size_t num_cols);
	int receive_new_matrix();
	int get_matrix_dimensions();
	int get_transpose();
	int matrix_multiply();
	int get_matrix_rows();
};

}

#endif
