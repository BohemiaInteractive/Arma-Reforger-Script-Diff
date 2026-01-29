[BaseContainerProps(), SCR_BaseContainerCustomTitleUIInfo("m_Info")]
class SCR_KickPlayerContextAction: SCR_SelectedEntitiesContextAction
{
	[Attribute("0", desc: "How long before kicked-out player can reconnect.\n-1 means permanent ban (at least until exe restart)")]
	protected int m_iKickTimeout;
	
	[Attribute(uiwidget: UIWidgets.SearchComboBox, desc: "Reason for kicking shown to kicked-out player.", enums: ParamEnumArray.FromEnum(SCR_PlayerManagerKickReason))]
	protected SCR_PlayerManagerKickReason m_KickReason;
	
	[Attribute(ETimeFormatParam.SECONDS.ToString(), desc: "How will the kick time out be converted and displayed within the UIInfo. EG: As Seconds, Minutes, Hours or Days.\n\nNot applicable if time is 0 or less", uiwidget: UIWidgets.SearchComboBox, enums: ParamEnumArray.FromEnum(ETimeFormatParam))]
	protected ETimeFormatParam m_eKickDurationConverter;
	
	//------------------------------------------------------------------------------------------------
	override bool CanBeShown(SCR_EditableEntityComponent selectedEntity, vector cursorWorldPosition, int flags)
	{
		SCR_EditablePlayerDelegateComponent playerDelegate = SCR_EditablePlayerDelegateComponent.Cast(selectedEntity);
		return playerDelegate && !SCR_Global.IsAdmin(playerDelegate.GetPlayerID());
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBePerformed(SCR_EditableEntityComponent selectedEntity, vector cursorWorldPosition, int flags)
	{
		return CanBeShown(selectedEntity, cursorWorldPosition, flags);
	}
	
	//------------------------------------------------------------------------------------------------
	override void Perform(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition,int flags, int param = -1)
	{
		SCR_EditablePlayerDelegateComponent playerDelegate;		
		foreach (SCR_EditableEntityComponent entity: selectedEntities)
		{
			playerDelegate = SCR_EditablePlayerDelegateComponent.Cast(entity);
			if (playerDelegate)
			{
				GetGame().GetPlayerManager().KickPlayer(playerDelegate.GetPlayerID(), m_KickReason, m_iKickTimeout);
			}			
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override SCR_UIInfo GetInfo()
	{
		SetKickDurationInUIInfo();
		return super.GetInfo();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetKickDurationInUIInfo()
	{
		//~ To time set or perma ban
		if (m_iKickTimeout <= 0)
			return;
		
		SCR_FormatedUIInfo formattedUIInfo = SCR_FormatedUIInfo.Cast(m_Info);
		if (!formattedUIInfo)
			return;
		
		string param1;
		formattedUIInfo.GetParams(param1);
		if (!param1.IsEmpty())
			return;
		
		switch (m_eKickDurationConverter)
		{
			case ETimeFormatParam.SECONDS:
			{
				formattedUIInfo.SetParams(m_iKickTimeout.ToString());
				break;
			}
			case ETimeFormatParam.MINUTES:
			{
				formattedUIInfo.SetParams((m_iKickTimeout / 60).ToString());
				break;
			}
			case ETimeFormatParam.HOURS:
			{
				formattedUIInfo.SetParams((m_iKickTimeout / 3600).ToString());
				break;
			}
			case ETimeFormatParam.DAYS:
			{
				formattedUIInfo.SetParams((m_iKickTimeout / 86400).ToString());
				break;
			}
		}
	}
};