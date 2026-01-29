[BaseContainerProps(), SCR_ContainerAIActionTitle()]
class SCR_ScenarioFrameworkAIAddSetting : SCR_ScenarioFrameworkAIAction
{
	[Attribute()]
	protected ref SCR_AISettingBase m_Setting;
	
	[Attribute("1", UIWidgets.CheckBox, desc: "If true, it will remove all settings of same categorization type, and if they were provided by Scenario Framework.")]
	protected bool m_bRemoveSame;
	
	//------------------------------------------------------------------------------------------------
	void SCR_ScenarioFrameworkAIAddSetting()
	{
		// Init the AI setting object. We must provide it the Origin value.
		if (m_Setting)
			m_Setting.Internal_ConstructedAtProperty(SCR_EAISettingOrigin.SCENARIO, SCR_EAISettingFlags.SCENARIO_FRAMEWORK);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnActivate()
	{
		super.OnActivate();
		
		if (!m_Setting)
			return;
		
		SCR_AIGroupSettingsComponent settingsComp = SCR_AIGroupSettingsComponent.Cast(m_AIGroup.FindComponent(SCR_AIGroupSettingsComponent));
		
		if (!settingsComp)
		{
			Print(string.Format("ScenarioFramework Action: SCR_AIGroupSettingsComponent not found for Action %1.", this), LogLevel.ERROR);
			return;
		}
		
		settingsComp.AddSetting(m_Setting, true, m_bRemoveSame);
	}
}