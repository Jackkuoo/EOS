#!/bin/bash

SESSION="lab6"
PORT=4446

make clean
make

tmux has-session -t $SESSION 2>/dev/null

if [[ $? -eq 0 ]]
then
	tmux kill-session -t $SESSION
fi

tmux new-session -d -s $SESSION

tmux split-window -h -p 80

tmux send-keys -t 0 "./hw3 $PORT" C-m
sleep 1

#tmux send-keys -t 1 "./client 127.0.0.1 $PORT" C-m
tmux send-keys -t 1 "./hw3_checker 127.0.0.1 $PORT" C-m

tmux -2 attach-session -t $SESSION
