[ComponentEditorProps(category: "GameScripted/Misc", description: "")]
class SCR_PlayerRestrictionZoneWarningComponentClass : ScriptComponentClass
{
}

class SCR_PlayerRestrictionZoneWarningComponent : ScriptComponent
{
	protected SCR_RestrictionZoneWarningHUDComponent m_WarningHUD;
	protected bool m_bShowingWarning;
	protected float m_fWarningEffectStrenghtPerc;
	protected float m_fWarningEffectStrenghtLookAtPerc;
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] showWarning
	//! \param[in] warningType
	//! \param[in] centerChanged
	//! \param[in] zoneCenter
	//! \param[in] warningRadiusSq
	//! \param[in] zoneRadiusSq
	void ShowWarningServer(bool showWarning, ERestrictionZoneWarningType warningType, bool centerChanged, vector zoneCenter, float warningRadiusSq, float zoneRadiusSq)
	{
		if ((m_bShowingWarning == showWarning && !centerChanged) || !Replication.IsServer())
			return;
		
		m_bShowingWarning = showWarning;
		
		if (showWarning)
			Rpc(ShowWarningOwner, showWarning, warningType, zoneCenter, warningRadiusSq, zoneRadiusSq);
		else 
			Rpc(HideWarningOwner);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void HideWarningOwner()
	{
		if (m_WarningHUD)
			m_WarningHUD.ShowZoneWarning(false, -1, vector.Zero, -1, -1);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void ShowWarningOwner(bool showWarning, ERestrictionZoneWarningType warningType, vector zoneCenter, float warningRadiusSq, float zoneRadiusSq)
	{		
		if (m_WarningHUD)
			m_WarningHUD.ShowZoneWarning(showWarning, warningType, zoneCenter, warningRadiusSq, zoneRadiusSq);
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		SCR_HUDManagerComponent hudManager = SCR_HUDManagerComponent.Cast(owner.FindComponent(SCR_HUDManagerComponent));
		if (hudManager)
		{
			array<BaseInfoDisplay> infoDisplays = new array<BaseInfoDisplay>;
			int count = hudManager.GetInfoDisplays(infoDisplays);
		
			for(int i = 0; i < count; i++)
       		{
	            if (infoDisplays[i].Type() == SCR_RestrictionZoneWarningHUDComponent)
				{
					m_WarningHUD = SCR_RestrictionZoneWarningHUDComponent.Cast(infoDisplays[i]);
					if (m_WarningHUD)
						break;
				}
       	 	}
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		SetEventMask(owner, EntityEvent.INIT);
	}
}
