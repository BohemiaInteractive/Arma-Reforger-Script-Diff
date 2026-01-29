#ifdef WORKBENCH
enum SCR_ESearchXOBPluginSearchType
{
	ListAllXOBs,
	LoadAllXOBs,
	InvalidMask,
	LayerPreset,
	LayerMask,
}

enum SCR_ESearchXOBPluginLayerFilter
{
	ANY,
	ALL,
	EXACT,
	NONE,
}

class SCR_SearchXOBFunctor
{
	protected ref array<ResourceName> m_aFiles;
	protected int m_iCounter;
	protected bool m_bPrintFiles;
	protected bool m_bIgnoreNoColliders;
	protected ResourceName m_sSearchedDirectory;
	protected int m_iAddon;

	//------------------------------------------------------------------------------------------------
	//!
	void Search()
	{
		WBProgressDialog progress = new WBProgressDialog("Searching...", null);

		PrintHeader();

		string addon = SCR_AddonTool.ToFileSystem(SCR_AddonTool.GetAddonID(m_iAddon));

		m_aFiles = SCR_WorkbenchHelper.SearchWorkbenchResources({ "xob" }, null, addon + m_sSearchedDirectory.GetPath());
		int count = m_aFiles.Count();
		int updateMask = SCR_Math.IntegerMask(count * 0.1);

		foreach (int i, ResourceName file : m_aFiles)
		{
			ProcessFile(file);

			if ((i & updateMask) == updateMask)
				progress.SetProgress(i / count);
		}

		PrintResults();
		m_aFiles.Clear();
	}

	// methods to be overridden
	protected void PrintHeader();
	protected void ProcessFile(ResourceName file);
	protected void PrintResults();

	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] printFiles
	//! \param[in] ignoreNoColliders
	//! \param[in] searchedDir
	//! \param[in] searchedAddon
	void SCR_SearchXOBFunctor(bool printFiles, bool ignoreNoColliders, ResourceName searchedDir, int searchedAddon)
	{
		m_aFiles = {};
		m_bPrintFiles = printFiles;
		m_bIgnoreNoColliders = ignoreNoColliders;
		m_sSearchedDirectory = searchedDir;
		m_iAddon = searchedAddon;
	}
}

class SCR_SearchXOBListAllFunctor : SCR_SearchXOBFunctor
{
	//------------------------------------------------------------------------------------------------
	override void PrintHeader()
	{
		Print("--- List All XOBs --------------------------------------------------------------", LogLevel.NORMAL);
	}

	//------------------------------------------------------------------------------------------------
	override void ProcessFile(ResourceName file)
	{
		if (m_bPrintFiles)
			Print(file, LogLevel.NORMAL);
	}

	//------------------------------------------------------------------------------------------------
	override void PrintResults()
	{
		Print("Total: " + m_aFiles.Count(), LogLevel.NORMAL);
	}
}

class SCR_SearchXOBLoadAllFunctor : SCR_SearchXOBFunctor
{
	//------------------------------------------------------------------------------------------------
	override void PrintHeader()
	{
		Print("--- Load All XOBs --------------------------------------------------------------", LogLevel.NORMAL);
	}

	//------------------------------------------------------------------------------------------------
	override void ProcessFile(ResourceName file)
	{
		Resource res = Resource.Load(file);
		MeshObject mesh = res.GetResource().ToMeshObject();
		if (mesh)
			m_iCounter++;
	}

	//------------------------------------------------------------------------------------------------
	override void PrintResults()
	{
		Print("Loaded: " + m_iCounter, LogLevel.NORMAL);
		Print("Total: " + m_aFiles.Count(), LogLevel.NORMAL);
	}
}

class SCR_SearchXOBInvalidMaskFunctor : SCR_SearchXOBFunctor
{
	//------------------------------------------------------------------------------------------------
	override void PrintHeader()
	{
		Print("--- Find Invalid Mask ----------------------------------------------------------", LogLevel.NORMAL);
	}

	//------------------------------------------------------------------------------------------------
	override void ProcessFile(ResourceName file)
	{
		Resource res = Resource.Load(file);
		MeshObject mesh = res.GetResource().ToMeshObject();
		if (m_bIgnoreNoColliders && mesh.GetNumGeoms() == 0)
			return;

		if (mesh.HasValidMask())
			return;

		m_iCounter++;
		if (m_bPrintFiles)
			Print(string.Format("@\"%1\"", file.GetPath()), LogLevel.NORMAL);
	}

	//------------------------------------------------------------------------------------------------
	override void PrintResults()
	{
		Print("Found: " + m_iCounter, LogLevel.NORMAL);
		Print("Total: " + m_aFiles.Count(), LogLevel.NORMAL);
	}
}

class SCR_SearchXOBLayerMaskFunctor : SCR_SearchXOBFunctor
{
	protected int m_iLayerMask;
	protected SCR_ESearchXOBPluginLayerFilter m_eLayerFilter = SCR_ESearchXOBPluginLayerFilter.ANY;

	//------------------------------------------------------------------------------------------------
	override void PrintHeader()
	{
		Print("--- Find Layer Mask ------------------------------------------------------------", LogLevel.NORMAL);
	}

	//------------------------------------------------------------------------------------------------
	override void ProcessFile(ResourceName file)
	{
		Resource res = Resource.Load(file);
		MeshObject mesh = res.GetResource().ToMeshObject();
		if (m_bIgnoreNoColliders && mesh.GetNumGeoms() == 0)
			return;

		if (!mesh.HasLayerMask(m_iLayerMask, m_eLayerFilter))
			return;

		m_iCounter++;
		if (m_bPrintFiles)
			Print(string.Format("@\"%1\"", file.GetPath()), LogLevel.NORMAL);
	}

	//------------------------------------------------------------------------------------------------
	override void PrintResults()
	{
		Print("Found: " + m_iCounter, LogLevel.NORMAL);
		Print("Total: " + m_aFiles.Count(), LogLevel.NORMAL);
	}

	//------------------------------------------------------------------------------------------------
	// constructor
	void SCR_SearchXOBLayerMaskFunctor(bool printFiles, bool ignoreNoColliders, ResourceName searchedDir, int searchedAddon, int layerMask, SCR_ESearchXOBPluginLayerFilter layerFilter)
	{
		m_iLayerMask = layerMask;
		m_eLayerFilter = layerFilter;
	}
}

[WorkbenchPluginAttribute(name: "Search Tool (XOBs)", description: "Searches XOB files", wbModules: { "ResourceManager" }, awesomeFontCode: 0xF002)]
class SearchXOBPlugin : WorkbenchPlugin // TODO: SCR_
{
	[Attribute("0", UIWidgets.ComboBox, "Type of search", "", enumType: SCR_ESearchXOBPluginSearchType)]
	protected SCR_ESearchXOBPluginSearchType m_eSearchType;

	[Attribute("1", UIWidgets.ComboBox, "In which addon should be search performed", "", SCR_ParamEnumArray.FromAddons())]
	protected int m_iAddon;

	[Attribute(defvalue: "", desc: "Folder where to perform search. If empty, search is performed everywhere", params: "unregFolders")]
	protected ResourceName m_sSearchedDirectory;

	[Attribute("0", UIWidgets.ComboBox, "LayerPreset search parameter", "", enumType: EPhysicsLayerPresets)]
	protected EPhysicsLayerPresets m_eLayerPreset;

	[Attribute("0", UIWidgets.ComboBox, "LayerMask search parameter", "", enumType: EPhysicsLayerDefs)]
	protected ref array<EPhysicsLayerDefs> m_aLayerDefinitions;

	[Attribute("0", UIWidgets.ComboBox, "Filter applied to layer mask", "", enumType: SCR_ESearchXOBPluginLayerFilter)]
	protected SCR_ESearchXOBPluginLayerFilter m_eLayerFilter;

	[Attribute(defvalue: "0", desc: "Print all found files")]
	protected bool m_bPrintFiles;

	[Attribute(defvalue: "0", desc: "Ignore files with no physics geometry")]
	protected bool m_bIgnoreNoColliders;

	//------------------------------------------------------------------------------------------------
	override void Run()
	{
		if (!Workbench.ScriptDialog("Search XOBs", "Choose search type, set search parameters ...", this))
			return;

		SCR_SearchXOBFunctor functor = CreateFunctor();
		functor.Search();
	}

	//------------------------------------------------------------------------------------------------
	override void RunCommandline()
	{
		// ResourceManager module = Workbench.GetModule(ResourceManager);
		// module = Workbench.GetModule(ResourceManager);

		Print("Command-line functionality not implemented", LogLevel.WARNING);

		//SCR_SearchXOBFunctor functor = CreateFunctor();
		//functor.Search();

		Workbench.Exit(0);
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Run")]
	protected bool OK()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Cancel")]
	protected bool Cancel()
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected int GetLayerMask()
	{
		int mask = 0;
		foreach (int layer : m_aLayerDefinitions)
		{
			mask |= layer;
		}
		return mask;
	}

	//------------------------------------------------------------------------------------------------
	protected SCR_SearchXOBFunctor CreateFunctor()
	{
		switch (m_eSearchType)
		{
			case SCR_ESearchXOBPluginSearchType.ListAllXOBs: return new SCR_SearchXOBListAllFunctor(m_bPrintFiles, m_bIgnoreNoColliders, m_sSearchedDirectory, m_iAddon);
			case SCR_ESearchXOBPluginSearchType.LoadAllXOBs: return new SCR_SearchXOBLoadAllFunctor(m_bPrintFiles, m_bIgnoreNoColliders, m_sSearchedDirectory, m_iAddon);
			case SCR_ESearchXOBPluginSearchType.InvalidMask: return new SCR_SearchXOBInvalidMaskFunctor(m_bPrintFiles, m_bIgnoreNoColliders, m_sSearchedDirectory, m_iAddon);
			case SCR_ESearchXOBPluginSearchType.LayerPreset: return new SCR_SearchXOBLayerMaskFunctor(m_bPrintFiles, m_bIgnoreNoColliders, m_sSearchedDirectory, m_iAddon, m_eLayerPreset, m_eLayerFilter);
			case SCR_ESearchXOBPluginSearchType.LayerMask: return new SCR_SearchXOBLayerMaskFunctor(m_bPrintFiles, m_bIgnoreNoColliders, m_sSearchedDirectory, m_iAddon, GetLayerMask(), m_eLayerFilter);
		}

		return new SCR_SearchXOBFunctor(m_bPrintFiles, m_bIgnoreNoColliders, m_sSearchedDirectory, m_iAddon);
	}
}
#endif // WORKBENCH
