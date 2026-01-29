[EntityEditorProps(insertable: false)]
class SCR_Tutorial_SquadLeadership_RequestGroupClass : SCR_BaseTutorialStageClass
{
}

class SCR_Tutorial_SquadLeadership_RequestGroup : SCR_BaseTutorialStage
{
	protected string m_sSpawnedEntityName = "REQUESTING_GROUP";
	
	//------------------------------------------------------------------------------------------------
	override bool GetIsFinished()
	{
		return GetGame().GetWorld().FindEntityByName(m_sSpawnedEntityName);
	}
}