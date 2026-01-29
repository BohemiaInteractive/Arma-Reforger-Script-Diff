[EntityEditorProps(insertable: false)]
class SCR_Tutorial_SW_SWITCH_FIREMODEClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_SW_SWITCH_FIREMODE : SCR_BaseTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
		
		EventHandlerManagerComponent eventHandlerManager = EventHandlerManagerComponent.Cast(m_Player.FindComponent(EventHandlerManagerComponent));
		if (!eventHandlerManager)
			return;
		
		eventHandlerManager.RegisterScriptHandler("OnMuzzleChanged", this, MuzzleChanged);
		
		PlayNarrativeCharacterStage("SPECIALWEAPONS_InstructorM", 5);
	}
	
	//------------------------------------------------------------------------------------------------
	void MuzzleChanged(BaseWeaponComponent weapon, BaseMuzzleComponent muzzle, BaseMuzzleComponent prevMuzzle)
	{
		if (muzzle && muzzle.GetMuzzleType() == EMuzzleType.MT_UGLMuzzle)
			m_bFinished = true;
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_Tutorial_SW_SWITCH_FIREMODE()
	{
		EventHandlerManagerComponent eventHandlerManager = EventHandlerManagerComponent.Cast(m_Player.FindComponent(EventHandlerManagerComponent));
		if (!eventHandlerManager)
			return;
		
		eventHandlerManager.RemoveScriptHandler("OnMuzzleChanged", this, MuzzleChanged);
	}
};