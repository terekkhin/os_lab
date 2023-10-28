
client : client.c
	gcc -pthread -o client client.c -L. multmodlib.a

server : server.c
	gcc -pthread server.c -o server -L. multmodlib.a

test_1 : client
	./client --k 10 --mod 17 --servers ./server_ports

clean :
	rm server client
