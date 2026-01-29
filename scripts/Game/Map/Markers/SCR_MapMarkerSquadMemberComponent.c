class SCR_MapMarkerSquadMemberComponent : SCR_MapMarkerDynamicWComponent
{
	//------------------------------------------------------------------------------------------------
	//! \param[in] faction
	void SetFaction(Faction faction)
	{
		SetColor(faction.GetFactionColor());
	}
}
