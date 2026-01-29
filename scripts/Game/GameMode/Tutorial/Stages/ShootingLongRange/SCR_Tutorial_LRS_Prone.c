[EntityEditorProps(insertable: false)]
class SCR_Tutorial_LRS_ProneClass: SCR_BaseTutorialStageClass
{
}

class SCR_Tutorial_LRS_Prone : SCR_BaseTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override void Setup()
	{
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
		
		PlayNarrativeCharacterStage("LONGRANGESHOOTING_Instructor", 4);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnVoiceoverEventFinished(string eventName)
	{
		if (eventName == "SOUND_TUTORIAL_TA_SCOPE_PICKED_UP_INSTRUCTOR_K_04")
			RegisterWaypoint("LRS_mat");
	}
	
	//------------------------------------------------------------------------------------------------
	override bool GetIsFinished()
	{
		CharacterControllerComponent comp = CharacterControllerComponent.Cast(m_Player.FindComponent(CharacterControllerComponent));
					
		if (comp)
			return comp.GetStance() == ECharacterStance.PRONE;
		else
			return true;
	}
}