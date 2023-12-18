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

    if (!(rows_A == cols_A)) {
        if (rank == 0)
            std::cout << "Only for square matrixes\n";
        MPI::Finalize();
        return 1;
    }
 
    int block_number = size;
    int block_side = rows_A / size;
    int block_len = rows_A * rows_A / block_number;

    if (block_side == 0) {
        if (rank == 0)
            std::cout << "Number of threads should be less than matrix size\n";
        MPI::Finalize();
        return 1;
    }

    std::vector<double> A_local(block_len); 
    std::vector<double> B(rows_B);
    std::vector<double> C;  

    if (rank == 0) {
    	std::vector<double> A(rows_A * cols_A);
        for (int i = 0; i < rows_A * cols_A; ++i)
            A[i] = rand() / (double)RAND_MAX;
        for (int i = 0; i < rows_B; ++i)
            B[i] = rand() / (double)RAND_MAX;
        C.reserve(rows_A);

        std::vector<double> A_temp(block_len);
        for (int t_number = 0; t_number < size; ++t_number) {
            int q = 0; 
            int k = 0;
            for (int i = 0; i < rows_A; ++i) {
                if (i % block_side == 0 && i != 0)
                    k += block_side;
                for (int j = k; j < k + block_side; ++j)              
                    A_temp[q++] = A[i * cols_A + (j + t_number * block_side) % rows_A];
            }

            if (t_number == 0) {
                A_local = A_temp;
            } else {
                MPI::COMM_WORLD.Send(A_temp.data(), A_temp.size(), MPI::DOUBLE, t_number, t_number);
            }
        }
    }
    
    if (rank != 0)
        MPI::COMM_WORLD.Recv(A_local.data(), rows_A * rows_A, MPI::DOUBLE, 0, rank);

    MPI::COMM_WORLD.Bcast(B.data(), B.size(), MPI::DOUBLE, 0);
    
    std::vector<double> C_local(rows_A, 0.0);
    int q = rank * block_side;    
    double start = MPI::Wtime();
    
    for (int i = 0; i < rows_A; ++i) {
        if (i % block_side == 0 && i != 0)
            q = (q + block_side) % rows_A;
    	for (int k = 0; k < block_side; ++k)  	
    	    C_local[i] += A_local[k + block_side * i] * B[(k + q)];
    }
    
    double end = MPI::Wtime();
    
    MPI::COMM_WORLD.Reduce(C_local.data(), C.data(), C_local.size(), MPI::DOUBLE, MPI::SUM, 0);

    if (rank == 0)
        std::cout << "Time: " << (end - start)*1000 << "ms\n";

    MPI::Finalize();
    return 0;
}
