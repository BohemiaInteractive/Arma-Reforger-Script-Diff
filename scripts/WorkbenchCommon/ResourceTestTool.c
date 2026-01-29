//----------------------------------------------
/**
 * \defgroup ResourceTest plugin
 * This plugin opens resources specified in the -resources command line param. If multiple resources are specified, they must be delimitered by a comma ','. Resources are defined as this example {7297BDBDE223627F}
 * Every resource is captured to a screenshot which will be saved into directory specified by -screenshot-output-dir command line parameter. Name of the file is created as a RESOURCE_NAME.bmp.
 * For every screenshot a metadata file is created which includes additional information about the resource. Name of the file is created as a RESOURCE_NAME.bmp.txt.
 * Plugin also creates summary.txt file in the output directory.
 */

[WorkbenchPluginAttribute("ResourceTest Plugin", "Opens resources and creates screenshots")]
class ResourceTestPlugin: WorkbenchPlugin
{

/*
	override void Run() 
	{
		string ids = "{7297BDBDE223627F},{B90A1D47FB5036FA},{7C37800102FAC817}";
		string directory = "$logs:ResourceTestTool";

		array<string> resources = new array<string>;
		ids.Split(",", resources, true);
		
		MakeDirectory(directory);	
		foreach (string res: resources)
		{
			OpenResource(res, directory, 1000);
		}
		
		string summeryFilename = string.Format("%1/%2", directory, "summary.txt");
		MakeSummeryFile(summeryFilename);
	}
*/
	
	override void RunCommandline()
	{
		ResourceManager rb = Workbench.GetModule(ResourceManager);
		
		int errorCode = 0;
		
		string ids;
		rb.GetCmdLine("-resources", ids);
		
		string directory;
		rb.GetCmdLine("-autotest-output-dir", directory);
		if (directory.Length() == 0)
		{
			directory = "$logs:ResourceTestTool";
		}
		
		array<string> resources = new array<string>;
		ids.Split(",", resources, true);
		
		FileIO.MakeDirectory(directory);
		foreach (ResourceName res: resources)
		{
			if (!OpenResource(res, directory, 1000))
			{
				errorCode = -1;
			}
		}
		
		string summeryFilename = string.Format("%1/%2", directory, "summary.txt");
		MakeSummeryFile(summeryFilename);
		
		Workbench.Exit(errorCode);
	}
	
	bool OpenResource(ResourceName resourceName, string directory, int waitTime)
	{
		bool success = false;
		Print("ResourceTestPlugin: opening resource "+ resourceName, LogLevel.VERBOSE);

		ResourceManager rb = Workbench.GetModule(ResourceManager);
		
		if (rb.SetOpenedResource(resourceName))
		{
			success = true;
			
			string resourceFileName = FilePath.StripPath(resourceName.GetPath());
			
			string screenshotFilename = string.Format("%1/%2.bmp", directory, resourceFileName);
			string metadataFilename = screenshotFilename + ".txt";
			
			if (waitTime > 0) Sleep(waitTime);
			
			MakeScreenshotMetafile(metadataFilename, resourceName);
			System.MakeScreenshot(screenshotFilename);
			
			if (waitTime > 0) Sleep(waitTime);
		}
		
		if (success)
		{
			Print("ResourceTestPlugin: opened successfully! ", LogLevel.VERBOSE);
		}
		else
		{
			Print("ResourceTestPlugin: opening failed!", LogLevel.ERROR);
		}
		
		return success;
	}
	
	private void MakeSummeryFile(string filename)
	{
		FileHandle descrFile = FileIO.OpenFile(filename, FileMode.WRITE);
		
		if(descrFile)
		{
			descrFile.WriteLine("Autotest: Resource opening in Workbench");
			// TODO: would be nice to have a resolution here, but widget workspace needs game
			descrFile.Close();
			Print("Summary file successfully saved into " + filename);
		}
	}
	
	private void MakeScreenshotMetafile(string filename, ResourceName resourceName)
	{
		FileHandle descrFile = FileIO.OpenFile(filename, FileMode.WRITE);
		
		if(descrFile)
		{
			descrFile.WriteLine(string.Format("<p>FPS: %1</p>", System.GetFPS()));
			descrFile.WriteLine(string.Format("<p>ResourceID: %1%2</p>" ,resourceName, resourceName.GetPath()));

			string link = string.Format("enfusion://ResourceManager/%1", resourceName.GetPath());
			descrFile.WriteLine(string.Format("<a href=\"%1\">Link to Resource Manager</a>", link));
			
			descrFile.Close();
			Print("Screenshot metafile successfully saved into " + filename);
		}
	}
	
}

