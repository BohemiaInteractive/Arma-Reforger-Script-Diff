[EntityEditorProps(insertable: false)]
class SCR_Tutorial_Obstacle_WALLONEClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_Obstacle_WALLONE : SCR_BaseTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		RegisterWaypoint("WP_WALL", "", "JUMP").SetOffsetVector("0 0.3 0");
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
		
		if (!m_TutorialComponent.GetWas3rdPersonViewUsed())
			GetGame().GetCallqueue().CallLater(DelayedPopup, 2000, false, "#AR-Tutorial_Popup_Title-UC", "#AR-Tutorial_Popup_Camera", 7, "", "", "<color rgba='226,168,79,200'><shadow mode='image' color='0,0,0' size='1' offset='1,1' opacity = '0.5'><action name = 'SwitchCameraType'/></shadow></color>", "");
		
		PlayNarrativeCharacterStage("OBSTACLECOURSE_Instructor", 10);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		return m_TutorialComponent.CheckCharacterStance(ECharacterCommandIDs.CLIMB);
	}
};