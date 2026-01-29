//
////------------------------------------------------------------------------------------------------
//class SCR_RequestReinforcementsMessage : ScriptedRadioMessage
//{
//	protected SCR_CampaignMilitaryBaseComponent m_TargetBase;
//	protected SCR_CampaignFaction m_TargetFaction;
//	
//	//------------------------------------------------------------------------------------------------
//	override void OnDelivery(BaseTransceiver receiver, int freq, float quality)
//	{
//		if (m_TargetBase && m_TargetFaction && receiver.GetRadio() && receiver.GetRadio().GetOwner() == m_TargetFaction.GetMainBase() && GetTaskManager())
//		{
//			SCR_CampaignDefendTaskSupportEntity supportEntity = SCR_CampaignDefendTaskSupportEntity.Cast(GetTaskManager().FindSupportEntity(SCR_CampaignDefendTaskSupportEntity));
//			if (supportEntity)
//				supportEntity.CreateCampaignDefendTask(m_TargetBase, m_TargetFaction);
//		}
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	void SetTargetFaction(SCR_CampaignFaction targetFaction)
//	{
//		m_TargetFaction = targetFaction;
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	void SetTargetBase(SCR_CampaignMilitaryBaseComponent targetBase)
//	{
//		m_TargetBase = targetBase;
//	}
//};
