#ifdef WORKBENCH
/** @defgroup BatchPrefabsOperationsTool Batch Prefabs Operations Tool
Workbench tool for batch operation on multiple objects. It
*/

/** @ingroup BatchPrefabsOperationsTool
*/

/*!
Workbench tool for generation of windows & glass prefabs.

With this tool you can either create dozens of prefabs with selected configuration from XOB models
by pressing "Create" button or modify configuration of existing prefabs with "Modify" button.
*/
[WorkbenchToolAttribute("Batch prefabs operations tool", "### Creating new prefabs ###\nTo create new prefabs you need to fill 'Prefab Config', 'Path To Save' and\n'Models To Process'\n\nIf Generate Names is checked, there is no need to fill in New Names array\nUse Prefab Config to store additional settings in new prefab.\n\nOnce everything is set, click on 'Create' button to create new prefabs\n\n### Modifying  existing prefabs ###\nTo modify prefabs you need to have at least one prefab in\n'Prefabs To Process' array selected & valid config selected in\n'Prefabs Config'\n\nWith those 2 parameters set, click on 'Modify' button to modify selected prefabs", "2", awesomeFontCode: 0xF085)]
class SCR_BatchPrefabsOperationsTool : WorldEditorTool
{
	[Attribute("1", UIWidgets.ComboBox, "File system where new prefabs will be created", "", SCR_ParamEnumArray.FromAddons(), "General")]
	protected int m_iAddonToUse;

	[Attribute("", UIWidgets.ResourceNamePicker, "Choose a base class of your prefabs.", "et", null, "Create")]
	protected ResourceName m_sBaseClass;

	[Attribute("", UIWidgets.ResourceNamePicker, "Choose a path to save your prefabs.", "unregFolders", null, "Create")]
	protected ResourceName m_sPathToSave;

	[Attribute("", UIWidgets.ResourceAssignArray, "Choose XOB files to process.", "xob", null, "Create")]
	protected ref array<ResourceName> m_aModelsToProcess;

	[Attribute("", UIWidgets.EditBox, "Choose new names of your prefabs.", "et", null, "Create")]
	protected ref array<string> m_aNewNames;

	[Attribute("", UIWidgets.CheckBox, "Generate names from source files.","", null, "Create")]
	protected bool m_bGenerateNames;

	[Attribute("", UIWidgets.ResourceAssignArray, "Choose prefabs to process.", "et", null, "Modify")]
	protected ref array<ResourceName> m_aPrefabsToProcess;

	[Attribute(defvalue: "", desc: "All prefabs in this directory will be processed. Replaces Prefabs To Process array", params: "unregFolders", category: "Modify")]
	protected ResourceName m_sDirectoryToProcess;

	[Attribute("", UIWidgets.ResourceNamePicker, "Pick a config with template configuration.", "conf", null, "Modify")]
	protected ResourceName m_sPrefabConfig;

	//------------------------------------------------------------------------------------------------
	//! Process all components listed in config and try to apply all changes defined in
	//! m_ComponentAttributesArray by calling ProcessAttributes function.
	//! If component is not found, new one is created.
	//! \param[in] entSrc Entity Source from which components should be read
	//! \param[in] m_PrefabGeneratorConfig Handle to config file containing instructions for the tool
	protected void ProcessComponents(IEntitySource entSrc, SCR_BatchPrefabTemplates prefabGeneratorConfig)
	{
		IEntityComponentSource component;
		array<ref ContainerIdPathEntry> containerPath = {};
		foreach (SCR_BatchPrefabTemplatesData currentComp : prefabGeneratorConfig.m_ComponentsArray)
		{
			int action = currentComp.m_ComponentAction;

			// Check if component already exists
			component = SCR_BaseContainerTools.FindComponentSource(entSrc, currentComp.m_ComponentName);
			if (!component)
			{
				// Return if modify only flag is used
				if (action == 2)
					return;

				// Create new component
				component = m_API.CreateComponent(entSrc, currentComp.m_ComponentName);
			}

			// Delete component
			if (action == 4)
			{
				bool success = m_API.DeleteComponent(entSrc, component);
				if (!success)
					Print("Unable to delete " + currentComp.m_ComponentName + " from " + entSrc + ". Component is probably inherited", LogLevel.WARNING);

				return;
			}

			containerPath.Clear();
			containerPath.Insert(new ContainerIdPathEntry(currentComp.m_ComponentName));
			ProcessAttributes(entSrc, component, containerPath, currentComp.m_ComponentAttributesArray);
			//ProcessAttributes(entSrc,containerPath,currentComp.m_SubAttributesArray);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Process all attributes listed in config and apply change to selected entity source.
	//! Function supports basic math (multiplying, dividing & adding) &
	//! bitwise operations.
	//! \param[in] entSrc Entity Source where new values should be stored
	//! \param[in] entSrcAttribute Entity Source from which script should read values
	//! \param[in] containerPath Path to container where values will be stored
	//! \param[in] TemplateData Array of values to be processed
	protected void ProcessAttributes(IEntitySource entSrc, IEntitySource entSrcAttribute, array<ref ContainerIdPathEntry> containerPath, array<ref SCR_BatchPrefabTemplatesAttributes> TemplateData)
	{
		// Add atributes to currently processed component
		foreach (int currentAtributeIndex, SCR_BatchPrefabTemplatesAttributes currentAttribute : TemplateData)
		{
			string attributeName = currentAttribute.m_AttributeName;
			string attributeValue = currentAttribute.m_AttributeValue;
			int attributeOperation = currentAttribute.m_AttributeOperation;
			if (attributeOperation == SCR_EBatchPrefabOperationFlag.MATH)
			{
				float m_fCurrentValue;
				entSrcAttribute.Get(attributeName, m_fCurrentValue);

				// Remove * & / signs from string since ToFloat cannot cope with it
				string attributeValueShort = attributeValue;
				attributeValueShort.Replace("*","");
				attributeValueShort.Replace("/","");
				float operand = attributeValueShort.ToFloat();
				if (attributeValue.StartsWith("+") || attributeValue.StartsWith("-"))
					m_fCurrentValue = m_fCurrentValue + operand;

				if (attributeValue.StartsWith("/"))
					m_fCurrentValue = m_fCurrentValue / operand; // TODO: Catch division by zero

				if (attributeValue.StartsWith("*"))
					m_fCurrentValue = m_fCurrentValue * operand;

				attributeValue = m_fCurrentValue.ToString();
			}

			if (attributeOperation == SCR_EBatchPrefabOperationFlag.ENUM)
			{
				int m_iCurrentValue;
				entSrcAttribute.Get(attributeName, m_iCurrentValue);
				if (attributeValue.StartsWith("+"))
					m_iCurrentValue = m_iCurrentValue | attributeValue.ToInt();

				if (attributeValue.StartsWith("-"))
					m_iCurrentValue = m_iCurrentValue &~ attributeValue.ToInt();

				attributeValue = m_iCurrentValue.ToString();
			}

			m_API.SetVariableValue(entSrc, containerPath, attributeName, attributeValue);
			//ProcessAttributes(entSrc,containerPath,currentAttribute.m_SubAttributesArray);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Create bunch of inherited prefabs with selected models in m_aModelsToProcess array.
	//! By minimum, new prefabs will have attached models in MeshObject component and will inherit
	//! from m_sBaseClass prefab. They will be located in folder selected in m_sPathToSave & will
	//! have name based on XOB file.
	//!
	//! Optionally, you can also use m_sPrefabConfig to apply more values to your new prefab.
	//!
	//! If m_aNewNames array is used, number of entries in this array must match the number of models
	[ButtonAttribute("Create")]
	protected void Execute()
	{
		// Input parameters verification
		if (!m_aModelsToProcess)
		{
			Print("Models To Process is empty. Please fill it with at least one model", LogLevel.ERROR);
			return;
		}

		if (!m_sPathToSave)
		{
			Print("Path To Save is empty. Please select valid save location", LogLevel.ERROR);
			return;
		}

		if (!m_sBaseClass)
		{
			Print("Base Class is empty. Please select valid base prefab for your window", LogLevel.ERROR);
			return;
		}

		if (!m_bGenerateNames && !m_aNewNames)
		{
			Print("New Names and Generate Names are empty. Please either check Generate Names or type in new names", LogLevel.ERROR);
			return;
		}

		// Strip GUID
		if (m_sPathToSave.Contains("{") && m_sPathToSave.Contains("}"))
		{
			int guidBracket = m_sPathToSave.IndexOf("}");
			m_sPathToSave = m_sPathToSave.Substring(guidBracket+ 1, m_sPathToSave.Length() - guidBracket - 1);
		}

		// Get addon
		string addon = SCR_AddonTool.ToFileSystem(SCR_AddonTool.GetAddonID(m_iAddonToUse));

		// Get absolute path for CreateEntityTemplate
		string absPath;
		Workbench.GetAbsolutePath(addon + m_sPathToSave, absPath);

		string tempName;

		IEntitySource entitySource;
		array<ref ContainerIdPathEntry> containerPath;
		Resource holder;
		SCR_BatchPrefabTemplates prefabGeneratorConfig;
		int layerId = m_API.GetCurrentEntityLayerId();
		foreach (int currentIndex, ResourceName currentElement : m_aModelsToProcess)
		{
			// Load metafile and get resource GUID
			string modelName = currentElement;

			Print("Processing " + string.Format("@\"%1\"", currentElement.GetPath()), LogLevel.NORMAL);

			m_API.BeginEntityAction("Processing " + modelName);

			// Create entity with selected base class
			entitySource = m_API.CreateEntity(m_sBaseClass, string.Empty, layerId, null, vector.Zero, vector.Zero);

			containerPath = {};

			// Try to find MeshObject component and if it doesn't exist, then create new one
			if (!(SCR_BaseContainerTools.FindComponentSource(entitySource, "MeshObject")))
				m_API.CreateComponent(entitySource, "MeshObject");

			containerPath.Clear();
			containerPath.Insert(new ContainerIdPathEntry("MeshObject"));
			m_API.SetVariableValue(entitySource, containerPath, "Object", modelName);

			if (m_sPrefabConfig)
			{
				holder = BaseContainerTools.LoadContainer(m_sPrefabConfig);
				if (!holder)
					return;

				// Process attributes from config
				prefabGeneratorConfig = SCR_BatchPrefabTemplates.Cast(BaseContainerTools.CreateInstanceFromContainer(holder.GetResource().ToBaseContainer()));
				ProcessAttributes(entitySource, entitySource, containerPath, prefabGeneratorConfig.m_AttributesArray);
				ProcessComponents(entitySource, prefabGeneratorConfig);
			}

			// Generate new name based on xob file name. If not checked, then custom one is used
			if (m_bGenerateNames)
			{
				array<string> tempArray = {};
				modelName.Split("/", tempArray, false);
				tempName = tempArray[tempArray.Count()-1];
				tempName.Replace(".xob","");
			}
			else
			{
				tempName = m_aNewNames[currentIndex];
			}

			// Create file
			bool fileCreated = m_API.CreateEntityTemplate(entitySource, FilePath.Concat(absPath, tempName + ".et"));

			// End entity manipulation
			m_API.DeleteEntity(entitySource);
			m_API.EndEntityAction();
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Modify all prefabs listed in m_aPrefabsToProcess with instructions contained in m_sPrefabConfig.
	//! After values are changed, function is also triggering saving of currently open scene
	//! World Editor to store all changes on the user drive.
	[ButtonAttribute("Modify")]
	protected void ExecuteModify()
	{
		// Input parameters verification
		if (!m_aPrefabsToProcess && !m_sDirectoryToProcess)
		{
			Print("m_aPrefabsToProcess & m_sDirectoryToProcess is empty. Please fill with at least one prefab", LogLevel.ERROR);
			return;
		}

		if (!m_sPrefabConfig)
		{
			Print("Missing config file for batch prefab operation!", LogLevel.ERROR);
			return;
		}

		if (!m_sDirectoryToProcess.IsEmpty())
		{
			string addon = SCR_AddonTool.ToFileSystem(SCR_AddonTool.GetAddonID(m_iAddonToUse));

			// Get all prefabs from the selected subdirectory
			SearchResourcesFilter filter = new SearchResourcesFilter();
			filter.fileExtensions = { "et" };
			filter.rootPath = addon + m_sDirectoryToProcess.GetPath();
			ResourceDatabase.SearchResources(filter, m_aPrefabsToProcess.Insert);

			Print("Processing " + m_aPrefabsToProcess.Count(), LogLevel.NORMAL);

			// Exit if there are no valid prefabs in selected folder
			if (m_aPrefabsToProcess.IsEmpty())
			{
				Print("There are no valid prefabs in selected folder", LogLevel.ERROR);
				return;
			}
		}

		IEntitySource entitySource;
		IEntitySource prefab;
		array<ref ContainerIdPathEntry> containerPath;
		Resource holder;
		SCR_BatchPrefabTemplates prefabGeneratorConfig;
		int layerId = m_API.GetCurrentEntityLayerId();
		foreach (int currentIndex, ResourceName currentElement : m_aPrefabsToProcess)
		{
			// Load metafile and get resource GUID
			string modelName = currentElement;

			Print("Processing " + string.Format("@\"%1\"", currentElement.GetPath()), LogLevel.NORMAL);
			//m_API.BeginEntityAction("Processing - " + modelName);

			// Create entity with selected base class
			entitySource = m_API.CreateEntity(currentElement, string.Empty, layerId, null, vector.Zero, vector.Zero);

			prefab = entitySource.GetAncestor();

			containerPath = {};

			// Load config and verify if its valid one
			holder = BaseContainerTools.LoadContainer(m_sPrefabConfig);
			if (!holder)
				return;

			// Process all attributes from config
			prefabGeneratorConfig = SCR_BatchPrefabTemplates.Cast(BaseContainerTools.CreateInstanceFromContainer(holder.GetResource().ToBaseContainer()));
			ProcessAttributes(prefab, prefab, containerPath, prefabGeneratorConfig.m_AttributesArray);
			ProcessComponents(prefab, prefabGeneratorConfig);

			// if we use 'entitySource' to modify prefab instance
			// if we use 'entitySource.GetAncestor()' to modify prefab (first prefab in prefab hierarchy)
			m_API.DeleteEntity(entitySource);
			//m_API.EndEntityAction();
		}

		WorldEditor worldEditor = Workbench.GetModule(WorldEditor);
		worldEditor.Save(); // invoke world editor save to save changes to the prefab
	}
}

//! Custom title for BatchPrefabTemplates config parameters
class SCR_BatchPrefabTitle : BaseContainerCustomTitle
{
	protected string m_aParamString[4];

	//------------------------------------------------------------------------------------------------
	// constructor
	void SCR_BatchPrefabTitle(string paramString1, string paramString2 = "", string paramString3 = "", string paramString4 = "")
	{
		m_aParamString[0] = paramString1;
		m_aParamString[1] = paramString2;
		m_aParamString[2] = paramString3;
		m_aParamString[3] = paramString4;
	}

	//------------------------------------------------------------------------------------------------
	override bool _WB_GetCustomTitle(BaseContainer source, out string title)
	{
		string temp;
		string prop;
		for (int i = 1; i <= 4; i++)
		{
			temp = "";
			prop = m_aParamString[i - 1];
			if (!source.Get(prop, temp))
				temp = prop;

			title = title + temp;
		}

		return title != string.Empty;
	}
}

//! Unique flags for the batch operations
enum SCR_EBatchPrefabOperationFlag
{
	REPLACE = 1, ///< Default state - replace current value with anything that
	MATH = 2, ///< Math operations are performed on the attribute
	ENUM = 4 ///< Bitwise operations on enum
}

enum SCR_EBatchPrefabOperationComponentFlag
{
	CREATE = 1, ///< Create new component - if component exist then it will be modified
	MODIFY = 2, ///< Only modifies component - if component is missing then all actions related to this component will be skipped
	DELETE = 4 ///< Delete component
}

[BaseContainerProps(configRoot: true)]
class SCR_BatchPrefabTemplates
{
	[Attribute(desc: "Array of attributes which be aplied to prefab (not to its components)")]
	ref array<ref SCR_BatchPrefabTemplatesAttributes> m_AttributesArray;

	[Attribute(desc: "Array of components which will be added or modified")]
	ref array<ref SCR_BatchPrefabTemplatesData> m_ComponentsArray;
}

[BaseContainerProps(configRoot: false), SCR_BatchPrefabTitle("Component: ", "m_ComponentName")]
class SCR_BatchPrefabTemplatesData
{
	[Attribute(desc: "Name of component to modify or add")]
	string m_ComponentName;

	[Attribute(defvalue: "1", UIWidgets.ComboBox, desc: "Create new component if its missing prefab", enumType: SCR_EBatchPrefabOperationComponentFlag)]
	SCR_EBatchPrefabOperationComponentFlag m_ComponentAction;

	[Attribute(desc: "Array of attributes which will be modified inside component belonging to prefab. If component is not present, new one will be created")]
	ref array<ref SCR_BatchPrefabTemplatesAttributes> m_ComponentAttributesArray;
}

[BaseContainerProps(configRoot: false), SCR_BatchPrefabTitle("m_AttributeName",": ","m_AttributeValue")]
class SCR_BatchPrefabTemplatesAttributes
{
	[Attribute(desc: "Name of attribute to modify")]
	string m_AttributeName;

	[Attribute(desc: "Value which will be stored in prefab. Simple math (adding, multiplying & dividing) & bitwise operations are allowed (use + to add and - to remove flag)")]
	string m_AttributeValue;

	[Attribute("1", UIWidgets.ComboBox, category: "", desc: "Specifies type of operation that is performed.", enumType: SCR_EBatchPrefabOperationFlag)]
	SCR_EBatchPrefabOperationFlag m_AttributeOperation;

	[Attribute(desc: "Feature not implemented yet!")]
	ref array<ref SCR_BatchPrefabTemplatesAttributes> m_SubAttributesArray;
}
#endif // WORKBENCH
