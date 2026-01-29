class CheckGUIDRequest : JsonApiStruct
{
	string matName;

	void CheckGUIDRequest()
	{
		RegV("matName");
	}
}

class CheckGUIDResponse : JsonApiStruct
{
	bool exists = false;

	void CheckGUIDResponse()
	{
		RegV("exists");
	}
}

class CheckGUID : NetApiHandler
{
	override JsonApiStruct GetRequest()
	{
		return new CheckGUIDRequest();
	}

	override JsonApiStruct GetResponse(JsonApiStruct request)
	{
		CheckGUIDRequest req = CheckGUIDRequest.Cast(request);
		CheckGUIDResponse response = new CheckGUIDResponse();

		// GUID formatting
		ResourceName guid = "{" + req.matName + "}";
		string resName = Workbench.GetResourceName(guid);

		// if the resName is the same as GUID then Workbench couldn't find the resource thus it doesn't exist
		if (resName != guid)
		{
			response.exists = true;
		}

		return response;
	}

}