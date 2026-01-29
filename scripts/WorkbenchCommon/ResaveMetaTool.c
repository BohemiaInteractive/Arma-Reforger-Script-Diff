[WorkbenchPluginAttribute("Re-Save Meta Tool", "Saves all meta files with given extension", "", "", {"ResourceManager"},"",0xf0c7)]
class ResaveMetaPlugin: WorkbenchPlugin
{
	[Attribute(uiwidget: UIWidgets.FileNamePicker, desc: "Root directory to search", params:"unregFolders" )]
	string RootPath;

	[Attribute(uiwidget: UIWidgets.EditBox, desc: "File extensions (without .meta)" )]
	ref array<string> Extensions;

	ResourceManager m_module;
	ref array<string> m_files = {};

	void Find(ResourceName resName, string file)
	{
		m_files.Insert(file);
	}
	
	void Resave()
	{
		WBProgressDialog progress = new WBProgressDialog("Resaving...", m_module);
		int cnt = m_files.Count();						
		foreach (int i, string file: m_files)
		{
			MetaFile mf = m_module.GetMetaFile(file);
			if (mf)
			{
				mf.Save();
				mf.Release();
			}

			progress.SetProgress(i / cnt);
		}
		m_files.Clear();
	}
	
	override void Run()
	{
		if (Workbench.ScriptDialog("Resave", "Which files metafiles you want to resave?", this))
		{
			m_module = Workbench.GetModule(ResourceManager);
			
			SearchResourcesFilter filter = new SearchResourcesFilter();
			filter.rootPath = RootPath;
			filter.fileExtensions = Extensions;
			ResourceDatabase.SearchResources(filter, Find);
			
			Resave();			
		}
	}
	
	override void RunCommandline() 
	{
		m_module = Workbench.GetModule(ResourceManager);
		SearchResourcesFilter filter = new SearchResourcesFilter();

		string addonPathCli;
		if (m_module.GetCmdLine("-addonPath", addonPathCli))
		{
			if (!FilePath.IsAbsolutePath(addonPathCli))
			{
				PrintFormat("Argument 'addonPath' must be an absolute path, but its value is '%1'", addonPathCli, level: LogLevel.ERROR);
				Workbench.Exit(1);
				return;
			}

			string addonGuid, addonId, addonExactRoot;
			bool found = AddonBuildInfoTool.FindAddonByAbsolutePath(
				addonPathCli, addonGuid, addonId, addonExactRoot
			);
			if (found)
				filter.rootPath = addonExactRoot;
		}

		string ext;
		if (m_module.GetCmdLine("-extension", ext))
		{
			ext.Replace("\"", "");
			ext.Replace(".", "");
			Extensions = {ext};
		}

		filter.fileExtensions = Extensions;
		ResourceDatabase.SearchResources(filter, Find);
		
		Resave();
		Workbench.Exit(0);
	}
	
	[ButtonAttribute("Re-Save")]
	bool OK()
	{
		return true;
	}
	
	[ButtonAttribute("Cancel")]
	bool Cancel()
	{
		return false;
	}
}