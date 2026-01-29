#ifdef WORKBENCH

class EBTContainerFields
{	
	// EMAT
	static const string conf = "Configurations";
	static const string materialAssigns = "MaterialAssigns";
	static const string UVTransform = "MatUVTransform";
	// EDDS
	static const string conversion = "Conversion";
	static const string colorSpace = "ColorSpace";
	static const string mips = "ContainsMips";
	static const string rgHQCompression = "RedGreenHQCompression";
	static const string colorHQCompression = "ColorHQCompression";
	static const string tosRGB = "TosRGB";
	static const string toLinear = "ToLinear";
	static const string raw = "Raw";
	static const string sRGB = "sRGB";
	static const string redHQCompression = "RedHQCompression";
	//PREFAB
	static const string coords = "coords";
	static const string angles = "angles";
	static const string scale = "scale";
	static const string pivot = "PivotID";
	static const string hierarchy = "Hierarchy";
	static const string components = "components";
	static const string meshObject = "MeshObject";
	static const string object = "Object";
	static const ref array<string> supportedTypes = {"GenericEntity", "Building", "GameEntity", "StaticModelEntity"};
}


class GetRelativePathRequest : JsonApiStruct
{
	string absPath;
	
	void GetRelativePathRequest()
	{
		RegV("absPath");
	}
}

class GetRelativePathResponse : JsonApiStruct
{
	string relPath;
	string errorString;
	
	void GetRelativePathResponse()
	{
		RegV("relPath");
		RegV("errorString");
	}
}


class GetRelativePath : NetApiHandler
{
	//------------------------------------------------------------------------------------------------
	override JsonApiStruct GetRequest()
	{
		return new GetRelativePathRequest();
	}

	//------------------------------------------------------------------------------------------------
	override JsonApiStruct GetResponse(JsonApiStruct request)
	{
		GetRelativePathRequest req = GetRelativePathRequest.Cast(request);
		GetRelativePathResponse res = GetRelativePathResponse();
		
		if (req.absPath.StartsWith("{"))
		{
			res.relPath = req.absPath;
			return res;
		}
		
		ResourceManager resourceManager = Workbench.GetModule(ResourceManager);
		MetaFile metaFile = resourceManager.GetMetaFile(req.absPath);
		
		if (!metaFile)
		{
			res.errorString = "No Resource found at given absolute path " + req.absPath;
			return res;
		}
		
		res.relPath = metaFile.GetResourceID();
		

		return res;
	}
}

class EBTEmatUtils
{
	bool GetMaterials(string xobPath, out map<string,ResourceName> AssignedMats)
	{
		// Get xob meta file
		ResourceManager resourceManager = Workbench.GetModule(ResourceManager);
		MetaFile meta = resourceManager.GetMetaFile(xobPath);
		// return false if meta is missing
		if(!meta)
		{
			return false;
		}
		// Get assigned materials from Meta file
		BaseContainerList configurations = meta.GetObjectArray(EBTContainerFields.conf);
		BaseContainer cfg = configurations.Get(0);
		string materialAssigns;
		array<string> pairs = new array<string>;
		cfg.Get(EBTContainerFields.materialAssigns, materialAssigns);
		
		// format them into map and pair correct name - path
		materialAssigns.Split(";", pairs, true);
		foreach (string pair : pairs)
		{
			array<string> keyValue = new array<string>;
			pair.Split(",", keyValue, true);
			AssignedMats.Set(keyValue[0], keyValue[1]);
		}
		return true;
	}
	
	
}

#endif