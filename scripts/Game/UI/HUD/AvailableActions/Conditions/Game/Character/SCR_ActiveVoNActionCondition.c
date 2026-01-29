//! Returns true if VoN UI is active
[BaseContainerProps(), BaseContainerCustomStringTitleField("VoN is Active")]
class SCR_ActiveVoNActionCondition: SCR_AvailableActionCondition
{	
	protected SCR_VonDisplay m_VoNDisplay;
	
	override bool IsAvailable(SCR_AvailableActionsConditionData data)
	{
		if (!data)
			return false;
		
		if (!m_VoNDisplay)
		{
			PlayerController playerController = GetGame().GetPlayerController();
			
			if (!playerController)
				return GetReturnResult(false);
			
			SCR_VONController voNController = SCR_VONController.Cast(playerController.FindComponent(SCR_VONController));
			
			if (!voNController)
				return GetReturnResult(false);
			
			m_VoNDisplay = voNController.GetDisplay();
			
			if (!m_VoNDisplay)
				return GetReturnResult(false);
		}

		return GetReturnResult(m_VoNDisplay.GetActiveTransmissionsCount() > 0 || m_VoNDisplay.IsCapturingTransmisionActive());
	}
};