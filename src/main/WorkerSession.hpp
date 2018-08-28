#ifndef ACI__WORKERSESSION_HPP
#define ACI__WORKERSESSION_HPP


#include "Session.hpp"
#include "Worker.hpp"


namespace alchemist {

using boost::asio::ip::tcp;
using std::string;

typedef std::deque<Message> Message_queue;

class Worker;

class WorkerSession : public Session
{
public:
	WorkerSession(tcp::socket, Worker &);
	WorkerSession(tcp::socket, Worker &, Session_ID);
	WorkerSession(tcp::socket, Worker &, Session_ID, Log_ptr &);

	int handle_message();

	void set_alchemist_worker(WorkerInfo _alchemist_worker);

	void send_data(Matrix_ID matrix_ID, vector<vector<float> > & data, uint32_t dims[8]);
	void send_data(Matrix_ID matrix_ID, vector<vector<float> > & data, vector<uint32_t> & layout);

	void start();

	void remove_session();

	std::shared_ptr<WorkerSession> shared_from_this()
	{
		return std::static_pointer_cast<WorkerSession>(Session::shared_from_this());
	}

private:
	Worker & worker;
	WorkerInfo alchemist_worker;
};

typedef std::shared_ptr<WorkerSession> WorkerSession_ptr;

}			// namespace alchemist

#endif		// ACI__WORKERSESSION_HPP
