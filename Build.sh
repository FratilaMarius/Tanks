rm -f /dev/shm/board /dev/shm/sem.semaphore
gcc main.c -o main -lncurses -lpthread -lrt 
chmod +x main
./main "$@"

# TMUX: ctrlb % split, :setw synchronize-panes on