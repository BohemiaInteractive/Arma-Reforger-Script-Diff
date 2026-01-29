class DeployMenuSystem : GameSystem
{
	override static void InitInfo(WorldSystemInfo outInfo)
	{
		outInfo
			.SetAbstract(false)
			.AddPoint(ESystemPoint.Frame);
	}

	protected SCR_PlayerDeployMenuHandlerComponent m_LocalMenuHandler;
	protected SCR_RespawnSystemComponent m_RespawnSystem;
	protected bool m_bReady = false;

	//------------------------------------------------------------------------------------------------
	protected override void OnInit()
	{
		m_RespawnSystem = SCR_RespawnSystemComponent.GetInstance();
		if (m_RespawnSystem)
		{
			if (m_RespawnSystem.UsesLoadingPlaceholder())
				return; // System is needed to animate the placeholder
			
			auto spawnLogic = m_RespawnSystem.GetSpawnLogic();
			if (spawnLogic && spawnLogic.IsInherited(SCR_MenuSpawnLogic))
				return; // System is needed to update deploy menu logic
		}

		Enable(false); // No loading anim menu logic to update.
	}

	//------------------------------------------------------------------------------------------------
	void Register(notnull SCR_PlayerDeployMenuHandlerComponent handler)
	{
		if (handler.GetPlayerController() == GetGame().GetPlayerController())
			m_LocalMenuHandler = handler;
	}

	//------------------------------------------------------------------------------------------------
	void Unregister(notnull SCR_PlayerDeployMenuHandlerComponent handler)
	{
		if (handler.GetPlayerController() == GetGame().GetPlayerController())
			m_LocalMenuHandler = null;
	}

	//------------------------------------------------------------------------------------------------
	//! Indicate that the deploy menu was opened and is ready for user interaction.
	void SetReady(bool ready)
	{
		m_bReady = ready;
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnUpdatePoint(WorldUpdatePointArgs args)
	{
		float dt = args.GetTimeSliceSeconds();
		
		if (m_RespawnSystem)
			m_RespawnSystem.UpdateLoadingPlaceholder(dt);

		if (m_bReady && m_LocalMenuHandler)
			m_LocalMenuHandler.Update(dt);
	}
}
