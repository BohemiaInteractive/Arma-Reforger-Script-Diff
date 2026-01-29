class TextureValidationRequest : JsonApiStruct
{
	ref array<string> mats = new array<string>;
	string fbxPath;

	void TextureValidationRequest()
	{
		RegV("mats");
		RegV("fbxPath");
	}
}

class TextureValidationResponse : JsonApiStruct
{
	ref array<string> invalidTextures = new array<string>;
	bool xob = true;
	ref array<string> xobMats = new array<string>;

	void TextureValidationResponse()
	{
		RegV("invalidTextures");
		RegV("xob");
		RegV("xobMats");
	}
}

class TextureValidationUtils
{
	// Getting assigned materials from XOB
	map<string, ResourceName> GetMaterialAssigns(string xobPath, TextureValidationResponse response)
	{	
		map<string,ResourceName> materials = new map<string,ResourceName>();
		EBTEmatUtils ematUtils = new EBTEmatUtils();
		bool meta = ematUtils.GetMaterials(xobPath, materials);
		if(!meta)
		{
			return materials;
		} 
		
		for (int i = 0; i < materials.Count(); i++)
		{
			response.xobMats.Insert(materials.GetKey(i));
		}
		return materials;
	}

	array<string> GetInvalidTextures(ResourceName emat)
	{
		ResourceName temp;
		array<string> invalidText = {};
		ResourceName ematGuid = emat.Substring(0, emat.IndexOf("}")+ 1);

		// check if the Resource in the xob meta corresponds to the GUID of the material
		if (ematGuid.Length() == 18)
		{
			string resName = Workbench.GetResourceName(ematGuid);
			if (ematGuid == resName)
			{
				invalidText.Insert("Emat does not have valid GUID in XOB file");
				return invalidText;
			}
		}
		// load the emat
		Resource resource = Resource.Load(emat);
		BaseContainer ematCont = resource.GetResource().ToBaseContainer();

		for (int i = 0; i < ematCont.GetNumVars(); i++)
		{
			ematCont.Get(ematCont.GetVarName(i), temp);
			if (ematCont.IsVariableSet(ematCont.GetVarName(i)))
			{
				// get textures
				if (temp.Contains(".edds"))
				{
					// check texture GUID the same way as the material one
					ResourceName texGuid = temp.Substring(0, temp.IndexOf("}")+ 1);
					if (texGuid.Length() == 18)
					{
						string resName = Workbench.GetResourceName(texGuid);
						if (texGuid == resName)
						{
							// if the resource name is equal to the GUID that means workbench couldn't find the resource
							invalidText.Insert(temp.Substring(0, temp.Length() - 2));
						}
					}
				}
			}
		}
		return invalidText;
	}
}

class TextureValidation : NetApiHandler
{
	override JsonApiStruct GetRequest()
	{
		return new TextureValidationRequest();
	}

	override JsonApiStruct GetResponse(JsonApiStruct request)
	{
		TextureValidationRequest req = TextureValidationRequest.Cast(request);
		TextureValidationResponse response = new TextureValidationResponse();
		TextureValidationUtils utils = new TextureValidationUtils();
		array<string> invalidTextArr = {};

		// get the material assigns from fbx
		req.fbxPath.Replace(".fbx",".xob");
		map<string, ResourceName> matAssigns = utils.GetMaterialAssigns(req.fbxPath, response);

		// return if no material was assigned
		if (matAssigns.IsEmpty())
		{
			response.xob = false;
			return response;
		}

		// checking all textures per material in the fbx
		foreach (string mat : req.mats)
		{
			string invalidTextStr = string.Empty;
			ResourceName emat = matAssigns.Get(mat);
			// if mat in fbx exists but emat in matAssigns is empty
			if (emat == string.Empty)
			{
				response.invalidTextures.Insert("Material is in FBX but not found in XOB");
				continue;
			}

			// list of all invalid textures for that material
			invalidTextArr = utils.GetInvalidTextures(emat);

			// add all invalid texture errors to the output
			for (int i = 0; i < invalidTextArr.Count(); i++)
			{
				invalidTextStr += invalidTextArr[i];
				if (i != invalidTextArr.Count() - 1)
				{
					invalidTextStr += ", ";
				}
			}
			// insert the error messages of the invalid texture for that material
			response.invalidTextures.Insert(invalidTextStr);
		}
		return response;
	}
}
