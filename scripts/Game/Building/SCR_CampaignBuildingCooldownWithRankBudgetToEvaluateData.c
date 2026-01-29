[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(EEditableEntityBudget, "m_iBudget")]
//------------------------------------------------------------------------------------------------
class SCR_CampaignBuildingCooldownWithRankBudgetToEvaluateData : SCR_CampaignBuildingCooldownBudgetToEvaluateData
{
	[Attribute(desc: "Custom cooldown time for defined ranks.")]
	protected ref array<ref SCR_CooldownData> m_aCustomCooldownData;

	//------------------------------------------------------------------------------------------------
	//! Modify the cooldown time based on the player rank.
	override int CooldownTimeModifier(int playerId, int originalCooldown)
	{
		if (m_aCustomCooldownData.IsEmpty())
			return originalCooldown;

		PlayerManager playerManager = GetGame().GetPlayerManager();
		if (!playerManager)
			return originalCooldown;

		IEntity playerEntity = playerManager.GetPlayerControlledEntity(playerId);
		if (!playerEntity)
			return originalCooldown;

		foreach (SCR_CooldownData customCooldown : m_aCustomCooldownData)
		{
			if (customCooldown.GetRank() == SCR_CharacterRankComponent.GetCharacterRank(playerEntity))
				return originalCooldown * customCooldown.GetTimeMultiplier();
		}

		return originalCooldown;
	}
}

[BaseContainerProps()]
class SCR_CooldownData
{
	[Attribute("1", desc: "Affected rank", uiwidget: UIWidgets.SearchComboBox, enums: ParamEnumArray.FromEnum(SCR_ECharacterRank))]
	protected SCR_ECharacterRank m_eRank;

	[Attribute("0", desc: "Default cooldown time set in SCR_EditableEntityComponent is multiplied by this value")]
	protected float m_fDefaultTimeMultiplier;

	//------------------------------------------------------------------------------------------------
	//! Get a rank
	SCR_ECharacterRank GetRank()
	{
		return m_eRank;
	}

	//------------------------------------------------------------------------------------------------
	//!Get a multiplier value
	ESpawnerVariantType GetTimeMultiplier()
	{
		return m_fDefaultTimeMultiplier;
	}

}
