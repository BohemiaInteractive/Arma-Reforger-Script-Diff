class EBT_HTTPErrorResponse: JsonApiStruct
{
	string error;
	
	//---------------------------------------------------------------------
	void EBT_HTTPErrorResponse()
	{
		RegV("error");
	}
}

class EBT_HTTPRequest
{
	//---------------------------------------------------------------------
	static void DefaultOnError(RestCallback cb)
	{
	    int code = cb.GetHttpCode();          
	    string data = cb.GetData();       
	    
		EBT_HTTPErrorResponse eResponse = new EBT_HTTPErrorResponse();
		
		eResponse.ExpandFromRAW(data);
		PrintFormat(string.Format("EBT HTTP Request ERROR: %1", eResponse.error), LogLevel.ERROR);
	    
	}
		
	//---------------------------------------------------------------------
	static void Send(string method, string endpoint, string payload, notnull RestCallback callback)
	{
		RestApi api = GetGame().GetRestApi();
		RestContext ctx = api.GetContext("http://localhost:8080/");

		if (method == "POST")
			ctx.POST(callback, endpoint, payload);
		else if (method == "GET")
			ctx.GET(callback, endpoint);
		else
			PrintFormat("Unsupported HTTP method: %1", method);
	}
}