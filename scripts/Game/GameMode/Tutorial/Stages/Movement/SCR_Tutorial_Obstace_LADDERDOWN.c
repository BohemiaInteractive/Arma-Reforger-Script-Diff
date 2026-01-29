[EntityEditorProps(insertable: false)]
class SCR_Tutorial_Obstacle_LADDERDOWNClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_Obstacle_LADDERDOWN : SCR_BaseTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		RegisterWaypoint("WP_LADDER_DOWN");
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
		
		PlayNarrativeCharacterStage("OBSTACLECOURSE_Instructor", 14);
		
		if (GetGame().GetInputManager().IsUsingMouseAndKeyboard())
			ShowHint();
		
		GetGame().OnInputDeviceIsGamepadInvoker().Insert(OnInputChanged);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnInputChanged(bool isGamepad)
	{
		if (isGamepad)
		{
			SCR_HintManagerComponent.HideHint();
			SCR_HintManagerComponent.ClearLatestHint();
			return;
		}
		
		ShowHint();
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		return m_TutorialComponent.CheckCharacterStance(ECharacterCommandIDs.LADDER);
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_Tutorial_Obstacle_LADDERDOWN()
	{
		GetGame().OnInputDeviceIsGamepadInvoker().Remove(OnInputChanged);
	}
};