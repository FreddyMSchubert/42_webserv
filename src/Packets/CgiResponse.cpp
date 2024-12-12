#include "Response.hpp"

void Response::handleCgiResponse(Request &req, Config &config)
{
	Path path(req.getPath(), Path::Type::URL, config);
}