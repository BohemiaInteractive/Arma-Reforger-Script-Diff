[WorkbenchPluginAttribute(
	name: "Generate addon.ci-info files",
	description: "Generates addon metadata for external tools to know how to build data all at once or in parts, and saves it into JSON file.",
	wbModules: {"ResourceManager"},
	awesomeFontCode: 0xF6E3
)]
class AddonBuildInfoTool : WorkbenchPlugin
{
	private ref array<string> m_EntFiles;

	/*
		Build-NavMesh-High: .nmn + .ntile
		Build-NavMesh-Low:  .nmn + .ntile
		Build-ShoreMap:     .smd
		Build-SoundMap:     .smap

		Build-FlowMap:
			* generates .Rivers/{GUID}_flow.edds
			* command line:
				* -wbmodule=ResourceManager
				* -plugin=GenerateFlowMaps
				* -world=${srcDir}/${entFilePath}
				* -saveDir=${dstDir}/${entDirPath}

	 */

	static bool FindAddonByAbsolutePath(
		string queryAbsolutePath,
		out string outAddonGuid,
		out string outAddonId,
		out string outAddonExactRoot
	)
	{
		if (!FilePath.IsAbsolutePath(queryAbsolutePath))
			return false;

		queryAbsolutePath = FilePath.ToInternalFormat(queryAbsolutePath);
		if (!queryAbsolutePath.EndsWith("/"))
			queryAbsolutePath += "/";

		array<string> addonGuids = {};
		GameProject.GetLoadedAddons(addonGuids);

		foreach(string addonGuid : addonGuids)
		{
			string addonId = GameProject.GetAddonID(addonGuid);
			string addonExactRoot = string.Format("$%1:", addonId);
			string addonAbsolutePath;
			if (!Workbench.GetAbsolutePath(addonExactRoot, addonAbsolutePath))
				continue;

			if (!addonAbsolutePath.EndsWith("/"))
				addonAbsolutePath += "/";

			if (addonAbsolutePath.Compare(queryAbsolutePath, caseSensitive: false) == 0)
			{
				outAddonGuid = addonGuid;
				outAddonId = addonId;
				outAddonExactRoot = addonExactRoot;
				return true;
			}
		}

		return false;
	}

	private void OnEntFileFound(ResourceName resName, string file)
	{
		m_EntFiles.Insert(resName.GetPath());
	}

	private bool TryExtractGUID(ResourceName resName, out string guid)
	{
		int guidBeg = resName.IndexOf("{");
		int guidEnd = resName.IndexOf("}");
		if (guidBeg != -1 && guidEnd != -1)
		{
			guidBeg += 1;
			guid = resName.Substring(guidBeg, guidEnd - guidBeg);
			return true;
		}
		else
		{
			return false;
		}
	}

	void GenerateCiInfo(ResourceManager rm, FileHandle outputFile, string addonExactRoot, string addonID)
	{
		Print(addonExactRoot);
		Print(addonID);
		m_EntFiles = {};
		SearchResourcesFilter filter = new SearchResourcesFilter();
		filter.fileExtensions = {"ent"};
		filter.rootPath = addonExactRoot;
		ResourceDatabase.SearchResources(filter, OnEntFileFound);

		auto result = new map<string, ref array<ref WorldBuildMeta>>();

		foreach (string entFile : m_EntFiles)
		{
			Print(entFile);
			string entExactPath = addonExactRoot + entFile;
			MetaFile mf = rm.GetMetaFile(entExactPath);
			BaseContainerList configs = mf.GetObjectArray("Configurations");
			for (int i = 0, count = configs.Count(); i < count; i++)
			{
				BaseContainer config = configs[i];
				string resourceID;
				if (!TryExtractGUID(mf.GetResourceID(), resourceID))
				{
					PrintFormat("Couldn't extract GUID from resource name '%1'", mf.GetResourceID());
					continue;
				}

				string configName = config.GetName();
				array<string> buildTags;
				config.Get("BuildTags", buildTags);
				if (!buildTags.IsEmpty())
				{
					WorldBuildMeta wbm = new WorldBuildMeta();
					wbm.id = resourceID;
					wbm.path = entFile;
					wbm.tags = buildTags;
					array<ref WorldBuildMeta> resultList;
					if (!result.Find(configName, resultList))
					{
						resultList = {};
						result.Set(configName, resultList);
					}
					resultList.Insert(wbm);
				}
			}
		}

		string json = "{";
		bool separateConfigs = false;
		foreach (string k, array<ref WorldBuildMeta> v : result)
		{
			if (separateConfigs)
				json += ",";

			json += string.Format("\"%1\":{", k);
			bool separateWbm = false;
			foreach (WorldBuildMeta wbm : v)
			{
				if (separateWbm)
					json += ",";

				json += string.Format(
					"\"%1\": {\"name\":\"%2\",\"path\":\"%3\",\"tags\":[",
					wbm.id,
					FilePath.StripExtension(FilePath.StripPath(wbm.path)),
					wbm.path
				);
				bool separateTags = false;
				foreach (string tag : wbm.tags)
				{
					if (separateTags)
						json += ",";
					json += string.Format("\"%1\"", tag);
					separateTags = true;
				}
				json += "]}";
				separateWbm = true;
			}
			json += "}";
			separateConfigs = true;
		}
		json += "}";

		outputFile.WriteLine(json);
	}

#ifdef AddonBuildInfoTool_IN_UI
	override void Run()
	{
		ResourceManager rm = Workbench.GetModule(ResourceManager);
		array<string> addonGUIDs = {};
		GameProject.GetLoadedAddons(addonGUIDs);

		foreach(string addonGUID : addonGUIDs)
		{
			string addonID = GameProject.GetAddonID(addonGUID);
			string addonExactRoot = string.Format("$%1:", addonID);
			FileHandle outputFile = FileIO.OpenFile(addonExactRoot + "addon.ci-info", FileMode.WRITE);
			if (!outputFile.IsOpen())
				continue;

			GenerateCiInfo(rm, outputFile, addonExactRoot, addonID);
			outputFile.Close();
		}
	}
#endif

	override void RunCommandline()
	{
		ResourceManager rm = Workbench.GetModule(ResourceManager);
		string addonPathCli;
		if (!rm.GetCmdLine("-addonPath", addonPathCli))
		{
			Print("Missing argument 'addonPath', which is required AddonBuildInfoTool plugin.", level: LogLevel.ERROR);
			Workbench.Exit(1);
			return;
		}
		if (!FilePath.IsAbsolutePath(addonPathCli))
		{
			PrintFormat("Argument 'addonPath' must be an absolute path, but its value is '%1'", addonPathCli, level: LogLevel.ERROR);
			Workbench.Exit(2);
			return;
		}

		string addonGuid, addonId, addonExactRoot;
		bool found = AddonBuildInfoTool.FindAddonByAbsolutePath(
			addonPathCli, addonGuid, addonId, addonExactRoot
		);
		if (!found)
		{
			PrintFormat("Could not find addon matching path '%1'.", addonPathCli);
			Workbench.Exit(3);
		}

		string ciInfoDstCli;
		if (!rm.GetCmdLine("-ciInfoDst", ciInfoDstCli))
		{
			Print("Missing argument 'ciInfoDst' for AddonBuildInfoTool plugin.", level: LogLevel.ERROR);
			Workbench.Exit(4);
			return;
		}
		ciInfoDstCli = FilePath.ToInternalFormat(ciInfoDstCli);

		FileHandle outputFile = FileIO.OpenFile(ciInfoDstCli, FileMode.WRITE);
		if (!outputFile.IsOpen())
		{
			PrintFormat("Couldn't open file '%1' for writing.", ciInfoDstCli, level: LogLevel.ERROR);
			Workbench.Exit(5);
			return;
		}

		GenerateCiInfo(rm, outputFile, addonExactRoot, addonId);
		outputFile.Close();
		Workbench.Exit(0);
	}
}

class WorldBuildMeta
{
	string id;
	string path;
	ref array<string> tags;
}
