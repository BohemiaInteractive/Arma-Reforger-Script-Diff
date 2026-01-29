[WorkbenchPluginAttribute(name: "Image Set Generator", wbModules: { "ResourceManager" }, awesomeFontCode: 0xF302)]
class SCR_ImageSetGenerator : WorkbenchPlugin
{
	const string TAB = " ";

	//[Attribute("{717ED53394FC3C27}UI/Textures/Editor/Concept/editor_mockup_sheet.edds", UIWidgets.ResourceNamePicker, "Path to the source image", "edds")]
	//protected ResourceName m_sImage;

	[Attribute("generatedImageSet", desc: "File name of the image set")]
	protected string m_sFileName;

	[Attribute("$ArmaReforger:UI/imagesets/", desc: "Directory at which the image set will be saved")]
	protected string m_sDirectory;

	[Attribute("512", desc: "Width of the image set")]
	protected int m_iImageWidth;

	[Attribute("512", desc: "Height of the image set")]
	protected int m_iImageHeight;

	[Attribute("", UIWidgets.Auto, "Independent tile areas. There can be multiple within one image set, e.g., left half containing 32x32 tiles and the right one 64x64 tiles")]
	protected ref array<ref ImageSetGeneratorArea> m_aAreas;

	protected string m_sTabs;
	protected int m_iTabsCount;
	protected ref FileHandle m_File;

	//------------------------------------------------------------------------------------------------
	protected void AddTabs(int tabsCountDelta)
	{
		m_iTabsCount += tabsCountDelta;
		m_sTabs = "";
		for (int i = 0; i < m_iTabsCount; i++)
		{
			m_sTabs += TAB;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void AddLine(string text)
	{
		m_File.WriteLine(m_sTabs + text);
	}

	//------------------------------------------------------------------------------------------------
	protected void Generate()
	{

		if (!m_sDirectory.EndsWith("/"))
			m_sDirectory += "/";

		string fullPath = m_sDirectory + m_sFileName + ".imageset";
		m_File = FileIO.OpenFile(fullPath, FileMode.WRITE);
		Print(m_File, LogLevel.NORMAL);
		if (!m_File)
			return;

		//--- Header
		AddLine("ImageSetClass {");
		AddTabs(1);

		AddLine(string.Format("RefSize %1 %2", m_iImageWidth, m_iImageHeight));

		/*
		AddLine("Textures {");
		AddTabs(1);

		AddLine("ImageSetTextureClass {");
		AddTabs(1);

		AddLine("mpix 1");
		AddLine(string.Format("directory \"%1\"", m_sImage.GetPath()));

		AddTabs(-1);
		AddLine("}");

		AddTabs(-1);
		AddLine("}");
		*/

		//--- Groups
		AddLine("Groups {");
		AddTabs(1);

		foreach (ImageSetGeneratorArea area : m_aAreas)
		{
			string groupName = string.Format("%1x%2", area.tileWidth, area.tileHeight);

			AddLine(string.Format("ImageSetGroupClass \"%1\" {", groupName));
			AddTabs(1);
			AddLine(string.Format("Name \"%1\"", groupName));
			AddLine("Images {");
			AddTabs(1);

			//--- Images
			int widthFull = area.tileWidth + area.marginWidth;
			int heightFull = area.tileHeight + area.marginHeight;
			int columns = area.areaWidth / widthFull;
			int rows = area.areaHeight / heightFull;
			for (int r = 0; r < rows; r++)
			{
				for (int c = 0; c < columns; c++)
				{
					string tileName = string.Format("%1-%2", c, r);
					AddLine(string.Format("ImageSetDefClass %1 {", groupName));
					AddTabs(1);

					//--- Image
					AddLine(string.Format("Name \"%1\"", tileName));
					AddLine(string.Format("Pos %1 %2", area.areaX + c * widthFull, area.areaY + r * heightFull));
					AddLine(string.Format("Size %1 %2", area.tileWidth, area.tileHeight));
					AddLine("Flags 0");

					AddTabs(-1);
					AddLine("}");
				}
			}

			AddTabs(-1);
			AddLine("}");

			AddTabs(-1);
			AddLine("}");
		}

		AddTabs(-1);
		AddLine("}");

		AddTabs(-1);
		AddLine("}");

		m_File.Close();

		Print(string.Format("Image set generated at %1", fullPath), LogLevel.DEBUG);
	}

	//------------------------------------------------------------------------------------------------
	override void Run()
	{
		if (!Workbench.ScriptDialog("Image Set Generator", "Generate an image set template with pre-define grids.\nOnce generated, you can simply map the texture and rename existing tiles,\ninstead of having to configure them manually one by one.", this))
			return;

		Generate();
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("OK")]
	protected bool ButtonOK()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Cancel")]
	protected bool ButtonCancel()
	{
		return false;
	}
}

// TODO: SCR_, m_
[BaseContainerProps()]
class ImageSetGeneratorArea
{
	[Attribute("0", UIWidgets.Auto, "Position of left edge")]
	int areaX;

	[Attribute("0", UIWidgets.Auto, "Position of top edge")]
	int areaY;

	[Attribute("512", UIWidgets.Auto, "Area width")]
	int areaWidth;

	[Attribute("512", UIWidgets.Auto, "Area height")]
	int areaHeight;

	[Attribute("32", UIWidgets.Auto, "Width of individual tile")]
	int tileWidth;

	[Attribute("32", UIWidgets.Auto, "Height of individual tile")]
	int tileHeight;

	[Attribute("0", UIWidgets.Auto, "Width of a margin between tiles")]
	int marginWidth;

	[Attribute("0", UIWidgets.Auto, "Height of a margin between tiles")]
	int marginHeight;
}
