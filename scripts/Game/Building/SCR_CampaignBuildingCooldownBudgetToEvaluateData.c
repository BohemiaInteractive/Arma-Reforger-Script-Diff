[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(EEditableEntityBudget, "m_iBudget")]
//------------------------------------------------------------------------------------------------
class SCR_CampaignBuildingCooldownBudgetToEvaluateData : SCR_CampaignBuildingBudgetToEvaluateData
{
	//------------------------------------------------------------------------------------------------
	//! In this method the cooldown time can be modified in inherited classes.
	//! \param[in] playerId
	//! \param[in] originalCooldown
	int CooldownTimeModifier(int playerId, int originalCooldown)
	{
		return originalCooldown;
	}
}
