[BaseContainerProps(configRoot: true), BaseContainerCustomTitleField("m_sLoadoutName")]
class SCR_FactionPlayerLoadout : SCR_PlayerLoadout
{
	[Attribute("", UIWidgets.EditBox, "")]
	string m_sAffiliatedFaction;
	
	//------------------------------------------------------------------------------------------------
	//! \return FactionKey for the faction affiliated to this loadout.
	FactionKey GetFactionKey()
	{
		return m_sAffiliatedFaction;
	}
	
	//------------------------------------------------------------------------------------------------
	ResourceName GetDefaultLoadoutResource()
	{
		return m_sLoadoutResource;
	}

	//------------------------------------------------------------------------------------------------
	override void OnLoadoutSpawned(GenericEntity pOwner, int playerId)
	{
		if (pOwner)
		{
			FactionAffiliationComponent comp = FactionAffiliationComponent.Cast(pOwner.FindComponent(FactionAffiliationComponent));
			if (comp)
				comp.SetAffiliatedFactionByKey(m_sAffiliatedFaction);
		}
	}
}
