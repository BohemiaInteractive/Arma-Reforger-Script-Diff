#ifdef WORKBENCH

class EBTAddonInfo : JsonApiStruct
{
	string version;
	ref array<string> loadedModules = new array<string>();
	
	void EBTAddonInfo()
	{
		RegV("version");
		RegV("loadedModules");
	}
}

class EBTInfoRequest : JsonApiStruct
{
	string title;
	string msg;
	
	void EBTInfoRequest()
	{
		
		RegV("title");
		RegV("msg");
	}

}

class EBTInfoDialog
{
	[ButtonAttribute("OK")]
	void OkButton(){}
}

class EBTInfo : NetApiHandler
{
	//------------------------------------------------------------------------------------------------
	override JsonApiStruct GetRequest()
	{
		return new EBTInfoRequest();
	}

	//------------------------------------------------------------------------------------------------
	override JsonApiStruct GetResponse(JsonApiStruct request)
	{
		EBTInfoRequest req = EBTInfoRequest.Cast(request);
		EBTInfoDialog dialog = new EBTInfoDialog();
		
		Workbench.ScriptDialog(req.title, req.msg, dialog);
		

		return null;
	}
}


#endif