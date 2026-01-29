//! Forbids AI to 'touch' lights.
//! It doesn't force them to set lights to a specific value!
//! It's meant for cases when we don't want AI to touch his lights, so we can modify lights ourselves.
[BaseContainerProps(visible: false)]
class SCR_AICharacterLightInteractionSettingBase : SCR_AICharacterSettingWithCause
{
	override typename GetCategorizationType()
	{
		return SCR_AICharacterLightInteractionSettingBase;
	}
	
	bool IsLightInterractionAllowed();
}

[BaseContainerProps(description: "Allows AI to 'touch' lights. It's not meant to switch lights On/Off! Use it if you want to toggle lights yourself.")]
class SCR_AICharacterLightInteractionSetting : SCR_AICharacterLightInteractionSettingBase
{
	[Attribute("0", UIWidgets.CheckBox, desc: "Allows AI to touch lights")]
	protected bool m_bAllowInteraction;
	
	override bool IsLightInterractionAllowed()
	{
		return m_bAllowInteraction;
	}
	
	static SCR_AICharacterLightInteractionSetting Create(SCR_EAISettingOrigin origin, SCR_EAIBehaviorCause cause, bool allowInteraction)
	{
		auto s = new SCR_AICharacterLightInteractionSetting();
		s.Init(origin, cause);
		s.m_bAllowInteraction = allowInteraction;
		return s;
	}
	
	override string GetDebugText()
	{
		return string.Format("Allowed: %1", m_bAllowInteraction);
	}
}