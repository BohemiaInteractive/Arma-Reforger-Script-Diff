class LayerPresetsRequest : JsonApiStruct
{
	string Input;

	void LayerPresetsRequest()
	{
		RegV("Input");
	}
}

class LayerPresetsResponse : JsonApiStruct
{
	ref array<string> layersPresets;

	void LayerPresetsResponse(array<string> layers)
	{
		this.layersPresets = layers;
	}
	
	override void OnPack()
	{
		StartArray("Layer Presets");
		foreach (string item : layersPresets)
		{
			ItemString(item);
		}
		EndArray();
	}
}


static array<string> GetLayerPresets()
{
	// get project setting conf
	BaseContainer cont = Workbench.GetGameProjectSettings();
	BaseContainerList config = cont.GetObjectArray(EBTContainerFields.conf);

	cont = config.Get(0);
	cont = cont.GetObject("PhysicsSettings");
	cont = cont.GetObject("Interactions");
	config = cont.GetObjectArray("LayerPresets");

	// getting all LayerPresets
	BaseContainer contLayerPresets;
	
	array<string> result = new array<string>();
	
	for (int i = 0; i < config.Count(); i++; )
	{
		contLayerPresets = config.Get(i);
		//response.Layers += contLayerPresets.GetName() + " ";
		result.Insert(contLayerPresets.GetName());
	}
	return result;
}

class LayerPresets : NetApiHandler
{
	override JsonApiStruct GetRequest()
	{
		return new LayerPresetsRequest();
	}

	override JsonApiStruct GetResponse(JsonApiStruct request)
	{
		LayerPresetsRequest req = LayerPresetsRequest.Cast(request);
		return new LayerPresetsResponse(GetLayerPresets());
	}
}


class GetLayerInteractionRequest : JsonApiStruct
{
	string layer;

	void GetLayerInteractionRequest()
	{
		RegV("layer");
	}
}

class GetLayerInteractionResponse : JsonApiStruct
{
	ref array<string> layerPresets = new array<string>;
	
	void GetLayerInteractionResponse()
	{
		RegV("layerPresets");
	}
}

class GetLayerInteraction : NetApiHandler
{
	override JsonApiStruct GetRequest()
	{
		return new GetLayerInteractionRequest();
	}

	override JsonApiStruct GetResponse(JsonApiStruct request)
	{
		GetLayerInteractionRequest req = GetLayerInteractionRequest.Cast(request);
		GetLayerInteractionResponse response = new GetLayerInteractionResponse();
		// opening xob in ResourceManager
		
		BaseContainer cont = Workbench.GetGameProjectSettings();
		BaseContainerList projectSettings = cont.GetObjectArray(EBTContainerFields.conf);
		
		cont = projectSettings.Get(0);
		cont = cont.GetObject("PhysicsSettings");
		cont = cont.GetObject("Interactions");
		BaseContainerList configPresets = cont.GetObjectArray("LayerPresets");
	
		array<string> layers = {};
		cont.Get("Layers", layers);
		
		int index = layers.Find(req.layer);
		for(int i = 0; i < configPresets.Count(); i++)
		{
			BaseContainer layer = configPresets[i];
			int mask;
			layer.Get("Mask", mask);
			
			int shift = mask >> index;
			int result = shift & 1;
			if(result)
			{
				response.layerPresets.Insert(layer.GetName());
			}
		}
		return response;
	}
	
}

