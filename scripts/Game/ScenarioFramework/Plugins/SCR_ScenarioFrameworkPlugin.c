[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkPlugin : BaseContainerObject
{
	[Attribute(desc: "If set to true, when this Plugin performs certain methods, it will break the breakpoint in the Script Editor in respective methods. This can be also set during runtime via Debug Menu > ScenarioFramework > Plugin Inspector")]
	bool m_bDebug;
	
	SCR_ScenarioFrameworkLayerBase m_Object;

	//------------------------------------------------------------------------------------------------
	//! \return the object associated with this scenario layer.
	SCR_ScenarioFrameworkLayerBase GetObject()
	{
		return m_Object;
	}

	//------------------------------------------------------------------------------------------------
	//! Initializes scenario framework plugin with provided object, logs debug information if debug mode is enabled.
	//! \param[in] object Initializes scenario framework plugin with provided object, debugs if enabled.
	void Init(SCR_ScenarioFrameworkLayerBase object)
	{
		// Here you can debug specific Plugin instance.
		// This can be also adjusted during runtime via Debug Menu > ScenarioFramework > Plugin Inspector
		if (m_bDebug)
			Print("[SCR_ScenarioFrameworkPlugin.Init] debug line (" + __FILE__ + " L" + __LINE__ + ")", LogLevel.WARNING);
		
		m_Object = object;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] object Represents scenario framework layer base object for event handling in ArmA scenario.
	void OnWBKeyChanged(SCR_ScenarioFrameworkLayerBase object)
	{
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return[out] Actions
	array<ref SCR_ScenarioFrameworkActionBase> GetActions();
}