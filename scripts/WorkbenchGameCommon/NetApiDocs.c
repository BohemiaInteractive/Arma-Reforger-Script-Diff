/*!
\defgroup NetApi NetApi
\addtogroup NetApi
\{
1. \ref Page_NetApi
\}
*/

/*!

\page Page_NetApi Workbench NET API

\tableofcontents
\section Page_NetApiDocs_About About
Enfusion Workbench NET API provide network protocol for communication between Workbench and external tools. 
External tool connects to the Workbench NET API endpoint using TCP/IP socket and sends / recieves data according to the Workbench protocol. Sending and receiving `integer` and `string` (UTF-8 encoded) values are supported and each data transaction requires a new TCP/IP connection. `Integer` is represented by 4-byte little endian and `string` is in the Pascal-style string format (first 4-byte integer information repsesents the string length).

Communication is always initiated from the client side. When all the data are passed (based on the protocol) the Workbench sends a response.

\section Page_NetApiDocs_Protocol Protocol
The protocol is proprietary and very simple.

\subsection Page_NetApiDocs_Request Request
Whole message consist of four parts as visualized on the following diagram.

\code
 ┌──────────────┬─────────────────┬──────────────────┬──────────────────────────────────┐
 │ Protocol Ver.│   Client ID     │   Content Type   │       Payload                    │
 │              │                 │                  │                                  │
 │  (Integer)   │   (String)      │    (String)      │       (String)                   │
 └──────────────┴─────────────────┴──────────────────┴──────────────────────────────────┘
\endcode

- **Protocol Ver.** - integer values of the protocol version, currently only version `1` is supported.
- **Client ID** - string representation of the client making the request, for example `PythonClient`.
- **Content Type** - the format of the payload, currently only `JsonRPC` is supported.
- **Payload** - JSON message encoded as UTF-8 string.

\subsubsection Page_NetApiDocs_RequestPayload Payload format
The `JsonRPC` payload format is a JSON encoded as a string, defining what API endpoint to call by its name (`APIFunc`) as well as parameter names and their respective values.

\code{.json}
{
    "APIFunc": "Enforce Script Class implementing NetApiHandler or built-in function name",
    "PARAM1" : "Value1",
    "PARAM2" : 0,
    "PARAM3": {
        "A": true,
        "B": "false"
  },
}
\endcode

Following list contains all the built-in function you can use in the `APIFunc`.

- **OpenResource**
	- required parameters for input:
	\code{.json}
	{
		"APIFunc": "OpenResource",
		"ResourceName": "string", // name of a resource file to open in Workbench
	}
	\endcode
	- response:
	\code{.json}
	{
		"Opened": "bool", // result of the operation
	}
	\endcode
- **BringModuleWindowToFront**
	- required parameters for input:
	\code{.json}
	{
		"APIFunc": "BringModuleWindowToFront",
		"ModuleName": "string", // name of a workbench module
	}
	\endcode
	- response:
	\code{.json}
	{
		// none
	}
	\endcode
- **IsWorkbenchRunning**
	- required parameters for input:
	\code{.json}
	{
		"APIFunc": "IsWorkbenchRunning",
	}
	\endcode
	- response:
	\code{.json}
	{
		"IsRunning": "bool", // is workbench running
		"ScriptsCompiled": "bool", // are scripts compiled successfully
	}
	\endcode
- **IsWorldEditorRunning**
	- required parameters for input:
	\code{.json}
	{
		"APIFunc": "IsWorldEditorRunning",
	}
	\endcode
	- response:
	\code{.json}
	{
		"IsRunning": "bool", // is workbench running
		"ScriptsCompiled": "bool", // are scripts compiled successfully
	}
	\endcode
- **ValidateScripts**
	- required parameters for input:
	\code{.json}
	{
		"APIFunc": "ValidateScripts",
		"Configuration": "string", // script configuration to validate (see project settings, e.g. WORKBENCH, PC, PLAYSTATION, XBOX...)
	}
	\endcode
	- response:
	\code{.json}
	{
	  // array of errors
	  "Errors": [
	    {
	      "error": "Incompatible parameter 'b'", // text of error
	      "file": "scripts/Game/game.c", // file where error occurs
		  "fileAbs": "F:\\DATA\\scripts\\Game\\game.c", // optional, present just for scripts which are unpacked
		  "addon": "MyAddon", // optional, present just for scripts which are unpacked
	      "line": 147 // position in file where error occurs
	    }
	  ],
	  // array of warninigs
	  "Warnings": [
	    {
	      "error": "Variable 'a' is not used", // text of warning
	      "file": "scripts/Game/game.c", // file where warning occurs
		  "fileAbs": "F:\\DATA\\scripts\\Game\\game.c", // optional, present just for scripts which are unpacked
		  "addon": "MyAddon", // optional, present just for scripts which are unpacked
	      "line": 146 // position in file where warning occurs
	    }
	  ],
	  "Success": false
	}
	\endcode


\subsection Page_NetApiDocs_Response Response

Message consists of two parts.

\code
 ┌──────────────┬────────────────────────────────────┐
 │ Error Code   │       Payload                      │
 │              │                                    │
 │  (String)    │       (String)                     │
 └──────────────┴────────────────────────────────────┘
\endcode

- **Error Code** - string representation of the error code.
- **Payload** - JSON message encoded as UTF-8 string.

\subsubsection Page_NetApiDocs_ResponsePayload Payload format
The `JsonRPC` payload format is a JSON encoded as a string, the format is specific for each `APIFunc` which was called in the request.

\section Page_NetApiDocs_CustomEndpoint Custom endpoint
Most of the time you will want or need to implement your own logic of what should be done and return from the Workbench to the caller. In order to do so, you need to implement few things in the Enforce Script and have such script running in a live instance of the Workbench.

1. Implement your own API endpoint by extending `NetApiHandler`.
2. Define your own request (`JsonRPC` format) by extending `JsonApiStruct`.
3. Define your own response by extending `JsonApiStruct`.

\subsection Page_NetApiDocs_ExampleScript Example script
\snippet this NetApi Script Example
*/

#ifdef DOXYGEN

//! [NetApi Script Example]
class ExampleRequest : JsonApiStruct
{
	string input;

	void ExampleRequest()
	{
		RegV("input");
	}
}

class ExampleResponse : JsonApiStruct
{
	string output;
	
	void ExampleResponse()
	{
		RegV("output");
	}
}

// Class that is being called
class ExampleCall : NetApiHandler
{
	override JsonApiStruct GetRequest()
	{
		return new ExampleRequest();
	}

	override JsonApiStruct GetResponse(JsonApiStruct request)
	{
		ExampleRequest req = ExampleRequest.Cast(request);
		ExampleResponse response = new ExampleResponse();
		
		Print(req.input);
		response.output = "Hello from Workbench";

		return response;
	}
	
}
//! [NetApi Script Example]


#endif
