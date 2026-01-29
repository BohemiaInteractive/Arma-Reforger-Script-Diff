[EntityEditorProps(insertable: false)]
class SCR_Tutorial_Seizing_Place_AntennaClass : SCR_BaseTutorialStageClass
{
}

class SCR_Tutorial_Seizing_Place_Antenna : SCR_BaseTutorialStage
{
	protected string m_sSpawnedEntityName = "BUILDING_ANTENNA";
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		return GetGame().GetWorld().FindEntityByName(m_sSpawnedEntityName);
	}
}