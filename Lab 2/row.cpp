#include <vector>
#include <iostream>
#include <mpi.h>

int main(int argc, char **argv) {
    int rows_A, cols_A, rows_B;

    MPI::Init(argc, argv);
    int rank = MPI::COMM_WORLD.Get_rank();
    int size = MPI::COMM_WORLD.Get_size();

    try {
        rows_A = std::stoul(argv[1]);
        rows_B = cols_A = std::stoul(argv[2]);
    } catch (std::exception e) {
        if (rank == 0)
            std::cout << "Error in argument list: " << e.what() << "\n";
        MPI::Finalize();
        return 1;
    }
 
    int rows_per_thread = rows_A / size;

    std::vector<double> A;  
    std::vector<double> A_local(rows_per_thread * cols_A);
    std::vector<double> B(rows_B);
    std::vector<double> C;  

    if (rank == 0) {
        A.reserve(rows_A * cols_A);
        for (int i = 0; i < rows_A * cols_A; ++i)
            A[i] = rand() / (double)RAND_MAX;
        for (int i = 0; i < rows_B; ++i)
            B[i] = rand() / (double)RAND_MAX;
	C.reserve(rows_A);
    }

    MPI::COMM_WORLD.Scatter(A.data(), rows_per_thread * cols_A, MPI::DOUBLE, A_local.data(), rows_per_thread * cols_A, MPI::DOUBLE, 0);
    MPI::COMM_WORLD.Bcast(B.data(), rows_B, MPI::DOUBLE, 0);

    double start = MPI::Wtime();

    std::vector<double> C_local(rows_per_thread);
    for (int i = 0; i < rows_per_thread; ++i)
    	for (int k = 0; k < cols_A; ++k)
	    C_local[i] += A_local[i * cols_A + k] * B[k];
	    
    double end = MPI::Wtime();

    MPI::COMM_WORLD.Gather(C_local.data(), rows_per_thread, MPI::DOUBLE, C.data(), rows_per_thread, MPI::DOUBLE, 0);

    if (rank == 0)
        std::cout << "Time: " << (end - start)*1000 << "ms\n";

    MPI::Finalize();
    return 0;
}
