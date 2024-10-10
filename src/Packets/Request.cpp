#include "../../include/Packets/Request.hpp"

// TODO: this function should not return void but the response to the request that we can send it back directly
void Request::Run()
{
	std::cout << "Request::Run()" << std::endl;
}

/*
Response Request::Run()
{
	// do what the request is supposed to do and then return the response
}
*/