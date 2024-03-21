all:	TCPServer TCPClient UDPServer UDPClient

TCPServer:	TCPServer.c
	$(CC) $(CFLAGS) -o TCPServer TCPServer.c
    
TCPClient:	TCPClient.c
	$(CC) $(CFLAGS) -o TCPClient TCPClient.c

UDPServer:	UDPServer.c
	$(CC) $(CFLAGS) -o UDPServer UDPServer.c

UDPClient:	UDPClient.c
	$(CC) $(CFLAGS) -o UDPClient UDPClient.c

clean:
	rm -f TCPServer
	rm -f TCPClient
	rm -f UDPServer
	rm -f UDPClient
