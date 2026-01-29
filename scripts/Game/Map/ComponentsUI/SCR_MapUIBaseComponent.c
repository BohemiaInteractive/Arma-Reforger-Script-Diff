// Base for 2D map UI components
[BaseContainerProps()]
class SCR_MapUIBaseComponent : ScriptedWidgetComponent
{		
	[Attribute("0", UIWidgets.Auto, "Disable this component, useful for example when we want to inherit config without a specific component" )]
	protected bool m_bDisableComponent;
	
	[Attribute("0", desc: "All other exclusive Map UI Components are disabled when this component is enabled")]
	protected bool m_bIsExclusive;

	protected bool m_bHookToRoot = false;							// determine whether this component is hooked to the root widget for use of ScriptedWidgetEventHandler events
	protected Widget m_RootWidget;									// map layout root widget	
	protected SCR_MapEntity m_MapEntity;	
						
	//------------------------------------------------------------------------------------------------
	// BASE METHODS
	//------------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	//! SCR_MapEntity event
	//! \param[in] config
	protected void OnMapOpen(MapConfiguration config)
	{	
		if (m_bHookToRoot)
			m_RootWidget.AddHandler(this);
	}
	
	//------------------------------------------------------------------------------------------------
	//! SCR_MapEntity event
	//! \param[in] config
	protected void OnMapClose(MapConfiguration config)
	{
		if (m_bHookToRoot)
			m_RootWidget.RemoveHandler(this);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return true if this component is disabled in config, false otherwise
	bool IsConfigDisabled()
	{
		return m_bDisableComponent;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return true if this component should disable other exclusive UI components on the map when toggled on
	bool IsExclusive()
	{
		return m_bIsExclusive;
	}

	//------------------------------------------------------------------------------------------------
	//! Enable open/close events, called every time component is activated, usually on map open
	//! \param[in] active is target state
	//! \param[in] isCleanup determines if this is just deactivation or a cleanup
	void SetActive(bool active, bool isCleanup = false)
	{
		if (active)
		{
			m_RootWidget = m_MapEntity.GetMapConfig().RootWidgetRef; // Needs to be refreshed here
			
			m_MapEntity.GetOnMapOpen().Insert(OnMapOpen);
			m_MapEntity.GetOnMapClose().Insert(OnMapClose);
		}
		else 
		{
			m_MapEntity.GetOnMapOpen().Remove(OnMapOpen);
			m_MapEntity.GetOnMapClose().Remove(OnMapClose);
				
			if (!isCleanup)	
				m_MapEntity.DeactivateComponent(this);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Init method for cases where all modules and components should be loaded already so constructor cannot be used, called once after creation
	void Init();
	
	//------------------------------------------------------------------------------------------------
	//! Update method for frame operations
	void Update(float timeSlice);
	
	//------------------------------------------------------------------------------------------------	
	bool IsToolMenuFocused()
	{
		SCR_MapToolMenuUI toolMenu = SCR_MapToolMenuUI.Cast(m_MapEntity.GetMapUIComponent(SCR_MapToolMenuUI));
		if (toolMenu)
			return toolMenu.IsFocused();

		return false;
	}

	//------------------------------------------------------------------------------------------------
	// constructor
	void SCR_MapUIBaseComponent()
	{
		m_MapEntity = SCR_MapEntity.GetMapInstance();
	}
}
