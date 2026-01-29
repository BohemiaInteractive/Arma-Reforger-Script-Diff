#ifdef WORKBENCH

[WorkbenchPluginAttribute(name: "Data To Spreadsheet plugin", category: "", shortcut: "Alt+Shift+E", wbModules: { "ResourceManager" }, awesomeFontCode: 0xF6DD)]
class SCR_DataToSpreadsheetPlugin : ResourceManagerPlugin
{
	[Attribute("", UIWidgets.ResourceNamePicker, "Pick a config with Spreadsheet parser configuration.", "conf", null, "Settings")]
	protected ResourceName m_sPrefabConfig;

	[Attribute("1", desc: "Use comma as delimiter, otherwise use dot.", category: "Settings")]
	protected bool m_bCommaAsDelimiter;

	[Attribute("1", desc: "Use Prefab Array from config instead of the current Resource Browser selection.", category: "Settings")]
	protected bool m_bUseSelectionFromConfig;

	[Attribute("0", desc: "Check this option to enable additional debug.", category: "Debug")]
	bool m_bPrintToConsole;

	protected string m_sExportString;
	protected string m_sHeaderString;
	protected bool m_bGenerateHeader;
	protected bool m_bRunExport;
	protected bool m_bReadData;
	protected bool m_bSaveRequired;

	protected ref map<string, ref SCR_DataToSpreadsheetElement> m_mImportData = new map<string, ref SCR_DataToSpreadsheetElement>();

	protected static const string DESCRIPTION = "This plugin allows to transfer data between Workbench and spreadsheets\nExport button grabs data from Workbench to clipboard\nImport button grabs data from clipboard to Workbench";

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Import")]
	protected void ImportButton()
	{
		//m_bRunExport = false;
		m_bReadData = false;
		m_mImportData.Clear();

		// Get content of user clipboard
		string input = System.ImportFromClipboard();

		// Verify input
		if (input.IsEmpty())
			return;

		// Parse input
		array<string> parsedText = {};
		input.Split("\n", parsedText, false);

		// Parse header
		array<string> parsedHeader = {};
		parsedText[0].Split("\t", parsedHeader, false);
		if (m_bPrintToConsole)
			PrintFormat("Header: %1", parsedHeader, LogLevel.NORMAL);

		// Parse lines
		for (int i = 1, count = parsedText.Count(); i < count; ++i)
		{
			array<string> parsedLine = {};
			map<string, string> data = new map<string, string>();
			parsedText[i].Split("\t", parsedLine, false);

			// Empty new line for instance
			if (parsedLine.Count() == 1)
				continue;

			if (parsedHeader.Count() != parsedLine.Count())
			{
				Print("Invalid data - number of columns in one of the line doesn't match the amount of columns in header", LogLevel.ERROR);
				m_bRunExport = false;
				return;
			}

			for (int i2 = 1, count2 = parsedLine.Count(); i2 < count2; ++i2)
			{
				data.Insert(parsedHeader[i2].Trim(), parsedLine[i2].Trim());
				if (m_bPrintToConsole)
					PrintFormat("'%1': '%2'", parsedHeader[i2], parsedLine[i2], level: LogLevel.NORMAL);
			}

			m_mImportData.Insert(parsedLine[0], new SCR_DataToSpreadsheetElement(data));
		}
		// Verify parse input
		if (parsedText.Count() == 1)
		{
			Print("No data to import was detected - check content of clipboard", LogLevel.WARNING);
			m_bRunExport = false;
		}
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Export")]
	protected void ExportButton()
	{
		ExportToExcel(true);
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Cancel")]
	protected bool CancelButton()
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected override void Configure()
	{
		Workbench.ScriptDialog("Data To Spreadsheets", DESCRIPTION, this);
	}

	//------------------------------------------------------------------------------------------------
	//! Initial processing of data provided by config
	//! All of the top attributes are then processed - including their children elements
	protected void ProcessData(IEntitySource entSrc, ResourceName prefabName, SCR_DataToSpreadsheetTemplates prefabConfig)
	{
		array<ref ContainerIdPathEntry> containerPath = {};
		ProcessAttributes(entSrc, containerPath, prefabName, prefabConfig.m_aAttributesArray);
	}

	//------------------------------------------------------------------------------------------------
	//! Handle all attributes listed in config and send them for further processing
	protected void ProcessAttributes(IEntitySource entSrc, array<ref ContainerIdPathEntry> containerPath, ResourceName prefabName, array<ref SCR_DataToSpreadsheetTemplatesBase> TemplateData)
	{
		IEntityComponentSource component;
		foreach (SCR_DataToSpreadsheetTemplatesBase currentEntry : TemplateData)
		{
			switch (currentEntry.ReturnType())
			{
				case "Component" :
				{
					SCR_DataToSpreadsheetTemplatesComponent entry = SCR_DataToSpreadsheetTemplatesComponent.Cast(currentEntry);
					component = SCR_BaseContainerTools.FindComponentSource(entSrc, entry.m_sComponentName);
					if (!component)
						continue;

					array<ref ContainerIdPathEntry> containerPathSub = {};
					foreach (ContainerIdPathEntry obj : containerPath)
					{
						containerPathSub.Insert(obj);
					}

					containerPathSub.Insert(new ContainerIdPathEntry(entry.m_sComponentName));
					ProcessAttributes(component, containerPathSub, prefabName, entry.GetAttributes());
				}
				break;

				case "Attribute" :
				{
					SCR_DataToSpreadsheetTemplatesAttribute entry = SCR_DataToSpreadsheetTemplatesAttribute.Cast(currentEntry);
					ProcessAttribute(entSrc, containerPath, prefabName, entry);
				}
				break;

				case "Object" :
				{
					SCR_DataToSpreadsheetTemplatesObject entry = SCR_DataToSpreadsheetTemplatesObject.Cast(currentEntry);
					BaseContainer container = entSrc.GetObject(entry.m_sObjectName);
					if (!container)
						continue;

					array<ref ContainerIdPathEntry> containerPathSub = {};
					foreach (ContainerIdPathEntry obj : containerPath)
					{
						containerPathSub.Insert(obj);
					}

					containerPathSub.Insert(ContainerIdPathEntry(entry.m_sObjectName));
					ProcessAttributes(container, containerPathSub, prefabName, entry.GetAttributes());
				}
				break;

				case "ObjectArray" :
				{
					SCR_DataToSpreadsheetTemplatesObjectArray entry = SCR_DataToSpreadsheetTemplatesObjectArray.Cast(currentEntry);
					BaseContainerList containerList = entSrc.GetObjectArray(entry.m_ObjectName);
					array<ref ContainerIdPathEntry> containerPathSub = new array<ref ContainerIdPathEntry>;
					foreach (ContainerIdPathEntry obj : containerPath)
					{
						containerPathSub.Insert(obj);
					}
					containerPathSub.Insert(ContainerIdPathEntry(entry.m_ObjectName));
					ProcessAttributesList(containerList, containerPathSub, prefabName, entry.GetAttributes());
				}
				break;
			}
		}
	}

	protected static BaseContainer GetChildFromList(notnull BaseContainerList containerList, string childName)
	{
		if (childName.IsEmpty())
			return null;

		string arrayItemName;
		BaseContainer containerListElement;
		for (int i, cnt = containerList.Count(); i < cnt; i++)
		{
			containerListElement = containerList[i];
			arrayItemName = containerListElement.GetName();

			if (arrayItemName.IsEmpty())
				arrayItemName = containerListElement.GetClassName(); // if .et

			if (arrayItemName.IsEmpty())
				continue;

			arrayItemName.ToLower();
			childName.ToLower();
			if (arrayItemName == childName)
				return containerListElement;
		}

		return null;
	}

	void ProcessAttributesList(BaseContainerList containerList, array<ref ContainerIdPathEntry> containerPath, ResourceName prefabName, array<ref SCR_DataToSpreadsheetTemplatesBase> TemplateData)
	{

		foreach (SCR_DataToSpreadsheetTemplatesBase currentEntry : TemplateData)
		{
			switch (currentEntry.ReturnType())
			{
				case "Object" :
				{
					SCR_DataToSpreadsheetTemplatesObject entry = SCR_DataToSpreadsheetTemplatesObject.Cast(currentEntry);
					BaseContainer container = GetChildFromList(containerList, entry.m_sObjectName);
					array<ref ContainerIdPathEntry> containerPathSub = new array<ref ContainerIdPathEntry>;
					foreach (ContainerIdPathEntry obj : containerPath)
					{
						containerPathSub.Insert(obj);
					}
					containerPathSub.Insert(ContainerIdPathEntry(entry.m_sObjectName));
					ProcessAttributes(container, containerPathSub, prefabName, entry.GetAttributes());
				}
				break;

				case "ObjectIndex" :
				{
					SCR_DataToSpreadsheetTemplatesObjectIndex entry = SCR_DataToSpreadsheetTemplatesObjectIndex.Cast(currentEntry);
					BaseContainer container = containerList.Get(entry.m_iObjectIndex);
					array<ref ContainerIdPathEntry> containerPathSub = new array<ref ContainerIdPathEntry>;
					foreach (ContainerIdPathEntry obj : containerPath)
					{
						containerPathSub.Insert(obj);
					}
					containerPathSub.Insert(ContainerIdPathEntry(entry.m_iObjectIndex.ToString()));
					ProcessAttributes(container, containerPathSub, prefabName, entry.GetAttributes());
				}
				break;

				default:
				{
					Print("Incorrect element");
				}
			}
		}
	}


	//------------------------------------------------------------------------------------------------
	//! Process single, selected attribute
	//! Depending on settings, data from selected attribute can be either exported or modified
	void ProcessAttribute(IEntitySource entSrcAttribute, array<ref ContainerIdPathEntry> containerPath, ResourceName prefabName, SCR_DataToSpreadsheetTemplatesAttribute currentAttribute)
	{
		// Read or write
		string attributeName = currentAttribute.m_sAttributeName;
		int attributeType = currentAttribute.m_eAttributeType;
		string value;
		if (attributeType == SCR_EDataToSpreadsheetDataType.INT)
		{
			string currentValue;
			entSrcAttribute.Get(attributeName, currentValue);
			if (currentValue.IsEmpty())
				entSrcAttribute.GetDefaultAsString(attributeName, currentValue);

			value = currentValue;
		}
		else
		if (attributeType == SCR_EDataToSpreadsheetDataType.FLOAT)
		{
			string currentValue;
			entSrcAttribute.Get(attributeName, currentValue);
			if (currentValue.IsEmpty())
				entSrcAttribute.GetDefaultAsString(attributeName, currentValue);

			if (m_bCommaAsDelimiter)
				currentValue.Replace(".", ",");

			value = currentValue;
		}
		else
		if (attributeType == SCR_EDataToSpreadsheetDataType.STRING)
		{
			string currentValue;
			entSrcAttribute.Get(attributeName, currentValue);
			if (currentValue.IsEmpty())
				entSrcAttribute.GetDefaultAsString(attributeName, currentValue);

			value = currentValue;
		}
		else
		if (attributeType == SCR_EDataToSpreadsheetDataType.RESOURCE)
		{
			ResourceName currentValue;
			entSrcAttribute.Get(attributeName, currentValue);
			if (currentValue.IsEmpty())
				entSrcAttribute.GetDefaultAsString(attributeName, currentValue);

			value = currentValue;
		}

		if (m_bPrintToConsole)
			PrintFormat("%1: %2", attributeName, value, level: LogLevel.NORMAL);

		if (m_bReadData)
		{
			m_sExportString += value + "\t";
			if (m_bGenerateHeader)
				m_sHeaderString += attributeName + "\t";
		}
		else
		{
			SCR_DataToSpreadsheetElement data;
			if (!m_mImportData.Find(prefabName, data))
				return;

			string importedData = data.GetData(attributeName);
			if (importedData.IsEmpty())
				return;

			// Check if there is some change between imported data and what is in the prefab/config
			// If there is a change detected, we are marking that this object need to be resaved by changing m_bSaveRequired flag
			if (!importedData.Compare(value))
				return;

			if (m_bCommaAsDelimiter && (attributeType == SCR_EDataToSpreadsheetDataType.INT || attributeType == SCR_EDataToSpreadsheetDataType.FLOAT))
			{
				importedData.Replace(",", ".");
				value.Replace(",", ".");
			}
			if(attributeType == SCR_EDataToSpreadsheetDataType.FLOAT)
			{
				if(importedData.ToFloat() == value.ToFloat())
					return;
			}

			entSrcAttribute.Set(attributeName, importedData);
			m_bSaveRequired = true;
			PrintFormat("Detected difference in data '%1' '%2' '%3'", importedData, value, attributeName, level: LogLevel.NORMAL);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void ExportToExcel(bool useListFromConfig)
	{
		if (!m_sPrefabConfig)
		{
			Print("Missing Prefab Config file", LogLevel.ERROR);
			return;
		}

		Resource holder = BaseContainerTools.LoadContainer(m_sPrefabConfig);
		if (!holder)
		{
			Print("Invalid Prefab Config file - either its not correct type or its missing", LogLevel.ERROR);
			return;
		}

		SCR_DataToSpreadsheetTemplates prefabConfig = SCR_DataToSpreadsheetTemplates.Cast(BaseContainerTools.CreateInstanceFromContainer(holder.GetResource().ToBaseContainer()));
		array<ref ResourceName> selection = {};
		if (useListFromConfig)
		{
			selection = prefabConfig.m_aPrefabsArray;
		}
		else
		{
			// Grab reference to ResourceManager
			ResourceManager resourceManager = Workbench.GetModule(ResourceManager);
			if (!resourceManager)
				return;

			// Get list of currently selected resources
			resourceManager.GetResourceBrowserSelection(selection.Insert, true);
		}

		// Verify if something is selected - if no, exit method & print error message
		if (selection.IsEmpty())
		{
			Print("No elements are selected in Resource Browser", LogLevel.WARNING);
			return;
		}

		if (m_bPrintToConsole)
		{
			// Print ResourceManager selection directly to the console
			Print(selection, LogLevel.NORMAL);
		}

		m_sExportString = "";
		m_sHeaderString = "Name\t";
		m_bGenerateHeader = true;

		// Copy file name to clipboard - each element will be written on new line
		BaseContainerList baseContainerList;
		foreach (ResourceName prefabName : selection)
		{
			if (m_bPrintToConsole)
				Print(prefabName, LogLevel.NORMAL);

			m_sExportString += prefabName + "\t";
			Resource prefab = Resource.Load(prefabName);
			BaseContainer prefabSource;
			MetaFile meta;
			if (prefabName.EndsWith(".et"))
			{
				prefabSource = SCR_BaseContainerTools.FindEntitySource(prefab);
			}
			else if (prefabName.EndsWith(".xob") || prefabName.EndsWith(".edds"))
			{
				// For .xob, .emat, and .edds files, use their meta file and convert to base container
				ResourceManager resourceManager = Workbench.GetModule(ResourceManager);
				meta = resourceManager.GetMetaFile(prefabName.GetPath());
				if (meta)
				{
					prefabSource = meta.ToBaseContainer();
				}
				else
				{
					Print(string.Format("Failed to get meta file for: %1", prefabName), LogLevel.ERROR);
					continue;
				}
			}
			else
			{
				prefabSource = prefab.GetResource().ToBaseContainer();
			}

			ProcessData(prefabSource, prefabName, prefabConfig);

			m_sExportString += "\n";
			m_bGenerateHeader = false;
			if (m_bSaveRequired)
			{
				if (meta)
					meta.Save();
				BaseContainerTools.SaveContainer(prefabSource, prefabName);
				m_bSaveRequired = false;
			}
		}

		if (m_bReadData)
		{
			m_sHeaderString += "\n";
			m_sExportString.Replace("\t\n","\n");

			System.ExportToClipboard(m_sHeaderString + m_sExportString);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected override void Run()
	{
		m_bRunExport = true;
		m_bReadData = true;

		if (Workbench.ScriptDialog("Data To Spreadsheets", DESCRIPTION, this) == 0)
			return;

		if (m_bRunExport)
			ExportToExcel(m_bUseSelectionFromConfig);
	}
}

//! Unique flags for each data type
enum SCR_EDataToSpreadsheetDataType
{
	INT			= 1 << 0,
	FLOAT		= 1 << 1,
	STRING		= 4 << 2,
	RESOURCE	= 8 << 3,
}

class SCR_DataToSpreadsheetElement
{
	protected ref map<string, string> m_mElements;

	//------------------------------------------------------------------------------------------------
	//! \param[in] attribute
	//! \return
	string GetData(string attribute)
	{
		return m_mElements.Get(attribute);
	}

	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] data
	void SCR_DataToSpreadsheetElement(notnull map<string, string> data)
	{
		m_mElements = data;
	}
}

[BaseContainerProps(configRoot: true)]
class SCR_DataToSpreadsheetTemplates
{
	[Attribute(desc: "Array of prefabs which will be processed")]
	ref array<ref ResourceName> m_aPrefabsArray;

	[Attribute(desc: "Array of attributes which will be processed")]
	ref array<ref SCR_DataToSpreadsheetTemplatesBase> m_aAttributesArray;
}

[BaseContainerProps(configRoot: false), SCR_BatchPrefabTitle("Template Base - Has no function")]
class SCR_DataToSpreadsheetTemplatesBase
{
	//------------------------------------------------------------------------------------------------
	//!
	//! \return
	string ReturnType()
	{
		return "Base";
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	array<ref SCR_DataToSpreadsheetTemplatesBase> GetAttributes();
}

[BaseContainerProps(configRoot: false), SCR_BatchPrefabTitle("Component: ", "m_sComponentName")]
class SCR_DataToSpreadsheetTemplatesComponent : SCR_DataToSpreadsheetTemplatesBase
{
	[Attribute(desc: "Name of component to modify or add")]
	string m_sComponentName;

	[Attribute(desc: "Sub array of attributes")]
	ref array<ref SCR_DataToSpreadsheetTemplatesBase> m_aAttributesArray;

	//------------------------------------------------------------------------------------------------
	override string ReturnType()
	{
		return "Component";
	}

	//------------------------------------------------------------------------------------------------
	override ref array<ref SCR_DataToSpreadsheetTemplatesBase> GetAttributes()
	{
		return m_aAttributesArray;
	}
}

[BaseContainerProps(configRoot: false), SCR_BatchPrefabTitle("m_sAttributeName")]
class SCR_DataToSpreadsheetTemplatesAttribute : SCR_DataToSpreadsheetTemplatesBase
{
	[Attribute(desc: "Name of attribute to modify")]
	string m_sAttributeName;

	[Attribute("2", UIWidgets.ComboBox, desc: "Specifies type of parameter", enums: ParamEnumArray.FromEnum(SCR_EDataToSpreadsheetDataType))]
	SCR_EDataToSpreadsheetDataType m_eAttributeType;

	//------------------------------------------------------------------------------------------------
	override string ReturnType()
	{
		return "Attribute";
	}
}

[BaseContainerProps(configRoot: false), SCR_BatchPrefabTitle("Object: ", "m_sObjectName")]
class SCR_DataToSpreadsheetTemplatesObject : SCR_DataToSpreadsheetTemplatesBase
{
	[Attribute(desc: "Name of container to modify or add")]
	string m_sObjectName;

	[Attribute(desc: "Array of attributes which will be exported or modified inside container belonging to prefab")]
	ref array<ref SCR_DataToSpreadsheetTemplatesBase> m_aAttributesArray;

	//------------------------------------------------------------------------------------------------
	override string ReturnType()
	{
		return "Object";
	}

	//------------------------------------------------------------------------------------------------
	override array<ref SCR_DataToSpreadsheetTemplatesBase> GetAttributes()
	{
		return m_aAttributesArray;
	}
}

[BaseContainerProps(configRoot: false), SCR_BatchPrefabTitle("Object Array: ", "m_ObjectName")]
class SCR_DataToSpreadsheetTemplatesObjectArray: SCR_DataToSpreadsheetTemplatesBase
{
	[Attribute(desc: "Name of component to modify or add")]
	string m_ObjectName;

	[Attribute(desc: "Array of attributes which will be modified inside component belonging to prefab. If component is not present, new one will be created")]
	ref array<ref SCR_DataToSpreadsheetTemplatesBase> m_AttributesArray;

	override string ReturnType()
	{
		return "ObjectArray";
	}
	override ref array<ref SCR_DataToSpreadsheetTemplatesBase> GetAttributes()
	{
		return m_AttributesArray;
	}
}

[BaseContainerProps(configRoot: false), SCR_BatchPrefabTitle("Object Index: ", "m_iObjectIndex")]
class SCR_DataToSpreadsheetTemplatesObjectIndex : SCR_DataToSpreadsheetTemplatesBase
{
	[Attribute(desc: "Name of container to modify or add")]
	int m_iObjectIndex;

	[Attribute(desc: "Array of attributes which will be exported or modified inside container belonging to prefab")]
	ref array<ref SCR_DataToSpreadsheetTemplatesBase> m_aAttributesArray;

	//------------------------------------------------------------------------------------------------
	override string ReturnType()
	{
		return "ObjectIndex";
	}

	//------------------------------------------------------------------------------------------------
	override array<ref SCR_DataToSpreadsheetTemplatesBase> GetAttributes()
	{
		return m_aAttributesArray;
	}
}

#endif // WORKBENCH