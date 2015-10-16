all: centralized_sensereversal openmp_tournament omp_builtin tournamentmpi mpi_dissemination mpi_builtin combined_barrier

centralized_sensereversal: OpenMP/centralized_sensereversal.c
	gcc -o centralized_sensereversal OpenMP/centralized_sensereversal.c -fopenmp
openmp_tournament: OpenMP/openmp_tournament.c
	gcc -o openmp_tournament OpenMP/openmp_tournament.c -lm -fopenmp
omp_builtin: OpenMP/omp_builtin.c
	gcc -o omp_builtin OpenMP/omp_builtin.c -fopenmp
tournamentmpi: MPI/tournamentmpi.c
	mpicc -o tournamentmpi MPI/tournamentmpi.c -lm
mpi_dissemination: MPI/mpi_dissemination.c
	mpicc -o mpi_dissemination MPI/mpi_dissemination.c -lm
mpi_builtin: MPI/mpi_builtin.c
	mpicc -o mpi_builtin MPI/mpi_builtin.c
combined_barrier: combined_barrier.c
	mpicc -o combined_barrier combined_barrier.c -lm -fopenmp
clean:
	rm centralized_sensereversal openmp_tournament omp_builtin tournamentmpi mpi_dissemination mpi_builtin combined_barrier