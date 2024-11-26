#include "SocketManager.hpp"

void updatePoll()
{
	// create pollfds
	// call poll
	// update _sockets with human-readable data
}

// IsClosed function in issue
bool isDataComplete(t_socket_data &socket) // cant be a bool need true, false, and shit broke go fix it as third state
{
	/*
		if (string.size > maxallowedbytes)
			return aaaahbrokenpacket;
	*/
	// check what encoding type
	/*
		if (string.end != \r\n\r\n && normalencoding)
			return aaaahbrokenpacket;
		else if (normalencoding)
			return yeehawparsepacket;
		if (chunkedencoding && end == 0/r/n/r/n)
			return yeehawparsepacket;
		return yeehawfalse; // just keep going with this socket dont chagne anything
	*/
	// if normal, check for \r\n\r\n or declare as invalid packet
	// if chunked, check for 0\r\n\r\n or check for overall size or declare as invalid packet
}

void Run()
{
	// call updatePoll
	// for (t_socket_data &socket : _sockets)
	// 1. if the client closed connection, clean up & remove from _sockets
	// 2. if socket ready, get data from client
	// 3. if data is ready (isDatacomplete), construct response
	// 4. if socket ready, send back response
}
