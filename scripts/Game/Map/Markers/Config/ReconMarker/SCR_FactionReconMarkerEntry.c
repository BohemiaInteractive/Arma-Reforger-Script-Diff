[BaseContainerProps()]
class SCR_FactionReconMarkerEntry
{
	[Attribute(desc: "Faction, if faction is not set, it is used for all factions")]
	protected FactionKey m_sFaction;

	[Attribute(desc: "Recon marker entries, for setting the recon markers")]
	protected ref array<ref SCR_BaseReconMarkerEntry> m_aReconMarkerEntries;

	//------------------------------------------------------------------------------------------------
	//! \return faction key
	FactionKey GetFactionKey()
	{
		return m_sFaction;
	}

	//------------------------------------------------------------------------------------------------
	array<ref SCR_BaseReconMarkerEntry> GetReconMarkerEntries()
	{
		return m_aReconMarkerEntries;
	}
}
