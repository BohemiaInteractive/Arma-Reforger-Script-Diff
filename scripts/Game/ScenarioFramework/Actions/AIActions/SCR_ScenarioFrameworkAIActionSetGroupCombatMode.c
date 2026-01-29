[BaseContainerProps(), SCR_ContainerAIActionTitle()]
class SCR_ScenarioFrameworkAIActionSetGroupCombatMode : SCR_ScenarioFrameworkAIAction
{
	[Attribute(typename.EnumToString(EAIGroupCombatMode, EAIGroupCombatMode.FIRE_AT_WILL), UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(EAIGroupCombatMode))]
	protected EAIGroupCombatMode m_eCombatMode;
	
	//------------------------------------------------------------------------------------------------
	override void OnActivate()
	{
		super.OnActivate();
		
		SCR_AIGroupUtilityComponent utility = SCR_AIGroupUtilityComponent.Cast(m_AIGroup.FindComponent(SCR_AIGroupUtilityComponent));
		
		if (!utility)
			return;
		
		utility.SetCombatMode(m_eCombatMode);
	}
}