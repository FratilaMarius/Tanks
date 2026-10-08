rm -f /dev/shm/shared_mem /dev/shm/sem.semaphore
gcc main.c -o main -lncurses -lpthread -lrt 
chmod +x main
./main "$@"