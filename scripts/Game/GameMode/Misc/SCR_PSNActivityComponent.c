class SCR_PSNActivityComponentClass : SCR_BaseGameModeComponentClass
{
}

class SCR_PSNActivityComponent : SCR_BaseGameModeComponent
{
	[Attribute(defvalue: "Default value", desc: "Activity ID")]
	protected string m_sActivityID;
	
	[Attribute(defvalue: "Default value", desc: "Next Activity ID")]
	protected string m_sNextActivityID;
	
	//------------------------------------------------------------------------------------------------
	override void OnGameModeStart()
	{
		GetGame().GetPlatformService().StartSingleplayerActivity(m_sActivityID);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnGameModeEnd(SCR_GameModeEndData data)
	{
		if (data.GetEndReason() == EGameOverTypes.VICTORY)
		{
			GetGame().GetPlatformService().EndSingleplayerActivity(m_sActivityID, "completed");
			
			if (!m_sNextActivityID.IsEmpty())
				GetGame().GetPlatformService().ChangeAvailabilitySingleplayerActivity({m_sNextActivityID}, {});
		}
		else
		{
			GetGame().GetPlatformService().EndSingleplayerActivity(m_sActivityID, "failed");
		}
	}
}