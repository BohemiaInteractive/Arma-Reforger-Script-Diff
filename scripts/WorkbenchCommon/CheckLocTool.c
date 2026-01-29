[WorkbenchPluginAttribute(name: "Check localization in UI", description: "Find non localized text in UI", wbModules: {"LocalizationEditor"})]
class CheckLocalizationPlugin: LocalizationEditorPlugin
{
	string FindTexts(WidgetSource src)
	{
		string report;
		
		if (src)
		{
			string text;
			if (src.Get("Text", text) && text.Length() > 0 && !text.Contains("#"))
			{
				report = "Text = \"" + text + "\" in Widget: " + src.GetName() + "\n";
			}
			
			for (int e = 0, count = src.GetNumChildren(); e < count; e++)
			{
				report += FindTexts(src.GetChild(e));
			}
		}
		
		return report;
	}
	
	override void Run()
	{
		LocalizationEditor editor = Workbench.GetModule(LocalizationEditor);
		WBProgressDialog progress = new WBProgressDialog("Scanning Layouts", editor);
		array<ResourceName> resNames = {};
		
		SearchResourcesFilter filter = new SearchResourcesFilter();
		filter.fileExtensions = {"layout"};
		ResourceDatabase.SearchResources(filter, resNames.Insert);
		
		int count = resNames.Count();
		int problems;
		string report;
		string desc;
		
		for (int i = 0; i < count; i++)
		{
			progress.SetProgress(i / count);
			Resource res = BaseContainerTools.LoadContainer(resNames[i]);
	
			if (res && res.IsValid())
			{
				WidgetSource cont = res.GetResource();
				
				string layoutReport = FindTexts(cont);
				if (layoutReport.Length())
				{
					report += resNames[i].GetPath() + ":\n";
					report += layoutReport;
					report += "\n";
					problems++;
				}
			}
		}
		
		if (problems)
		{
			desc = "Found " + problems + " layouts with not localized texts!";
		}
		else
		{
			desc = "All texts in UI are localized.";
		}
		
		progress = null;
		
		Workbench.Dialog("Check localization in UI", desc, report);
	}
}
