#ifndef ACI__COMMAND_HPP
#define ACI__COMMAND_HPP

#include <string>
#include <cstdlib>

namespace alchemist {

typedef enum _client_command : uint8_t {
	WAIT = 0,
	HANDSHAKE,
	REQUEST_ID,
	CLIENT_INFO,
	SEND_TEST_STRING,
	REQUEST_TEST_STRING,
	REQUEST_WORKERS,
	YIELD_WORKERS,
	SEND_ASSIGNED_WORKERS_INFO,
	LIST_ALL_WORKERS,
	LIST_ACTIVE_WORKERS,
	LIST_INACTIVE_WORKERS,
	LIST_ASSIGNED_WORKERS,
	LOAD_LIBRARY,
	RUN_TASK,
	UNLOAD_LIBRARY,
	MATRIX_INFO,
	MATRIX_LAYOUT,
	MATRIX_BLOCK,
	SHUT_DOWN
} client_command;

typedef enum _aci_command : uint8_t {
	IDLE = 0,
	START,
	SEND_INFO,
	ACCEPT_CONNECTION,
	CONNECT_TO_ALCHEMIST,
	HANDSHAKES,
	SEND_TEST_STRINGS,
	REQUEST_TEST_STRINGS,
	ALCHEMIST_MATRIX_LAYOUT,
	NEW_RANDOM_MATRIX,
	SET_MATRIX_ID,
	SEND_MATRIX_DATA
} aci_command;

inline const std::string get_command_name(const client_command & c)
{
	switch (c) {
		case WAIT:
			return "WAIT";
		case HANDSHAKE:
			return "HANDSHAKE";
		case REQUEST_ID:
			return "REQUEST ID";
		case CLIENT_INFO:
			return "CLIENT INFO";
		case SEND_TEST_STRING:
			return "SEND TEST STRING";
		case REQUEST_TEST_STRING:
			return "REQUEST TEST STRING";
		case REQUEST_WORKERS:
			return "REQUEST WORKERS";
		case YIELD_WORKERS:
			return "YIELD WORKERS";
		case SEND_ASSIGNED_WORKERS_INFO:
			return "SEND ASSIGNED WORKERS INFO";
		case LIST_ALL_WORKERS:
			return "LIST ALL WORKERS";
		case LIST_ACTIVE_WORKERS:
			return "LIST ACTIVE WORKERS";
		case LIST_INACTIVE_WORKERS:
			return "LIST INACTIVE WORKERS";
		case LIST_ASSIGNED_WORKERS:
			return "LIST ASSIGNED WORKERS";
		case LOAD_LIBRARY:
			return "LOAD LIBRARY";
		case RUN_TASK:
			return "RUN TASK";
		case UNLOAD_LIBRARY:
			return "UNLOAD LIBRARY";
		case MATRIX_INFO:
			return "MATRIX INFO";
		case MATRIX_LAYOUT:
			return "MATRIX LAYOUT";
		case MATRIX_BLOCK:
			return "MATRIX BLOCK";
		default:
			return "INVALID COMMAND";
		}
}

inline const std::string get_command_name(const aci_command & c)
{
	switch (c) {
		case START:
			return "START";
		case SEND_INFO:
			return "SEND_INFO";
		case ACCEPT_CONNECTION:
			return "ACCEPT CONNECTION";
		case CONNECT_TO_ALCHEMIST:
			return "CONNECT TO ALCHEMIST";
		case HANDSHAKES:
			return "HANDSHAKES";
		case SEND_TEST_STRINGS:
			return "SEND TEST STRINGS";
		case REQUEST_TEST_STRINGS:
			return "REQUEST TEST STRINGS";
		case ALCHEMIST_MATRIX_LAYOUT:
			return "ALCHEMIST_MATRIX LAYOUT";
		case NEW_RANDOM_MATRIX:
			return "NEW RANDOM MATRIX";
		case SET_MATRIX_ID:
			return "SET MATRIX ID";
		case SEND_MATRIX_DATA:
			return "SEND MATRIX DATA";
		default:
			return "INVALID COMMAND";
		}
}

}

#endif
