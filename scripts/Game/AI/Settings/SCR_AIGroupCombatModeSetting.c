[BaseContainerProps(visible: false)]
class SCR_AIGroupCombatModeSettingBase : SCR_AIGroupSetting
{
	override typename GetCategorizationType()
	{
		return SCR_AIGroupCombatModeSettingBase;
	}
	
	//! combatMode - the combat mode wanted by AI
	//! now it is not used, since this setting propagates data downwards. It is here for future compatibility.
	EAIGroupCombatMode GetCombatMode(EAIGroupCombatMode combatMode);
	
	override void ApplyEffects(SCR_AISettingsBaseComponent settingsComp)
	{
		SCR_AIGroupUtilityComponent utility = SCR_AIGroupUtilityComponent.Cast(settingsComp.GetOwner().FindComponent(SCR_AIGroupUtilityComponent));
		if (utility)
		{
			EAIGroupCombatMode combatMode = GetCombatMode(0);
			utility.SetCombatMode(combatMode);
		}
	}
}

[BaseContainerProps()]
class SCR_AIGroupCombatModeSetting : SCR_AIGroupCombatModeSettingBase
{
	[Attribute(typename.EnumToString(EAIGroupCombatMode, EAIGroupCombatMode.FIRE_AT_WILL), UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(EAIGroupCombatMode))]
	protected EAIGroupCombatMode m_eCombatMode;
	
	static SCR_AIGroupCombatModeSetting Create(SCR_EAISettingOrigin origin, EAIGroupCombatMode combatMode)
	{
		SCR_AIGroupCombatModeSetting s = new SCR_AIGroupCombatModeSetting();
		s.Init(origin);
		s.m_eCombatMode = combatMode;
		return s;
	}
	
	override EAIGroupCombatMode GetCombatMode(EAIGroupCombatMode combatMode)
	{
		return m_eCombatMode;
	}
	
	override string GetDebugText()
	{
		return typename.EnumToString(EAIGroupCombatMode, m_eCombatMode);
	}
}