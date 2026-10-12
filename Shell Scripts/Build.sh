gcc ../main.c ../source/*.c -o ../main -lncurses -lpthread -lrt
chmod +x ../main
# ./main "$@"

# TMUX: ctrlb % split, :setw synchronize-panes on