#ifndef ACI__ACI_HPP
#define ACI__ACI_HPP


#include <ctime>
#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <cstdlib>
#include <deque>
#include <list>
#include <memory>
#include <set>
#include <utility>
#ifdef ASIO_STANDALONE
#include <asio.hpp>
#else
#include <boost/version.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#endif
#include <El.hpp>
#include "mpi.h"
#include "Message.hpp"
#include "utility/logging.hpp"

#define UNLIKELY(x) __builtin_expect(!!(x), 0)

#define ACI_VERSION_MAJOR 0
#define ACI_VERSION_MINOR 2

#ifndef NDEBUG
#define ENSURE(x) assert(x)
#else
#define ENSURE(x) do { if(!(x)) { \
  fprintf(stderr, "FATAL: invariant violated: %s:%d: %s\n", __FILE__, __LINE__, #x); fflush(stderr); abort(); } while(0)
#endif


#ifdef ASIO_STANDALONE
typedef asio::io_context io_context;
typedef asio::error_code error_code;
#else
typedef boost::asio asio;
typedef boost::system::error_code error_code;
#if BOOST_VERSION < 106600
typedef asio::io_service io_context;
#else
typedef asio::io_context io_context;
#endif
#endif

namespace alchemist {

using std::string;
using std::vector;
using asio::ip::tcp;

typedef El::Matrix<double> Matrix;
typedef El::AbstractDistMatrix<double> DistMatrix;

inline const std::string get_ACI_version()
{
	std::stringstream ss;
	ss << ACI_VERSION_MAJOR << "." << ACI_VERSION_MINOR;
	return ss.str();
}

#ifndef ASIO_STANDALONE
inline const std::string get_Boost_version()
{
	std::stringstream ss;
	ss << BOOST_VERSION / 100000 << "." << BOOST_VERSION / 100 % 1000 << "." << BOOST_VERSION % 100;
	return ss.str();
}
#endif

typedef uint16_t Worker_ID;
typedef uint16_t Matrix_ID;

struct WorkerInfo {
	WorkerInfo(): id(0), hostname(string("0")), address(string("0")), port(0) {}
	WorkerInfo(string _hostname, string _address, uint16_t _port): id(0), hostname(_hostname), address(_address), port(_port) {}
	WorkerInfo(uint16_t _id, string _hostname, string _address, uint16_t _port): id(_id), hostname(_hostname), address(_address), port(_port) {}

	string hostname, address;
	uint16_t id, port;
};

struct MatrixInfo {
	Matrix_ID ID;
	uint32_t num_rows;
	uint32_t num_cols;

	explicit MatrixInfo() :
		ID(0), num_rows(0), num_cols(0) {
	}

	MatrixInfo(Matrix_ID & _ID, uint32_t & _num_rows, uint32_t & _num_cols) :
		ID(_ID), num_rows(_num_rows), num_cols(_num_cols) {
	}
};

//inline bool exist_test (const std::string & name) {
//    struct stat buffer;
//    return (stat(name.c_str(), & buffer) == 0);
//}
}	// namespace alchemist


#endif
