function usage {
    echo "Wrong number of parameters"
    echo "Usage: ./build_n_run.sh <mode> <number_of_threads> <rows_A> <cols_A>"
    echo "	mode = row / column / block / canon"
    exit -1
}

OPT=""

if [ "$#" -lt 4 ]; then
    usage
elif [[ "$1" == "row" ]]; then
    mpicxx $OPT row.cpp -o row && { mpiexec --use-hwthread-cpus -n "$2" row "${@:3}"; }
    
elif [[ "$1" == "column" ]]; then
    mpicxx $OPT column.cpp -o column && { mpiexec --use-hwthread-cpus -n "$2" column "${@:3}"; }
    
elif [[ "$1" == "block" ]]; then
    mpicxx $OPT block.cpp -o block && { mpiexec --use-hwthread-cpus -n "$2" block "${@:3}"; }
    
elif [[ "$1" == "cannon" ]]; then
    mpicxx $OPT cannon.cpp -o cannon && { mpiexec --use-hwthread-cpus -n "$2" cannon "${@:3}"; }
fi
