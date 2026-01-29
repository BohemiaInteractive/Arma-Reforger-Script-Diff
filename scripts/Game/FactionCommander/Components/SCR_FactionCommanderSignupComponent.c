//! Add this component to an entity at which players will volunteer for faction commander role via user action.
[ComponentEditorProps(category: "GameScripted/Commander", description: "Add this component to an entity at which players will volunteer for faction commander role via user action.")]
class SCR_FactionCommanderSignupComponentClass : SCR_MilitaryBaseLogicComponentClass
{
}

class SCR_FactionCommanderSignupComponent : SCR_MilitaryBaseLogicComponent
{
	//------------------------------------------------------------------------------------------------
	override bool IsControlledByFaction(notnull Faction faction)
	{
		foreach (SCR_MilitaryBaseComponent base : m_aBases)
		{
			if (!SCR_CampaignMilitaryBaseComponent.Cast(base))
				continue;

			if (base.GetFaction() == faction)
				return true;
		}

		return false;
	}
}
