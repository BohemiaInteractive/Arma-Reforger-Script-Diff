// Danger reaction base class

[BaseContainerProps()]
class SCR_AIDangerReaction
{
	[Attribute("0", UIWidgets.ComboBox, "Type of event activating the reaction", "", ParamEnumArray.FromEnum(EAIDangerEventType) )]
	EAIDangerEventType m_eType;
	
	bool PerformReaction(notnull SCR_AIUtilityComponent utility, notnull SCR_AIThreatSystem threatSystem, AIDangerEvent dangerEvent, int dangerEventCount) {}
	
	#ifdef AI_DEBUG
	protected void AddDebugMessage(SCR_AIUtilityComponent utility, string str)
	{
		SCR_AIInfoBaseComponent infoComp = SCR_AIInfoBaseComponent.Cast(utility.GetOwner().FindComponent(SCR_AIInfoBaseComponent));
		infoComp.AddDebugMessage(string.Format("%1: %2", this, str), msgType: EAIDebugMsgType.REACTION);
	}
	#endif
};