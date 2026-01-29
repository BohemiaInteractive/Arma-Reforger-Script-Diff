// Script File//------------------------------------------------------------------------------------------------
class SCR_AIDecoTimeSinceTargetDetected : DecoratorScripted
{
	static const string BASE_TARGET_PORT = "BaseTargetIn";
	
	[Attribute("0.5", UIWidgets.EditBox, "Threshold time since target was detected", "")]
	float m_TimeThreshold_S;
	
	PerceptionComponent m_PerceptionComp;
	
	//-----------------------------------------------------------------------------------------------------
	protected override bool TestFunction(AIAgent owner)
	{
		if (!m_PerceptionComp)
			m_PerceptionComp = PerceptionComponent.Cast(owner.GetControlledEntity().FindComponent(PerceptionComponent));
		
		BaseTarget target;
		GetVariableIn(BASE_TARGET_PORT,target);
		
		if (!target || !m_PerceptionComp)
		{
			return false;
		}
		
		// When m_TimeThreshold_S value is lower than perception component update interval,
		// which might be large at higher LOD levels, the decorator might periodically return false,
		// even when target is still visible.
		// Therefore we must prevent the threshold from being smaller than update interval of perception component.
		float timeMax = Math.Max(m_TimeThreshold_S, m_PerceptionComp.GetUpdateInterval()) + 0.02;
		
		return target.GetTimeSinceDetected() < timeMax;
	}
	
	//-----------------------------------------------------------------------------------------------------
	protected static override bool VisibleInPalette()
	{
		return true;
	}	
	
	//-----------------------------------------------------------------------------------------------------
	protected static override string GetOnHoverDescription()
	{
		return "Checks time since target was detected and compares it with threshold. Returns true when time is below the threshold.";
	}
	
	//-----------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsIn = {
		BASE_TARGET_PORT
	};
	protected override TStringArray GetVariablesIn()
	{
		return s_aVarsIn;
	}
};
