class GetPortalMatRequest : JsonApiStruct
{

	ref array<int> matHeights = new array<int>;
	ref array<int> matWidths = new array<int>;
	ref array<string> socketNames = new array<string>;

	void GetPortalMatRequest()
	{
		RegV("matHeights");
		RegV("matWidths");
		RegV("socketNames");
	}
}

class GetPortalMatResponse : JsonApiStruct
{
	ref array<string> matNames = new array<string>;

	void GetPortalMatResponse()
	{
		RegV("matNames");
	}
}

class GetPortalMat : NetApiHandler
{
	
	ref array<string> Portals = new array<string>();
	
	override JsonApiStruct GetRequest()
	{
		return new GetPortalMatRequest();
	}

	override JsonApiStruct GetResponse(JsonApiStruct request)
	{
		GetPortalMatRequest req = GetPortalMatRequest.Cast(request);
		GetPortalMatResponse response = new GetPortalMatResponse();

		// get all portal materials from assets
		if(Portals.Count() == 0)
		{
			SearchResourcesFilter filter = new SearchResourcesFilter();
			filter.rootPath = "$ArmaReforger:Assets";
			filter.fileExtensions = {"emat"};
			
			ResourceDatabase.SearchResources(filter, Find);
		}
		
		for (int i = 0; i < req.socketNames.Count(); i++)
		{
			// creating name with defined sizes
			string name = "PRT_" + req.matWidths[i] + "x" + req.matHeights[i];
			foreach (string portal : Portals)
			{
				// if the name with desired sizes was found add the matName + GUID to array
				if (FilePath.StripPath(portal).Contains(name))
				{
					ResourceManager resourceManager = Workbench.GetModule(ResourceManager);
					MetaFile meta = resourceManager.GetMetaFile(portal);
					string guid = meta.GetResourceID().Substring(1, 16);
					string matName = FilePath.StripExtension(FilePath.StripPath(portal)) + "_" + guid;
					response.matNames.Insert(matName);
				}
			}

			// if not add null
			if (i >= response.matNames.Count())
			{
				response.matNames.Insert("");
			}
		}
		return response;
	}
	
	void Find(ResourceName resName, string filePath)
	{
		// check if its a Portal material and if it meets naming convention so we could link the dimensions
		Resource resource = Resource.Load(resName);
		BaseContainer ematCont = resource.GetResource().ToBaseContainer();
		if(ematCont.GetClassName() == "MatLightPortal" && resName.Contains("PRT_"))
		{
			Portals.Insert(filePath);
		}
	}
}










