[BaseContainerProps(configRoot: true), BaseContainerCustomCheckIntTitleField("m_bEnabled", "Valuable Intel Effects", "(Disabled) Valuable Intel Effects", 1)]
class SCR_ValuableIntelArsenalRefundEffect : SCR_WeightedListArsenalRefundEffect
{
	//------------------------------------------------------------------------------------------------
	override bool IsValid(notnull IEntity item, notnull PlayerController playerController, SCR_ArsenalComponent arsenal)
	{
		if (!super.IsValid(item, playerController, arsenal))
			return false;
		
		SCR_IdentityInventoryItemComponent identityItem = SCR_IdentityInventoryItemComponent.Cast(item.FindComponent(SCR_IdentityInventoryItemComponent));

		//~ Check if the item has identity item, was handed in at a friendly arsenal and if the item has intel for the player
		return identityItem && UsedFriendlyArsenal(item, playerController, arsenal) && identityItem.HasValuableIntel(true, playerController);
	}
}

[BaseContainerProps(), BaseContainerCustomCheckIntTitleField("m_bEnabled", "Valuable Intel - Add XP", "(Disabled) Valuable Intel - Add XP", 1)]
class  SCR_ValuableIntelXPEffectEntry : SCR_BaseWeightedArsenalRefundEffectEntry
{
	[Attribute(SCR_EXPRewards.VALUABLE_INTEL_HANDIN_SMALL.ToString(), desc: "The XP reward given", uiwidget: UIWidgets.SearchComboBox, enums: ParamEnumArray.FromEnum(SCR_EXPRewards))]
	protected SCR_EXPRewards m_eXPReward;
	
	//------------------------------------------------------------------------------------------------
	override bool ExecuteRefundEffect(notnull IEntity item, notnull PlayerController playerController, SCR_ArsenalComponent arsenal)
	{
		if (playerController.GetPlayerId() <= 0)
			return false;
		
		BaseGameMode gameMode = GetGame().GetGameMode();
		if (!gameMode)
			return false;
		
		SCR_XPHandlerComponent compXP = SCR_XPHandlerComponent.Cast(gameMode.FindComponent(SCR_XPHandlerComponent));
		if (!compXP)
			return false;
		
		compXP.AwardXP(playerController.GetPlayerId(), m_eXPReward);
		
		SCR_IdentityManagerComponent identityManager = SCR_IdentityManagerComponent.GetInstance();
		if (identityManager)
			identityManager.OnValuableIntelHandIn_S(playerController);
		
		return true;
	}
}
