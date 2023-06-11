all: client server

sever: server.c
	clang -o server server.c

client: client.c
	clang -o client client.c

clean:
	rm -rf out/* server client
