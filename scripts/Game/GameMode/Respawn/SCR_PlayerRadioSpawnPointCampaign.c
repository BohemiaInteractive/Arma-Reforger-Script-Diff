[EntityEditorProps(category: "GameScripted/GameMode", description: "")]
class SCR_PlayerRadioSpawnPointCampaignClass : SCR_PlayerRadioSpawnPointClass
{
}

class SCR_PlayerRadioSpawnPointCampaign : SCR_PlayerRadioSpawnPoint
{
	//------------------------------------------------------------------------------------------------
	override void OnItemAdded(IEntity item,BaseInventoryStorageComponent storageOwner)
	{
		if (!storageOwner)
			return;
		
		if (SCR_CharacterRankComponent.GetCharacterRank(storageOwner.GetOwner()) == SCR_ECharacterRank.RENEGADE)
			return;
		
		super.OnItemAdded(item, storageOwner);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	Faction GetCachedFaction()
	{
		return m_CachedFaction;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	void ActivateSpawnPointPublic()
	{
		super.ActivateSpawnPoint();
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	void DeactivateSpawnPointPublic()
	{
		super.DeactivateSpawnPoint();
	}
}
