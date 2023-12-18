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
 
    int cols_per_thread = cols_A / size;

    std::vector<double> A(rows_A * cols_A);  
    std::vector<double> B;
    std::vector<double> B_local(cols_per_thread);
    std::vector<double> C;  

    if (rank == 0) {
        for (int i = 0; i < rows_A * cols_A; ++i)
            A[i] = rand() / (double)RAND_MAX;
        B.reserve(rows_B);
        for (int i = 0; i < rows_B; ++i)
            B[i] = rand() / (double)RAND_MAX; 
        C.reserve(rows_A);
    }

    MPI::COMM_WORLD.Scatter(B.data(), cols_per_thread, MPI::DOUBLE, B_local.data(), cols_per_thread, MPI::DOUBLE, 0);
    MPI::COMM_WORLD.Bcast(A.data(), rows_A * cols_A, MPI::DOUBLE, 0);

    double start = MPI::Wtime();
    
    std::vector<double> C_local(rows_A);
    for (int j = 0; j < cols_per_thread; ++j) 
        for (int i = 0; i < rows_A; ++i) 
            C_local[i] += A[i * cols_A + (rank * cols_per_thread)] * B_local[j];
            
    double end = MPI::Wtime();
    
    MPI::COMM_WORLD.Reduce(C_local.data(), C.data(), rows_A, MPI::DOUBLE, MPI::SUM, 0);

    if (rank == 0)
        std::cout << "Time: " << (end - start)*1000 << "ms\n";

    MPI::Finalize();
    return 0;
}
