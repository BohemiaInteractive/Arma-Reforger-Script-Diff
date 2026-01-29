class OpenXOBRequest : JsonApiStruct
{
	string path;

	void OpenXOBRequest()
	{
		RegV("path");
	}
}

class OpenXOBResponse : JsonApiStruct
{
	string Output;
	
	void OpenXOBResponse()
	{
		RegV("Output");
	}
}

class OpenXOB : NetApiHandler
{
	override JsonApiStruct GetRequest()
	{
		return new OpenXOBRequest();
	}

	override JsonApiStruct GetResponse(JsonApiStruct request)
	{
		OpenXOBRequest req = OpenXOBRequest.Cast(request);
		OpenXOBResponse response = new OpenXOBResponse();
		// opening xob in ResourceManager
		Workbench.GetModule(ResourceManager).SetOpenedResource(req.path);

		return response;
	}
	
}
