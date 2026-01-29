[EntityEditorProps(insertable: false)]
class SCR_Tutorial_Obstacle_LADDEROFFClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_Obstacle_LADDEROFF : SCR_BaseTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		RegisterWaypoint("WP_LADDER_OFF", "", "LADDER");
		
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		return !m_TutorialComponent.CheckCharacterStance(ECharacterCommandIDs.LADDER) && vector.DistanceSq(m_Player.GetOrigin(), GetWaypoint().GetPosition()) < 2;
	}
};