//! Signup component for vehicles. Volunteering is allowed when the vehicle is within
//! the friendly faction's radio signal, regardless of military base ownership.
[ComponentEditorProps(category: "GameScripted/Commander", description: "Faction commander signup component for command vehicles. Gates volunteering on faction radio signal range instead of military base presence.")]
class SCR_VehicleFactionCommanderSignupComponentClass : SCR_FactionCommanderSignupComponentClass
{
}

class SCR_VehicleFactionCommanderSignupComponent : SCR_FactionCommanderSignupComponent
{
	//------------------------------------------------------------------------------------------------
	override bool IsControlledByFaction(notnull Faction faction)
	{
		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
		if (!campaign)
			return false;

		SCR_CampaignMilitaryBaseManager baseManager = campaign.GetBaseManager();
		if (!baseManager)
			return false;

		IEntity owner = GetOwner();
		if (!owner)
			return false;

		return baseManager.IsEntityInFactionRadioSignal(owner, faction);
	}
}
