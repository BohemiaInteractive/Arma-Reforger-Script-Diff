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
			if (playerFaction.GetRankName(m_eRequiredRank).IsEmpty())
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
		//~ Make sure to have a fallback for factions without a rank. Both for the overlay icon as well as this hint
		
		IEntity localPlayerEntity = SCR_PlayerController.GetLocalControlledEntity();
		if (!localPlayerEntity)
		{
			Print("'SCR_ArsenalItemRankHintUIInfo' is unable to find the local player.", LogLevel.WARNING);
			return m_sRankLessFactionFallback;
		}
			
		FactionAffiliationComponent playerFactionAffiliation = FactionAffiliationComponent.Cast(localPlayerEntity.FindComponent(FactionAffiliationComponent));
		if (!playerFactionAffiliation)
		{
			Print("'SCR_ArsenalItemRankHintUIInfo' is unable to find the player's faction affiliation comp.", LogLevel.WARNING);
			return m_sRankLessFactionFallback;
		}
		
		SCR_Faction playerFaction = SCR_Faction.Cast(playerFactionAffiliation.GetAffiliatedFaction());
		if (!playerFaction)
		{
			Print("'SCR_ArsenalItemRankHintUIInfo' is unable to find the player's faction.", LogLevel.WARNING);
			return m_sRankLessFactionFallback;
		}
		
		string rankName = playerFaction.GetRankName(m_eRequiredRank);
		
		if (rankName.IsEmpty())
			return m_sRankLessFactionFallback;
		
		return WidgetManager.Translate(GetName(), rankName);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool SetIconTo(ImageWidget imageWidget)
	{
		if (!imageWidget)
			return false;
		
		IEntity localPlayerEntity = SCR_PlayerController.GetLocalControlledEntity();
		if (!localPlayerEntity)
			return false;
		
		ResourceName rankIconImageSet = SCR_XPInfoDisplay.GetRankIconImageSet();
		if (rankIconImageSet.IsEmpty())
			return false;
		
		FactionAffiliationComponent playerFactionAffiliation = FactionAffiliationComponent.Cast(localPlayerEntity.FindComponent(FactionAffiliationComponent));
		if (!playerFactionAffiliation)
			return false;
		
		SCR_Faction playerFaction = SCR_Faction.Cast(playerFactionAffiliation.GetAffiliatedFaction());
		if (!playerFaction)
			return false;
		
		string rankInsignia = playerFaction.GetRankInsignia(m_eRequiredRank);
		if (rankInsignia.IsEmpty())
		{
			imageWidget.LoadImageTexture(0, m_sRankLessFactionIconFallback);
			return true;
		}
			
		imageWidget.LoadImageFromSet(0, rankIconImageSet, rankInsignia);
		
		return true;
	}
}