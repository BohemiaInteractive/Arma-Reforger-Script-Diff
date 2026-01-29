class ValidateMaterialPlugin
{	
	static ref array<ResourceName> materials = new array<ResourceName>;
	static ref array<ResourceName> textures = new array<ResourceName>;
};


class MaterialValidatorUtils
{				
	// These dependencies are not using RGB colors
	void CheckDependencies(BaseContainer mat, out MaterialValidatorResponse material_validator)
	{
		// dictionary Key(Map) -> Value(Params) that depends on that map
		map<string,ref array<string>> dependencies = new map<string, ref array<string>>();
		dependencies["NMOMap"] = {"NormalPower"};
		dependencies["DetailNormalMap"] = {"DetailNormalPower"};
		dependencies["GlobalNMOMap"] = {"NormalPower"};
		dependencies["MudNMOMap"] = {"MudDetNormal"};
		dependencies["NMO_4"] = {"DetNormal_4"};
		dependencies["NMO_3"] = {"DetNormal_3"};
		dependencies["NMO_2"] = {"DetNormal_2"};
		dependencies["NMO_1"] = {"DetNormal_1"};
		dependencies["MaskMap"] = {"MaskRedScale","MaskGreenScale","MaskBlueScale"};
		dependencies["NTCMap"] = {"NormalPower"};
		dependencies["NMOMap2"] = {"NormalPower2"};
		dependencies["DetailNormalMap2"] = {"DetailNormalPower2"};
		
		
		for(int i = 0; i < dependencies.Count(); i++)
		{
			string mapVarname = dependencies.GetKey(i);
			// if map exists in the shader
			if(mat.GetVarIndex(mapVarname) != -1)
			{
				string mapValue;
				string paramValue;
				string defaultValue;
				array<string> depParam = dependencies.Get(mapVarname);
				// iterate through all params of that map
				foreach(string paramVarname: depParam)
				{
					// get values from the material
					mat.Get(mapVarname, mapValue);
					mat.Get(paramVarname, paramValue);
					mat.GetDefaultAsString(paramVarname, defaultValue);
					// Map is Null and Param is set with some value that is not zero
					if(mapValue == "" && (paramValue != "" && paramValue != "0" && paramValue != defaultValue))
					{
						material_validator.reports.Insert(paramVarname + " was directly set but " + mapVarname + " is missing");
						material_validator.severity.Insert(2);
					}
					// Map is set
					else if(mapValue != "")
					{
						// get the value of the param even if default
						GetValue(mat,paramVarname, paramValue);
						// param is 0 and map is set
						if(paramValue == "0")
						{
							material_validator.reports.Insert(mapVarname + " was directly set but " + paramVarname + " is 0");
							material_validator.severity.Insert(2);
						}
					}
				}
			}
		}
		return;
	}
	// Dictionary what Slots can be used for each suffix
	map<string,ref array<string>> GetTextureConnections()
	{
		map<string,ref array<string>> suffixToSlots = new map<string,ref array<string>>();
		suffixToSlots["BCR"] = {"BCRMap","BCR_1","BCR_2","BCR_3","BCR_4","MudBCRMap","DirtBCRMap","BCRMap2","EmissiveMap"};
		suffixToSlots["NMO"] = {"NMOMap","NMO_1","NMO_2","NMO_3","NMO_4","MudNMOMap","NMOMap2","GlobalNMOMap","DetailNormalMap","DetailNormalMap2"};
		suffixToSlots["N"] = suffixToSlots.Get("NMO");
		suffixToSlots["MCR"] = {"MCRMap","GlobalMCRMap","GlobalMCMap","MacroMap"};
		suffixToSlots["NHO"] = suffixToSlots.Get("NMO");
		suffixToSlots["MC"] = suffixToSlots.Get("MCR");
		suffixToSlots["MC"].Insert("GlobalMCMap");
		suffixToSlots["NTC"] = {"NTCMap"};
		suffixToSlots["MLOD"] = {"BCRMap"};
		suffixToSlots["VFX"] = {"VFX","VFXMap"};
		suffixToSlots["MASK"] = {"MaskMap","Mask_2","Mask_3","Mask_4","DirtMaskMap","MudMaskMap","SubsurfaceMask"};
		suffixToSlots["A"] = {"OpacityMap","HeightMap"};
		suffixToSlots["CRM"] = {"CamoCRMMap"};
		suffixToSlots["EM"] = {"EmissiveMap","ProjectionMap"};
		suffixToSlots["H"] = {"HeightMap","PuddleHeightMap"};
		return suffixToSlots;
	}
	
	// Checks if the right map is in the right slot (ex. _BCR/_MLOD in BCR Map)
	void CheckSlots(string slot, string suffix, out MaterialValidatorResponse material_validator)
	{			
		// Dictionary SUFFIX:[SLOT,SLOT,SLOT,..]
		map<string,ref array<string>> connections = GetTextureConnections();

				
		// get value of the suffix key (What slots can be used for the suffix)
		array<string> slots = connections.Get(suffix);
		// If array not null => Suffix is supported
		// and if it doesnt contain the slot, it is wrong
		if(slots && !slots.Contains(slot))
		{
			material_validator.reports.Insert("Texture with suffix " + suffix + " is in " + slot);
			material_validator.severity.Insert(3);
		}
	}
	
	
	// Checks parameters that should stay as default values(General tab, Wetness, etc..)
	void CheckDefaults(BaseContainer cont,out MaterialValidatorResponse material_validator)
	{
		//https://confluence.bistudio.com/display/~phammac/Typos+in+Workbanch+parametrs
		//DisableUserAphaInShadow
		// Doesnt need to clear the array, since its initialized everytime the function is called
		array<string> DefaultProperties = {"ZEnable", "ZBias", "Cull", "Sort", "SortBias", "CastShadow", "ReceiveShadow", "OccludeRain", 
		"OccludeWetness", "AddWetness", "DisableFog", "DisableInteriorProbes", "TCModFuncs", "NoDecals", "EnableClutter", "ClutterThresholdAngle", 
		"CrossFade", "WetnessScale", "PorosityScale", "CustomWetnessChannel", "SlidingDropsVisibility" };
		
		array<string> allDefaultProperties = {"ZEnable","ZBias","Cull","Sort","SortBias","CastShadow","ReceiveShadow","OccludeRain","OccludeWetness",
		"AddWetness","DisableFog","DisableInteriorProbes","TCModFuncs","NoDecals","EnableClutter","ClutterThresholdAngle","CrossFade",
		"WetnessScale","PorosityScale","CustomWetnessChannel","SlidingDropsVisibility","ZWrite","AlphaTest","AlphaMul","BlendMode","AllowUserAlphaBias","BisableUserAphaInShadow","ManualCascadesShadows"};
		string matClass = cont.GetClassName();
		for(int i = 0; i < cont.GetNumVars(); i++)
		{
			string var = cont.GetVarName(i);
			// if the prop is in General props that should be checked, check if it's set directly
			if(allDefaultProperties.Contains(var) && cont.IsVariableSetDirectly(var))
			{
				material_validator.reports.Insert(var + " doesn't match the default settings");
				material_validator.severity.Insert(1);
			}
		}
	}
	
	// Cont.Get but it if it's not set it will return default, not empty string
	void GetValue(BaseContainer cont, string varName, out string value)
	{
		// get value the normal way
		cont.Get(varName, value);
		// if the value is empty and variable is not set get default
		if(!cont.IsVariableSetDirectly(varName) && value == "")
		{
			cont.GetDefaultAsString(varName, value);
		}
		return;
	}
	
		
	// Checking all validations for all textures on material
	void CheckTextures(BaseContainer cont, out MaterialValidatorResponse material_validator)
	{
		array<string> slots = {};
		string absPath;
		// going through properties of emat
		for(int i = 0; i < cont.GetNumVars(); i++)
		{
			string var = cont.GetVarName(i);
			ResourceName value;
			cont.Get(var, value);
			// get edds in emat
			if(value.Contains(".edds"))
			{
				// texture value will be with anim, removing that
				if(value.EndsWith(" 0"))
				{
					value = value.Substring(0, value.Length() - 2);
				}	
				
				// checking GUID
				string guid = value.Substring(0,18);
				string guidtest = Workbench.GetResourceName(guid);
				if(guid == guidtest)
				{
					material_validator.reports.Insert(cont.GetResourceName() + " has a wrong guid!");
					material_validator.severity.Insert(3);
					return;
				}
				
				// meta of the edds to get the right extesion
				ResourceManager resourceManager = Workbench.GetModule(ResourceManager);
				MetaFile meta = resourceManager.GetMetaFile(value.GetPath());
				if(!meta)
				{
					material_validator.reports.Insert("Metafile for " + value + "(" + var +  ") couldn't be found!");
					material_validator.severity.Insert(3);
					continue;
				}
				BaseContainerList configurations = meta.GetObjectArray("Configurations");
				BaseContainer cfg = configurations.Get(0);
				
				// getting extension from edds
				string extension = cfg.GetClassName().Substring(0,3);
				// need to get relative path for texture validator class	
				Workbench.GetAbsolutePath(value.GetPath(),absPath);
				extension.ToLower();
				absPath.Replace("edds",extension);
				
				// if texture was selected and also in a material that was selected, the texture errors will be reported twice
				// this removes the texture from validating textures itself and validates it only once with the material
				if(ValidateMaterialPlugin.textures.Contains(absPath))
				{
					ValidateMaterialPlugin.textures.RemoveItem(absPath);
				}
				
				
				// absolute path of texture with right extension(.tif, etc..) for python
				material_validator.abs_textures.Insert(absPath);
				// texture suffix and its slot
				string suffix = FilePath.StripPath(value);
				
				suffix = suffix.Substring(suffix.LastIndexOf("_")+1, suffix.LastIndexOf(".") - suffix.LastIndexOf("_")-1);
				string slot = var;
				slots.Insert(slot);	
				// this is removing int from Mask1 => Mask. But I can add Mask1/2 into the dict..
				if(suffix[suffix.Length() - 1].ToInt())
				{
					suffix = suffix.Substring(0,suffix.Length()-1);
				}
				
				CheckSlots(slot,suffix, material_validator);
			}
		}
		return;
	}
		
	// Check extreme values of properties (NormalPower,etc...)
	void CheckExtremes(BaseContainer cont, float min, float max,out MaterialValidatorResponse material_validator)
	{
		array<string> properties = {"NormalPower","MudDetNormal","DetNormal_1","DetNormal_2","DetNormal_3","DetNormal_4","DetailNormalPower","DetailNormalPower2",
		"DetailMaskSharpness","DetailMaskOffset","DetailMaskUVScale","NormalPower2","RoughnessScale2","MetalnessScale2","RoughnessScale","MetalnessScale",
		"MudRoughness","MudMetalness","Roughness1","Metalness_1","Roughness_2","Metalness_2","Roughness_3","Metalness_3","Roughness_4","Metalness_4",
		"AO_1","AO_2","AO_3","AO_4",};
		foreach(string property: properties)
		{
			string value;
			// have to check if the property exists in the shader
			if(cont.GetVarIndex(property) != -1)
			{
				cont.Get(property,value);
				if(value != "" || cont.IsVariableSetDirectly(property))
				{
					float limitMin;
					float limitMax;
					float step;
					cont.GetLimits(cont.GetVarIndex(property),limitMin,limitMax, step);
					float newMin = limitMax * (min / 100);
					float newMax = limitMax * (max / 100);
					if((value.ToFloat() < newMin || value.ToFloat() > newMax) && (value.ToFloat() != 0))
					{
						material_validator.reports.Insert(property + " is out of limitation(" + newMin + "-" + newMax + ")");
						material_validator.severity.Insert(2);
					}	
				}
			}
		}
	}
	
	// Validates material UVs (Unified, Extremes)
	void ValidateUVs(BaseContainer cont, float MinTiling, float MaxTiling,out MaterialValidatorResponse material_validator)
	{
		BaseContainer empty;
		
		for(int i = 0; i < cont.GetNumVars(); i++)
		{
			string var = cont.GetVarName(i);
			// getting object
			if(cont.GetObject(var) != empty)
			{
				BaseContainer uvTiling = cont.GetObject(var);
				// checking if the object is UV Transforms
				if(uvTiling.GetClassName() == "MatUVTransform")
				{
					string tilingU, tilingV; 
					GetValue(uvTiling, "TilingU", tilingU);
					GetValue(uvTiling, "TilingV", tilingV);
					// UV tiling uniform
					if(tilingU != tilingV)
					{
						material_validator.reports.Insert("UV Tiling on " + var + " is not uniform.");
						material_validator.severity.Insert(2);
					}
					// UV tiling limits
					if(MinTiling > tilingU.ToFloat() || MaxTiling < Math.AbsFloat(tilingU.ToFloat()))
					{
						material_validator.reports.Insert("UV Tiling on " + var + " is out of limitation, U = " + tilingU);
						material_validator.severity.Insert(2);
					}
					if(MinTiling > tilingV.ToFloat() || MaxTiling < Math.AbsFloat(tilingV.ToFloat()))
					{
						material_validator.reports.Insert("UV Tiling on " + var + " is out of limitation, V = " + tilingV);
						material_validator.severity.Insert(2);
					}
					
				}
			}
			string value; 
			cont.Get(var, value);
		}
		return;
	}
}

class MaterialValidatorRequest: JsonApiStruct
{
	int mat_index;
	void MaterialValidatorRequest()
	{
		RegV("mat_index");		
	}
};


class MaterialValidatorResponse: JsonApiStruct
{
	ref array<string> abs_textures = new array<string>();
	ref array<string> reports = new array<string>();
	ref array<int> severity = new array<int>();
	string mat_name;
	
	void MaterialValidatorResponse()
	{
		RegV("abs_textures");
		RegV("reports");
		RegV("severity");
		RegV("mat_name");
	}
};

class MaterialValidator: NetApiHandler
{
	override JsonApiStruct GetRequest()
	{
		return new MaterialValidatorRequest();
	}
	
	override JsonApiStruct GetResponse(JsonApiStruct request)
	{
		MaterialValidatorRequest req = MaterialValidatorRequest.Cast(request);
		MaterialValidatorResponse material_report = new MaterialValidatorResponse();
		MaterialValidatorUtils matValid = new MaterialValidatorUtils();

		Resource resource = Resource.Load(ValidateMaterialPlugin.materials[req.mat_index]);
		BaseContainer material = resource.GetResource().ToBaseContainer();
		material_report.mat_name = material.GetName();
		
		int uv_min = 0;
		int uv_max = 5;
		int ext_min = 5;
		int ext_max = 95;
		
		matValid.ValidateUVs(material, uv_min, uv_max, material_report); //Done
		matValid.CheckDefaults(material, material_report);
		matValid.CheckExtremes(material,ext_min,ext_max, material_report);
		matValid.CheckDependencies(material, material_report);
		matValid.CheckTextures(material, material_report);
		return material_report;
	}
}



			
class TextureValidatorRequest: JsonApiStruct
{
	string texture_path;
	void TextureValidatorRequest()
	{
		RegV("texture_path");
	}
};

class TextureValidatorResponse: JsonApiStruct
{	
	ref array<string> reports = new array<string>();
	ref array<int> severity = new array<int>();
	
	void TextureValidatorResponse()
	{
		RegV("reports");
		RegV("severity");
	}
};

class TextureValidator: NetApiHandler
{
	
	void TextureImportSettings(string abs_texture_path, out TextureValidatorResponse checks)
	{
		ResourceManager resourceManager = Workbench.GetModule(ResourceManager);
		MetaFile meta = resourceManager.GetMetaFile(abs_texture_path);
		
		BaseContainerList configurations = meta.GetObjectArray("Configurations");
		BaseContainer cfg = configurations.Get(0);
		
		for(int i = 0; i < cfg.GetNumVars(); i++)
		{
			string var = cfg.GetVarName(i);
			if(cfg.IsVariableSetDirectly(var))
			{
				checks.reports.Insert("Has some changes in default import config!");
				checks.severity.Insert(1);
			}
		}
		
		TextureTypes textureTypes = new TextureTypes();
		TextureType type = textureTypes.FindTextureType(abs_texture_path);

		if(type.m_PostFix == string.Empty)
		{
			checks.reports.Insert("Has invalid suffix!");
			checks.severity.Insert(3);
		}
		return;
	}
	
	override JsonApiStruct GetRequest()
	{
		return new TextureValidatorRequest();
	}
	
	override JsonApiStruct GetResponse(JsonApiStruct request)
	{
		TextureValidatorRequest req = TextureValidatorRequest.Cast(request);
		TextureValidatorResponse response = new TextureValidatorResponse();
	
		TextureImportSettings(req.texture_path, response);
		return response;
	}
}