[ComponentEditorProps(category: "GameScripted/Commander")]
class SCR_FactionCommanderBaseLogicComponentClass : SCR_MilitaryBaseLogicComponentClass
{
}

class SCR_FactionCommanderBaseLogicComponent : SCR_MilitaryBaseLogicComponent
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
