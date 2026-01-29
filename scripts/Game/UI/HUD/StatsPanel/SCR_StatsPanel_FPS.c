enum EStatsPanelFPS
{
	LOCAL,
	SERVER
};

class SCR_StatsPanel_FPS : SCR_StatsPanelBase
{
	[Attribute(SCR_Enum.GetDefault(EStatsPanelFPS.LOCAL), UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(EStatsPanelFPS))]
	EStatsPanelFPS m_eFPSLocality;	
	
	SCR_NetworkedStatsComponent m_StatsComponent;
	int m_iFPS;
	
	//------------------------------------------------------------------------------------------------
	override bool DisplayStartDrawInit(IEntity owner)
	{
		if (!super.DisplayStartDrawInit(owner))
			return false;

		/*		
		if (SCR_Global.IsEditMode())
			return false;
		*/

		m_StatsComponent = SCR_NetworkedStatsComponent.Cast(m_PlayerController.FindComponent(SCR_NetworkedStatsComponent));
		m_StatsComponent.GetNetworkedStatPrefabData().GetOnStatsChanged(true).Insert(OnStatUpdate);		
		
		return true;
	}

	void OnStatUpdate(SCR_BaseNetworkedStat stat)
	{
		SCR_FPSNetworkedStat fpsStat = SCR_FPSNetworkedStat.Cast(stat);
		
		if (!fpsStat)
			return;
		
		if (m_eFPSLocality == EStatsPanelFPS.LOCAL)
		{
			m_iFPS = fpsStat.GetLastCheckedLocalFPS();
		}
		else
		{
			m_iFPS = fpsStat.GetLastCheckedAuthorityFPS();
		}
	}	
		
	//------------------------------------------------------------------------------------------------
	override protected float GetValue()
	{
		#ifdef DEBUG_STATS_PANELS
		return Math.RandomInt(10,80);
		#endif	
		
		return m_iFPS;
	}
}