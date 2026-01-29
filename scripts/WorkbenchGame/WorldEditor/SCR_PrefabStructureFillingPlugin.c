#ifdef WORKBENCH
[WorkbenchPluginAttribute(
	name: "Prefab Structure Filling",
	description: "Fill Prefab structure according based on the selected type",
	category: "Prefab Structure",
	wbModules: { "WorldEditor" },
	awesomeFontCode: 0xF6BE)]
class SCR_PrefabStructureFillingPlugin : WorkbenchPlugin
{
	//------------------------------------------------------------------------------------------------
	protected override void Run()
	{
		IEntitySource entitySource = SCR_WorldEditorToolHelper.GetPrefabEditModeEntitySource();
		if (!entitySource)
		{
			Print("Not in Prefab Edit mode or another error somewhere", LogLevel.WARNING);
			return;
		}

		SCR_PrefabStructureFillingPlugin_TypeSelection typeSelection = new SCR_PrefabStructureFillingPlugin_TypeSelection();
		if (!Workbench.ScriptDialog("Pick your poison", "So, whaddaya want?", typeSelection) || !typeSelection.m_sChoiceName) // .IsEmpty()
			return;
 
		typename selectedClass = ("SCR_PrefabStructureFillingPlugin_" + typeSelection.m_sChoiceName).ToType();
		if (!selectedClass)
		{
			Print("Configuration class does not exist - (SCR_PrefabStructureFillingPlugin_)" + typeSelection.m_sChoiceName, LogLevel.WARNING);
			return;
		}

		SCR_PrefabStructureFillingPlugin_TypeBase configuration = SCR_PrefabStructureFillingPlugin_TypeBase.Cast(selectedClass.Spawn());
		if (!configuration)
		{
			Print("Cannot create configuration instance for " + typeSelection.m_sChoiceName, LogLevel.WARNING);
			return;
		}

		if (!Workbench.ScriptDialog(typeSelection.m_sChoiceName + " Configuration", "Please configure", configuration))
		{
			if (!configuration.m_bClose)
				Run(); // beware of stack

			return;
		}

		IEntitySource actualPrefab = entitySource.GetAncestor();
		if (!actualPrefab)
		{
			Print("Currently edited entity does not have a Prefab", LogLevel.WARNING);
			return;
		}

		configuration.Process(actualPrefab);
	}
}

class SCR_PrefabStructureFillingPlugin_TypeSelection
{
	// buttons for now, combobox when there are too many options
//	[Attribute(defvalue: "Car", uiwidget: UIWidgets.ComboBox, desc: "Choice Name", params: "0 inf", enums: {
//		new ParamEnum("Animal", "Animal"),
//		new ParamEnum("Character - Male", "CharacterM"),
//		new ParamEnum("Character - Female", "CharacterF"),
//		new ParamEnum("Civilian Car", "Car"),
//		new ParamEnum("Military Car", "MilitaryCar"),
//		new ParamEnum("Tank/Armored", "Tank"),
//		new ParamEnum("Helicopter", "Helicopter"),
//		new ParamEnum("Civilian airplane", "Airplane"),
//		new ParamEnum("Jet", "Jet"),
//		// etc
//	})]
//	string m_sChoiceName2 = "Civilian Car";
//	[Attribute(defvalue: "Car", uiwidget: UIWidgets.ComboBox, desc: "Choice Name", params: "0 inf", enums: SCR_ParamEnumArray.FromString(
//		"Animal,Animal,"
//		+ ";CharacterM,Character - Male,"
//		+ ";CharacterF,Character - Female,"
//		+ ";Car,Civilian Car,"
//		+ ";MilitaryCar,Military Car,"
//		+ ";Tank,Tank/Armored,"
//		+ ";Airplane,Civilian airplane,"
//		+ ";Jet,Jet,"
//	))]
//	string m_sChoiceName3 = "Civilian Car";

	string m_sChoiceName;

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Character")]
	protected int ButtonCharacter()
	{
		m_sChoiceName = "Character";
		return 1;
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Car")]
	protected int ButtonCar()
	{
		m_sChoiceName = "Car";
		return 1;
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Tank")]
	protected int ButtonTank()
	{
		m_sChoiceName = "Tank";
		return 1;
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Helicopter")]
	protected int ButtonHelicopter()
	{
		m_sChoiceName = "Helicopter";
		return 1;
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Cancel")]
	protected int ButtonCancel()
	{
		return 0;
	}
}

//! Children properties require manually set defvalue
//! \code
//! [Attribute(defvalue: "42")]
//! protected int m_iValue = 42; // here
//! \endcode
//! as these objects are spawned by script, not by Workbench
class SCR_PrefabStructureFillingPlugin_TypeBase
{
	// [Attribute(defvalue: "42")]
	// protected int m_iValue = 42; // here

	bool m_bClose;

	//! do NOT usually keep reference to WorldEditorAPI
	//! here it is a special case where this class
	//! - checks its availability
	//! - sets its value for child classes to use it straight away
	protected WorldEditorAPI m_WorldEditorAPI;

	//------------------------------------------------------------------------------------------------
	//! Process the provided Prefab according to the class type
	//! \param[in] prefab the actual Prefab (editModePrefab.GetAncestor())
	//! \return true on success, false otherwise
	bool Process(notnull IEntitySource prefab)
	{
		if (!prefab.GetResourceName())
		{
			Print("Passed value is not a Prefab - " + prefab, LogLevel.ERROR);
			return false;
		}

		m_WorldEditorAPI = SCR_WorldEditorToolHelper.GetWorldEditorAPI();
		if (!m_WorldEditorAPI)
		{
			Print("World Editor API is not available", LogLevel.ERROR);
			return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("← Back")]
	protected int ButtonBack()
	{
		return 0;
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Process", true)]
	protected int ButtonOK()
	{
		return 1;
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Cancel")]
	protected int ButtonCancel()
	{
		m_bClose = true;
		return 0;
	}
}

#endif // WORKBENCH
