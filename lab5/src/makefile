CC=gcc
CFLAGS=-I.


all : mutex_off.c mutex_on.c task_2.c
	$(CC) -o task_2 task_2.c $(CFLAGS)
	$(CC) -o mutex_off mutex_off.c $(CFLAGS)
	$(CC) -o mutex_on mutex_on.c $(CFLAGS)

clean :
	rm mutex_off task_2 mutex_on

