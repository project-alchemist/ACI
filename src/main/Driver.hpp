#ifndef ACI__DRIVER_HPP
#define ACI__DRIVER_HPP

#include <iostream>
#include <fstream>
#include "Worker.hpp"
#include "DriverSession.hpp"


namespace alchemist {

using std::vector;

class DriverSession;

typedef std::shared_ptr<DriverSession> DriverSession_ptr;

class Driver : public Client, public std::enable_shared_from_this<Driver>
{
public:

	Driver(MPI_Comm & _world, MPI_Comm & _peers, io_context & _io_context, const unsigned int port);
	Driver(MPI_Comm & _world, MPI_Comm & _peers, io_context & _io_context, const tcp::endpoint & endpoint);
	~Driver();

	int start();

	void load_library();

	void print_menu();

	void send_info();

	void new_session();
	void add_session(DriverSession_ptr session);
	void remove_session();

	void add_alchemist_worker(WorkerInfo aw);

	void connect_workers();
	void connect_alchemist_workers(std::map<Worker_ID, WorkerInfo> _alchemist_workers);

	void handshake_with_workers();
	void send_test_strings_to_workers();
	void request_test_strings_from_workers();

	void list_all_alchemist_workers(string list);
	void list_active_alchemist_workers(string list);
	void list_inactive_alchemist_workers(string list);
	void list_assigned_alchemist_workers(string list);
	void list_assigned_alchemist_workers();

	void send_Elemental_test_matrix();
	void get_data_layout_table();

	void send_ScaLAPACK_test_matrix();
	void prepare_data_layout_table();

	void set_layout_matrix(vector<vector<vector<float> > > & _layout_rr);
	void print_layout_matrix();

	uint16_t get_num_workers();

	void get_matrix_dimensions(uint32_t & num_rows, uint32_t & num_cols);

	void send_layout_to_workers();

	void set_matrix_layout(vector<vector<uint32_t> > & _rows_on_alchemist_workers);

	void set_matrix_ID(Matrix_ID & _matrix_ID);
	void set_matrix_row_assignments(Matrix_ID & _matrix_ID, vector<uint16_t> & _alchemist_row_assignments);

	void prepare_layout_lists();

	void send_matrix_data();

private:
	MPI_Comm & world;
	MPI_Comm & peers;

	bool layout_ready;
	uint16_t num_workers;

	uint32_t current_num_rows, current_num_cols;

	vector<vector<vector<float> > > layout_rr;

	vector<vector<uint32_t> > rows_on_alchemist_workers;
	vector<vector<uint32_t> > rows_on_client_workers;
	vector<vector<vector<uint32_t> > > row_layout;

	vector<Worker_ID> alchemist_row_assignments;
	vector<Worker_ID> client_row_assignments;

	bool workers_assigned;
	bool workers_connected;

	DriverSession_ptr session;

	std::map<Matrix_ID, MatrixInfo> matrices;
	std::map<Worker_ID, WorkerInfo> workers;

	uint32_t next_matrix_ID;

	// ====================================   UTILITY FUNCTIONS   ====================================

	// ----------------------------------------   Printing   -----------------------------------------

	void print_welcome_message();
	void print_ready_message();

	void print_workers();

	void invalid_menu_choice();

	void get_endpoint();


	// -----------------------------------------   Workers   -----------------------------------------

	void start_workers();
	int register_workers();
	int assign_workers();

	// ----------------------------------------   File I/O   ----------------------------------------

	int read_HDF5();

	// ====================================   COMMAND FUNCTIONS   ====================================

	int handle_command(int command_code);
	int run_task();

	int shut_down();

	void send_handshake();
	void send_test_string();
	void request_test_string();


	void connect();

	// ---------------------------------------   Information   ---------------------------------------

	int send_assigned_workers_info();

	// ----------------------------------------   Parameters   ---------------------------------------

	int process_input_parameters(Parameters & input_parameters);
	int process_output_parameters(Parameters & output_parameters);

	// -----------------------------------------   Testing   -----------------------------------------

	int receive_test_string(const DriverSession_ptr, const char *, const uint32_t);
	int send_test_string(DriverSession_ptr);

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
