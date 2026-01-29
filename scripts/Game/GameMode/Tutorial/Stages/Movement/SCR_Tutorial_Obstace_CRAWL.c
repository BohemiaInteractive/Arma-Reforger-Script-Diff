[EntityEditorProps(insertable: false)]
class SCR_Tutorial_Obstacle_CRAWLClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_Obstacle_CRAWL : SCR_BaseTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		RegisterWaypoint("WP_WIREMESH").SetOffsetVector("0 0.2 0");
		PlayNarrativeCharacterStage("OBSTACLECOURSE_Instructor", 6);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		CharacterControllerComponent comp = CharacterControllerComponent.Cast(m_Player.FindComponent(CharacterControllerComponent));
					
		if (comp)
			return comp.GetStance() == ECharacterStance.PRONE;
		else
			return true;
	}
};