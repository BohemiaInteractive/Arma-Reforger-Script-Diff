class SCR_AntennaServicePointComponentClass : SCR_ServicePointComponentClass
{
}

class SCR_AntennaServicePointComponent : SCR_ServicePointComponent
{
	protected BaseRadioComponent m_RadioControl;
	
	//------------------------------------------------------------------------------------------------
	override void OnFactionChanged(FactionAffiliationComponent owner, Faction previousFaction, Faction faction)
	{
		super.OnFactionChanged(faction);
		
		SCR_Faction newFaction = SCR_Faction.Cast(faction);
		
		if (!newFaction)
			return;
		
		ChangeRadioSettings(newFaction);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] faction
	void ChangeRadioSettings(notnull SCR_Faction faction)
	{
		if (!m_RadioControl || m_RadioControl.TransceiversCount() == 0)
			return;
		
		BaseTransceiver tsv = m_RadioControl.GetTransceiver(0);
		if (!tsv)
			return;
		
		m_RadioControl.SetEncryptionKey(faction.GetFactionRadioEncryptionKey());
		
		int factionFrequency = faction.GetFactionRadioFrequency();
		
		// Setting frequency outside of limits causes a VME
		if (factionFrequency < tsv.GetMinFrequency() || factionFrequency > tsv.GetMaxFrequency())
			return;
		
		tsv.SetFrequency(factionFrequency);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		
		m_RadioControl = BaseRadioComponent.Cast(owner.FindComponent(BaseRadioComponent));
		m_RadioControl.SetPower(true);

		if (!m_RadioControl)
			Print("SCR_AntennaServicePointComponent: Owner is missing BaseRadioComponent!", LogLevel.ERROR);
	}
}
