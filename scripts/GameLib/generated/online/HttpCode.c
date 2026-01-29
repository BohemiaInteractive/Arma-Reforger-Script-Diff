/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup online
\{
*/

/*!
Common HTTP (result / error) codes
Can be used in code as constant values to easier find use of specific HTTP code
compared to search for simple numbers in the code.
\note Only most common HTTP codes are covered by this enum.
*/
enum HttpCode
{
	//! no code
	HTTP_CODE_NULL,
	//! continue
	HTTP_CODE_100,
	//! ok :-)
	HTTP_CODE_200,
	//! created
	HTTP_CODE_201,
	//! accepted
	HTTP_CODE_202,
	//! (redirect) multiple choices
	HTTP_CODE_300,
	//! (redirect) moved permanently
	HTTP_CODE_301,
	//! (redirect) found - this may indicate improper use of "Http" instead "Https"
	HTTP_CODE_302,
	//! (redirect) see other
	HTTP_CODE_303,
	//! bad request response
	HTTP_CODE_400,
	//! unauthorized response
	HTTP_CODE_401,
	//! forbidden
	HTTP_CODE_403,
	//! not found
	HTTP_CODE_404,
	//! method not allowed
	HTTP_CODE_405,
	//! request timeout
	HTTP_CODE_408,
	//! conflict state
	HTTP_CODE_409,
	//! precondition failed (access denied)
	HTTP_CODE_412,
	//! payload too large (attempt to upload too much data)
	HTTP_CODE_413,
	//! i'am a teapot
	HTTP_CODE_418,
	//! missing data
	HTTP_CODE_422,
	//! custom backend code
	HTTP_CODE_433,
	//! non standard nginx used to instruct nginx to close connection :: https://httpstatus.in/444/
	HTTP_CODE_444,
	//! server error
	HTTP_CODE_500,
	//! not implemented
	HTTP_CODE_501,
	//! bad gateway
	HTTP_CODE_502,
	//! service unavailable
	HTTP_CODE_503,
	//! gateway timeout
	HTTP_CODE_504,
	//! the origin server returned an empty, unknown, or unexpected response to Cloudflare
	HTTP_CODE_520,
	//! the origin server refused connections from Cloudflare, security solutions at the origin may be blocking legitimate connections from certain Cloudflare IP addresses
	HTTP_CODE_521,
	//! Cloudflare timed out contacting the origin server
	HTTP_CODE_522,
	//! Cloudflare could not reach the origin server (for example, if the DNS records for the origin server are incorrect or missing)
	HTTP_CODE_523,
	//! Cloudflare was able to complete a TCP connection to the origin server, but did not receive a timely HTTP response
	HTTP_CODE_524,
	//! Cloudflare could not negotiate a SSL/TLS handshake with the origin server
	HTTP_CODE_525,
	//! Cloudflare could not validate the SSL certificate on the origin web server, also used by Cloud Foundry's gorouter
	HTTP_CODE_526,
	//! Cloudflare was unable to resolve the origin hostname, body of the response contains an 1xxx error
	HTTP_CODE_530,
}

/*!
\}
*/
