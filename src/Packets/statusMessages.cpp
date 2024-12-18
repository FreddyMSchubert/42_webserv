#include "Packet.hpp"

#include <unordered_map>
#include <string>

// Mapping of HTTP status codes to their string representations
const std::unordered_map<int, std::string> statusCodes = {
	{0, "UNKNOWN"},
	
	// 1xx: Informational
	{100, "Continue"},
	{101, "SwitchingProtocols"},
	{102, "Processing"},
	{103, "EarlyHints"},
	
	// 2xx: Success
	{200, "OK"},
	{201, "Created"},
	{202, "Accepted"},
	{203, "NonAuthoritativeInformation"},
	{204, "NoContent"},
	{205, "ResetContent"},
	{206, "PartialContent"},
	{207, "MultiStatus"},
	{208, "AlreadyReported"},
	{226, "IMUsed"},
	
	// 3xx: Redirection
	{300, "MultipleChoices"},
	{301, "MovedPermanently"},
	{302, "Found"},
	{303, "SeeOther"},
	{304, "NotModified"},
	{305, "UseProxy"},
	{307, "TemporaryRedirect"},
	{308, "PermanentRedirect"},
	
	// 4xx: Client Errors
	{400, "BadRequest"},
	{401, "Unauthorized"},
	{402, "PaymentRequired"},
	{403, "Forbidden"},
	{404, "NotFound"},
	{405, "MethodNotAllowed"},
	{406, "NotAcceptable"},
	{407, "ProxyAuthenticationRequired"},
	{408, "RequestTimeout"},
	{409, "Conflict"},
	{410, "Gone"},
	{411, "LengthRequired"},
	{412, "PreconditionFailed"},
	{413, "PayloadTooLarge"},
	{414, "URITooLong"},
	{415, "UnsupportedMediaType"},
	{416, "RangeNotSatisfiable"},
	{417, "ExpectationFailed"},
	{418, "ImATeapot"},
	{421, "MisdirectedRequest"},
	{422, "UnprocessableEntity"},
	{423, "Locked"},
	{424, "FailedDependency"},
	{425, "TooEarly"},
	{426, "UpgradeRequired"},
	{428, "PreconditionRequired"},
	{429, "TooManyRequests"},
	{431, "RequestHeaderFieldsTooLarge"},
	{451, "UnavailableForLegalReasons"},
	
	// 5xx: Server Errors
	{500, "InternalServerError"},
	{501, "NotImplemented"},
	{502, "BadGateway"},
	{503, "ServiceUnavailable"},
	{504, "GatewayTimeout"},
	{505, "HTTPVersionNotSupported"},
	{506, "VariantAlsoNegotiates"},
	{507, "InsufficientStorage"},
	{508, "LoopDetected"},
	{510, "NotExtended"},
	{511, "NetworkAuthenticationRequired"}
};

std::string Packet::getStatusMessage(int code)
{
	auto it = statusCodes.find(code);
	if (it != statusCodes.end()) {
		return it->second;
	}
	return "UNKNOWN";
}
