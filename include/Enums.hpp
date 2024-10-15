#pragma once

#include <iostream>

enum OptionalBoolean
{
	MAYBE,
	TRUE,
	FALSE,
};

inline std::ostream &operator<<(std::ostream &os, const OptionalBoolean &b)
{
	switch (b)
	{
		case OptionalBoolean::TRUE:
			os << "true";
			break;
		case OptionalBoolean::FALSE:
			os << "false";
			break;
		default:
			os << "maybe";
			break;
	}
	return os;
}

enum class Method
{
	UNKNOWN,
	GET,
	POST,
	DELETE,
};

inline std::ostream &operator<<(std::ostream &os, const Method &m)
{
	switch (m)
	{
		case Method::GET:
			os << "GET";
			break;
		case Method::POST:
			os << "POST";
			break;
		case Method::DELETE:
			os << "DELETE";
			break;
		default:
			os << "UNKNOWN";
			break;
	}
	return os;
}

enum class Status {
	UNKNOWN = 0,
	Continue = 100,
	SwitchingProtocols = 101,
	Processing = 102,
	EarlyHints = 103,
	OK = 200,
	Created = 201,
	Accepted = 202,
	NonAuthoritativeInformation = 203,
	NoContent = 204,
	ResetContent = 205,
	PartialContent = 206,
	MultiStatus = 207,
	AlreadyReported = 208,
	IMUsed = 226,
	MultipleChoices = 300,
	MovedPermanently = 301,
	Found = 302,
	SeeOther = 303,
	NotModified = 304,
	UseProxy = 305,
	TemporaryRedirect = 307,
	PermanentRedirect = 308,
	BadRequest = 400,
	Unauthorized = 401,
	PaymentRequired = 402,
	Forbidden = 403,
	NotFound = 404,
	MethodNotAllowed = 405,
	NotAcceptable = 406,
	ProxyAuthenticationRequired = 407,
	RequestTimeout = 408,
	Conflict = 409,
	Gone = 410,
	LengthRequired = 411,
	PreconditionFailed = 412,
	PayloadTooLarge = 413,
	URITooLong = 414,
	UnsupportedMediaType = 415,
	RangeNotSatisfiable = 416,
	ExpectationFailed = 417,
	ImATeapot = 418,
	MisdirectedRequest = 421,
	UnprocessableEntity = 422,
	Locked = 423,
	FailedDependency = 424,
	TooEarly = 425,
	UpgradeRequired = 426,
	PreconditionRequired = 428,
	TooManyRequests = 429,
	RequestHeaderFieldsTooLarge = 431,
	UnavailableForLegalReasons = 451,
	InternalServerError = 500,
	NotImplemented = 501,
	BadGateway = 502,
	ServiceUnavailable = 503,
	GatewayTimeout = 504,
	HTTPVersionNotSupported = 505,
	VariantAlsoNegotiates = 506,
	InsufficientStorage = 507,
	LoopDetected = 508,
	NotExtended = 510,
	NetworkAuthenticationRequired = 511
};