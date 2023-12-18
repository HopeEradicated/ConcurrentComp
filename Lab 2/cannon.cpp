#include <vector>
#include <iostream>
#include <mpi.h>
#include <cmath>

int main(int argc, char *argv[]) {    
    int N = 100, Nd;
    double *A, *B, *C, *temp;
    int dims[2], periods[2];
    int left, right, up, down;

    MPI::Init(argc, argv);
    int rank = MPI::COMM_WORLD.Get_rank();
    int size = MPI::COMM_WORLD.Get_size();
    
    try {
        N = std::stoul(argv[1]);
    } catch (std::exception e) {
        if (rank == 0)
            std::cout << "Error in argument list: " << e.what() << "\n";
        MPI::Finalize();
        return 1;
    }

    dims[0] = 0, dims[1] = 0;
    periods[0] = 1, periods[1] = 1;

    MPI_Dims_create(size, 2, dims);

    if (dims[0] != dims[1]) {
        if (rank == 0)
            std::cout << "Number of processors must be square\n";
        MPI_Finalize();
        return 0;
    }

    Nd = N / dims[0];
    A = new double[Nd * Nd];
    B = new double[Nd * Nd];
    C = new double[Nd * Nd];
    temp = new double[Nd * Nd];

    for (int i = 0; i < Nd; i++)
        for (int j = 0; j < Nd; j++) {
            A[i * Nd + j] = rand() / (double)RAND_MAX;
            B[i * Nd + j] = rand() / (double)RAND_MAX;
            C[i * Nd + j] = 0.0;
        }

    MPI_Comm comm;
    MPI_Cart_create(MPI_COMM_WORLD, 2, dims, periods, 1, &comm);
    MPI_Cart_shift(comm, 0, 1, &left, &right);
    MPI_Cart_shift(comm, 1, 1, &up, &down);

    double start = MPI::Wtime();
    MPI_Status status;

    for (int shift = 0; shift < dims[0]; shift++) {
        for (int i = 0; i < Nd; i++)
            for (int k = 0; k < Nd; k++)
                for (int j = 0; j < Nd; j++)
                    C[i * Nd + j] += A[i * Nd + k] * B[k * Nd + j];
                    
        if (shift == dims[0] - 1)
            break;

        MPI_Sendrecv(A, Nd * Nd, MPI::DOUBLE, left, 1, temp, Nd * Nd, MPI::DOUBLE, right, 1, comm, &status);
        std::swap(temp,A);

        MPI_Sendrecv(B, Nd * Nd, MPI::DOUBLE, up, 2, temp, Nd * Nd, MPI::DOUBLE, down, 2, comm, &status);
        std::swap(temp,B);
    }    
    
    double end = MPI::Wtime();

    MPI_Barrier(comm);    
    delete[] A,B,temp,C;
    
    if (rank == 0)
       std::cout << "Time: " << (end - start)*1000 << "ms\n";

    MPI::Finalize();
    return 0;
}
