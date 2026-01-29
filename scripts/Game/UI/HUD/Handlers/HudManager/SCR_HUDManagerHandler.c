[BaseContainerProps()]
class SCR_HUDManagerHandler
{
	protected SCR_HUDManagerComponent m_HUDManager;

	[Attribute()]
	protected bool m_bCanUpdate;

	//------------------------------------------------------------------------------------------------
	bool CanUpdate()
	{
		return m_bCanUpdate;
	}

	//------------------------------------------------------------------------------------------------
	void SetCanUpdate(bool canUpdate)
	{
		m_bCanUpdate = canUpdate;
	}
	
	//------------------------------------------------------------------------------------------------
	void OnInit(notnull SCR_HUDManagerComponent owner)
	{
		m_HUDManager = owner;
	}

	//------------------------------------------------------------------------------------------------
	void OnStart(notnull SCR_HUDManagerComponent owner);

	//------------------------------------------------------------------------------------------------
	void OnUpdate(notnull SCR_HUDManagerComponent owner);

	//------------------------------------------------------------------------------------------------
	void OnStop(notnull SCR_HUDManagerComponent owner);
}
