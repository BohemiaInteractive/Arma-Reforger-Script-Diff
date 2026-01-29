class SCR_CampaignRadioRegisteringComponentClass : SCR_MilitaryBaseLogicComponentClass
{
}

class SCR_CampaignRadioRegisteringComponent : SCR_MilitaryBaseLogicComponent
{
	protected static const string RADIO_CHATTER_SIGNAL_NAME = "RadioChatter";
	protected static const string ESTABLISH_ACTION_SIGNAL_NAME = "EstablishAction";

	//------------------------------------------------------------------------------------------------
	override void OnCapturingFactionChanged(FactionKey faction)
	{
		super.OnCapturingFactionChanged(faction);

		// SFX not needed for headless
		if (System.IsConsoleApp())
			return;

		// Play or stop radio tuning SFX
		SignalsManagerComponent comp = SignalsManagerComponent.Cast(GetOwner().FindComponent(SignalsManagerComponent));

		if (!comp)
			return;

		if (faction.IsEmpty())
			comp.SetSignalValue(comp.AddOrFindSignal(ESTABLISH_ACTION_SIGNAL_NAME), 0);
		else
			comp.SetSignalValue(comp.AddOrFindSignal(ESTABLISH_ACTION_SIGNAL_NAME), 1);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] faction
	override void OnBaseFactionChanged(Faction faction)
	{
		super.OnBaseFactionChanged(faction);

		SetRadioChatterSignal(faction);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] base
	override void OnBaseRegistered(notnull SCR_MilitaryBaseComponent base)
	{
		super.OnBaseRegistered(base);

		SCR_CampaignMilitaryBaseComponent campaignBase = SCR_CampaignMilitaryBaseComponent.Cast(base);

		if (!campaignBase)
			return;

		SetRadioChatterSignal(base.GetFaction());
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] faction
	void SetRadioChatterSignal(Faction faction)
	{
		// SFX not needed for headless
		if (System.IsConsoleApp())
			return;

		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();

		if (!campaign)
			return;

		SignalsManagerComponent comp = SignalsManagerComponent.Cast(GetOwner().FindComponent(SignalsManagerComponent));

		if (!comp)
			return;

		if (!faction || faction.GetFactionKey() == campaign.GetFactionKeyByEnum(SCR_ECampaignFaction.INDFOR))
		{
			comp.SetSignalValue(comp.AddOrFindSignal(RADIO_CHATTER_SIGNAL_NAME), 0);
		}
		else
		{
			if (faction.GetFactionKey() == campaign.GetFactionKeyByEnum(SCR_ECampaignFaction.BLUFOR))
				comp.SetSignalValue(comp.AddOrFindSignal(RADIO_CHATTER_SIGNAL_NAME), 1);
			else
				comp.SetSignalValue(comp.AddOrFindSignal(RADIO_CHATTER_SIGNAL_NAME), 2);
		}
	}
}
