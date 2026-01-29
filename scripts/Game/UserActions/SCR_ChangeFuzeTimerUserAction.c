class SCR_ChangeFuzeTimerUserAction : ScriptedUserAction
{
	[Attribute("30", UIWidgets.Slider, "Value for the timer in seconds", "1 100 1")]
	protected float m_fNewTimer;
	
	protected TimerTriggerComponent m_TimerTriggerComp;
	
	//------------------------------------------------------------------------------------------------
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		m_TimerTriggerComp = TimerTriggerComponent.Cast(pOwnerEntity.FindComponent(TimerTriggerComponent));
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		if (!m_TimerTriggerComp)
			return false;
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		m_TimerTriggerComp.SetTimer(m_fNewTimer);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		outName = string.Format("Set timer to %1 seconds (Current: %2 seconds)", m_fNewTimer, m_TimerTriggerComp.GetTimer());
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool HasLocalEffectOnlyScript()
	{
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBroadcastScript()
	{
		return true;
	}
}
