#include "Driver.hpp"

int main(int argc, char *argv[]) {

	MPI_Init(NULL, NULL);

	El::Initialize();

	MPI_Comm peers, world = MPI_COMM_WORLD;

	// Get the rank and size in the original communicator
	int world_rank, world_size;
	MPI_Comm_rank(world, &world_rank);
	MPI_Comm_size(world, &world_size);

	bool is_driver = world_rank == 0;
	int color = is_driver ? 0 : 1; // Determine color based on row

	// Split the communicator based on the color and use the original rank for ordering
	MPI_Comm_split(world, color, world_rank, &peers);

	int peers_rank, peers_size;
	MPI_Comm_rank(peers, &peers_rank);
	MPI_Comm_size(peers, &peers_size);


	io_context _io_context;

	try {
		if (is_driver)
			alchemist::Driver d(world, peers, _io_context, 55555);
		else
			alchemist::Worker w(world, peers, _io_context, 55555+world_rank);
	}
	catch (std::exception & e) {
		std::cerr << "Exception while starting Alchemist Client Interface: " << e.what() << std::endl;
	}

//	MPI_Barrier(world);
	El::Finalize();
	MPI_Comm_free(&peers);
	MPI_Finalize();

	return 0;
}


//		boost::asio::io_context io_context;
//		alchemist::Client ac(io_context);
//		ac.start();


