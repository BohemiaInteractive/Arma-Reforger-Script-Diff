//------------------------------------------------------------------------------------------------
//! Map module base class
[BaseContainerProps()]
class SCR_MapModuleBase : Managed
{
	[Attribute("0", UIWidgets.Auto, "Disable this module, useful for example when we want to inherit config without a specific module" )]
	protected bool m_bDisableModule;
	
	protected Widget m_wRootWidget;	
	protected SCR_MapEntity m_MapEntity;
	
	//------------------------------------------------------------------------------------------------
	//! SCR_MapEntity event
	protected void OnMapOpen(MapConfiguration config)
	{}
	
	//------------------------------------------------------------------------------------------------
	//! SCR_MapEntity event
	protected void OnMapClose(MapConfiguration config)
	{}
	
	//------------------------------------------------------------------------------------------------
	//! Whether this module is disabled in config
	bool IsConfigDisabled()
	{
		return m_bDisableModule;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Enable open/close events
	//! \param active is target state
	//! \param isCleanup determines if this is just deactivation or a cleanup
	void SetActive(bool active, bool isCleanup = false)
	{
		if (active)
		{
			m_wRootWidget = m_MapEntity.GetMapConfig().RootWidgetRef; // Needs to be refreshed here
			
			m_MapEntity.GetOnMapOpen().Insert(OnMapOpen);
			m_MapEntity.GetOnMapClose().Insert(OnMapClose);
		}
		else 
		{
			m_MapEntity.GetOnMapOpen().Remove(OnMapOpen);
			m_MapEntity.GetOnMapClose().Remove(OnMapClose);
			
			if (!isCleanup)
				m_MapEntity.DeactivateModule(this);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Init method for cases where all modules and components should be loaded already so constructor cannot be used, called once after creation
	void Init()
	{}
	
	//------------------------------------------------------------------------------------------------
	//! Update method for frame operations
	void Update(float timeSlice)
	{}
	
	//------------------------------------------------------------------------------------------------
	//! Dont use arguments in the contructor for this class or its children
	void SCR_MapModuleBase()
	{
		m_MapEntity = SCR_MapEntity.GetMapInstance();
	}
};
