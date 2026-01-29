[BaseContainerProps()]
class SCR_IsBallisticDataDisplayVisibleCondition : SCR_AvailableActionCondition
{
	//------------------------------------------------------------------------------------------------
	//! Return true if character is in ADS of current controlled vehicle turret
	override bool IsAvailable(SCR_AvailableActionsConditionData data)
	{
		SCR_AdditionalGameModeSettingsComponent additionalGameSettings = SCR_AdditionalGameModeSettingsComponent.GetInstance();

		return GetReturnResult(additionalGameSettings && additionalGameSettings.GetProjectileBallisticInfoVisibility());
	}
}