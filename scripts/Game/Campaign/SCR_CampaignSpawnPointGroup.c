[EntityEditorProps(category: "GameScripted/GameMode", description: "Campaign Spawn point group entity", visible: false)]
class SCR_CampaignSpawnPointGroupClass : SCR_SpawnPointClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignSpawnPointGroup : SCR_SpawnPoint
{
	//------------------------------------------------------------------------------------------------
	override string GetSpawnPointName()
	{
		IEntity parent = GetParent();
		if (parent)
		{
			SCR_CampaignMilitaryBaseComponent base = SCR_CampaignMilitaryBaseComponent.Cast(parent.FindComponent(SCR_CampaignMilitaryBaseComponent));
			if (base)
				return base.GetBaseName();
		}

		return super.GetSpawnPointName();
	}
}
