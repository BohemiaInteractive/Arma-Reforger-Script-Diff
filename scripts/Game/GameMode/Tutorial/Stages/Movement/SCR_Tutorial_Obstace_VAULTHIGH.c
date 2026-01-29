[EntityEditorProps(insertable: false)]
class SCR_Tutorial_Obstacle_VAULTHIGHClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_Obstacle_VAULTHIGH : SCR_BaseTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		RegisterWaypoint("WP_VAULT2", "", "JUMP").SetOffsetVector("0 0.2 0");
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		return m_TutorialComponent.CheckCharacterStance(ECharacterCommandIDs.CLIMB);
	}
};