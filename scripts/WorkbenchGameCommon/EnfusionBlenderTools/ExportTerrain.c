class ExportTerrainRequest : JsonApiStruct
{
	int area;
	string worldPath;
	string path;
	int count;

	void ExportTerrainRequest()
	{
		RegV("area");
		RegV("worldPath");
		RegV("path");
		RegV("count");
	}
}

class ExportTerrainResponse : JsonApiStruct
{
	bool Output;

	void ExportTerrainResponse()
	{
		RegV("Output");
	}
}


class ExportTerrain : NetApiHandler
{
	override JsonApiStruct GetRequest()
	{
		return new ExportTerrainRequest();
	}

	override JsonApiStruct GetResponse(JsonApiStruct request)
	{
		ExportTerrainRequest req = ExportTerrainRequest.Cast(request);
		ExportTerrainResponse response = new ExportTerrainResponse();

		// getting WorldEditor API
		WorldEditor wm = Workbench.GetModule(WorldEditor);
		WorldEditorAPI api = wm.GetApi();
		string openedWorldPath;

		// if WorldEditor is not opened, open it and load correct map
		if (!api)
		{
			Workbench.OpenModule(WorldEditor);
			api = wm.GetApi();
			wm.SetOpenedResource(req.worldPath);
		}

		// if WE is opened check if with a correct map if not open it
		api.GetWorldPath(openedWorldPath);
		if (openedWorldPath != req.worldPath)
		{
			wm.SetOpenedResource(req.worldPath);
		}
		
		FileHandle bin = FileIO.OpenFile(req.path, FileMode.READ);
		for(int i = 0; i < req.count; i++)
		{
			array<float> coords = new array<float>;
			array<int> indices = new array<int>;
			
			bin.ReadArray(indices, 4, 2);
			bin.ReadArray(coords, 4, req.area);
			
			api.BeginTerrainAction(TerrainToolType.HEIGHT_EXACT);
			api.SetTerrainSurfaceTile(0, indices[0], indices[1], coords);
			api.EndTerrainAction();
		}
		
		
		bin.Close();
		response.Output = true;
		return response;
	}

}
