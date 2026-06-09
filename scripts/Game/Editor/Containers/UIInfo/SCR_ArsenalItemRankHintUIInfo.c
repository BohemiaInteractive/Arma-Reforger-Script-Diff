[BaseContainerProps(configRoot: true)]
class SCR_ArsenalItemRankHintUIInfo : SCR_InventoryItemHintUIInfo
{	
	[Attribute("#AR-Item_Hint_Arsenal_InsufficientRank")]
	protected LocalizedString m_sRankLessFactionFallback;
	
	[Attribute("{A4D40440E48CC7CC}UI/Textures/Editor/ContextMenu/ContextAction_BanPlayer.edds")]
	protected ResourceName m_sRankLessFactionIconFallback;
	
	protected SCR_ECharacterRank m_eRequiredRank;
	
	//------------------------------------------------------------------------------------------------
	override bool CanBeShown(InventoryItemComponent item, SCR_InventorySlotUI focusedSlot)
	{
		if (!super.CanBeShown(item, focusedSlot))
			return false;
		
		IEntity localPlayerEntity = SCR_PlayerController.GetLocalControlledEntity();
		if (!localPlayerEntity)
			return false;
		
		if (m_eRequiredRank <= SCR_CharacterRankComponent.GetCharacterRank(localPlayerEntity))
		{
			FactionAffiliationComponent playerFactionAffiliation = FactionAffiliationComponent.Cast(localPlayerEntity.FindComponent(FactionAffiliationComponent));
			if (!playerFactionAffiliation)
				return false;
			
			SCR_Faction playerFaction = SCR_Faction.Cast(playerFactionAffiliation.GetAffiliatedFaction());
			if (!playerFaction)
				return false;
			
			//~ Hide the hint if the faction has no rank equal to the required rank and the item is availible to be taken
			if (playerFaction.GetRanks().GetRankName(m_eRequiredRank).IsEmpty())
				return false;
		}
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetRequiredRank(SCR_ECharacterRank rank)
	{
		m_eRequiredRank = rank;
	}
	
	//------------------------------------------------------------------------------------------------
	override string GetItemHintName(InventoryItemComponent item)
	{
		SCR_Faction playerFaction = SCR_Faction.Cast(SCR_FactionManager.SGetLocalPlayerFaction());
		if (!playerFaction)
		{
			Print("'SCR_ArsenalItemRankHintUIInfo' is unable to find the player's faction.", LogLevel.WARNING);
			return m_sRankLessFactionFallback;
		}

		SCR_RankInfo rank = playerFaction.GetRanks().GetRankByID(m_eRequiredRank, true);
		if (!rank)
			return m_sRankLessFactionFallback;

		return WidgetManager.Translate(GetName(), rank.GetRankName());
	}
	
	//------------------------------------------------------------------------------------------------
	override bool SetIconTo(ImageWidget imageWidget)
	{
		if (!imageWidget)
			return false;

		ResourceName rankIconImageSet = SCR_XPInfoDisplay.GetRankIconImageSet();
		if (rankIconImageSet.IsEmpty())
			return false;

		SCR_Faction playerFaction = SCR_Faction.Cast(SCR_FactionManager.SGetLocalPlayerFaction());
		if (!playerFaction)
			return false;

		SCR_RankInfo rank = playerFaction.GetRanks().GetRankByID(m_eRequiredRank, true);
		if (!rank)
		{
			imageWidget.LoadImageTexture(0, m_sRankLessFactionIconFallback);
			return true;
		}

		imageWidget.LoadImageFromSet(0, rankIconImageSet, rank.GetRankInsignia());
		return true;
	}
}