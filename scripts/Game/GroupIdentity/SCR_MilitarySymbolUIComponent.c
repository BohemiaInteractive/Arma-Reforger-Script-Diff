class SCR_MilitarySymbolUIComponent : ScriptedWidgetComponent
{
	protected const ResourceName DEBUG_CONFIG = "{9D381AA96A15D6B4}Configs/GroupIdentity/MilitarySymbolConfig.conf";
	
	[Attribute(desc: "Initial military symbol data. When undefined, it has to be set by script.")]
	protected ref SCR_MilitarySymbol m_MilitarySymbol;
	
	protected Widget m_Widget;
	
	//------------------------------------------------------------------------------------------------
	//! Update the widget according to provided military symbol configuration.
	//! \param[in] symbol Military symbol
	//! \param[in] config GUI config influencing visuals
	void Update(notnull SCR_MilitarySymbol symbol, SCR_MilitarySymbolConfig config = null)
	{
		//--- Use default config
		if (!config)
		{
			SCR_GroupIdentityCore core = SCR_GroupIdentityCore.Cast(SCR_GroupIdentityCore.GetInstance(SCR_GroupIdentityCore));
			if (core)
				config = core.GetSymbols();
		}

		if (!config)
		{
			Print("Cannot show military symbol, config not found!", LogLevel.WARNING);
			return;
		}
		
		SCR_WidgetTools.RemoveChildrenFromHierarchy(m_Widget);
		config.CreateWidgets(symbol, m_Widget);
	}
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		m_Widget = w;

		if (m_MilitarySymbol)
		{
#ifdef WORKBENCH
			//--- Core not loaded when the game is running, use debug config
			Resource resource = Resource.Load(DEBUG_CONFIG);
			if (!resource.IsValid())
			{
				Print("Invalid config " + DEBUG_CONFIG, LogLevel.WARNING);
				return;
			}

			SCR_MilitarySymbolConfig config = SCR_MilitarySymbolConfig.Cast(BaseContainerTools.CreateInstanceFromContainer(resource.GetResource().ToBaseContainer()));
			Update(m_MilitarySymbol, config);
#else
			Update(m_MilitarySymbol);
#endif
		}
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
	}
}
