// Script File
[EntityEditorProps(category: "GameScripted/Gamemodes")]
class SCR_AISpawnerGroupFactionClass: AISpawnerGroupClass
{

};

class SCR_AISpawnerGroupFaction: AISpawnerGroup
{
	[Attribute("", UIWidgets.EditBox, "Faction")]
	FactionKey m_faction;
	
	override void OnSpawn(IEntity spawned)
	{		
		GenericEntity ge = GenericEntity.Cast(spawned);
		if (ge)
		{
			FactionAffiliationComponent comp = FactionAffiliationComponent.Cast(ge.FindComponent(FactionAffiliationComponent));
			if (comp)
			{
				comp.SetAffiliatedFactionByKey(m_faction);
			}
		}
	}	
};