/*
===========================================
Do not modify, this script is generated
===========================================
*/

#ifdef WORKBENCH

/*!
\addtogroup NetApi
\{
*/

/*!
Base class for NetApi handlers. To implement a custom NetApi handler, inherit from the base NetApiHandler class. The name of your derived class determines the name of the corresponding NetApi function.

NetApi Call Lifecycle:

1. The NetApi call is received by the framework.
2. The system searches for a subclass of NetApiHandler whose name matches the requested function name.
3. Once the appropriate handler is found, its GetRequest() method is invoked.
4. The request data is deserialized into a JsonApiStruct object.
5. The handler's GetResponse() method is called, using the populated request structure.
6. The method returns a JsonApiStruct response, which is sent back to the caller.

\see \ref Page_NetApi
\see \ref Page_NetApiDocs_ExampleScript
*/
class NetApiHandler
{
	//! override to create custom request data
	JsonApiStruct GetRequest();
	//! override to create custom respond data
	JsonApiStruct GetResponse(JsonApiStruct request);

}

/*!
\}
*/

#endif // WORKBENCH
