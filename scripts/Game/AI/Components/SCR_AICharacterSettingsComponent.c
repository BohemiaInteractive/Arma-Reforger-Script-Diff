//---------------------------------------------------------------------------------------------------
[ComponentEditorProps(category: "GameScripted/AI", description: "Component for character AI settings")]
class SCR_AICharacterSettingsComponentClass : SCR_AISettingsBaseComponentClass
{
	
}

class SCR_AICharacterSettingsComponent : SCR_AISettingsBaseComponent
{	
	[Attribute("", UIWidgets.Auto, desc: "These settings will be added as SCR_EAISettingOrigin.DEFAULT")]
	protected ref array<ref SCR_AICharacterSetting> m_aDefaultSettings;
	
	//---------------------------------------------------------------------------------------------------
	//! Adds settings to character.
	//! This is meant only for character-related settings, those inheriting from SCR_AICharacterSetting.
	//! createCopy - when true, a copy of the setting object will be created. When false, this component takes ownership of passed setting object.
	//! If you want to add same setting to many agents, always add it with createCopy=true, or make a copy yourself.
	//! removeSameTypeAndOrigin - if true, removes all settings which have same categorization type and origin.
	bool AddCharacterSetting(notnull SCR_AICharacterSetting setting, bool createCopy, bool removeSameTypeAndOrigin = false)
	{
		return AddSetting(setting, createCopy, removeSameTypeAndOrigin);
	}
	
	//---------------------------------------------------------------------------------------------------
	//! It's overridden as protected. This way SCR_AICharacterSettingsComponent accepts only character settings.
	override protected bool AddSetting(notnull SCR_AISettingBase setting, bool createCopy, bool removeSameTypeAndOrigin = false)
	{
		return super.AddSetting(setting, createCopy, removeSameTypeAndOrigin);
	}
	
	//---------------------------------------------------------------------------------------------------
	//! Finds settings component of controlled entity (character)
	static SCR_AICharacterSettingsComponent FindOnControlledEntity(notnull IEntity entity)
	{
		AIControlComponent controlComp = AIControlComponent.Cast(entity.FindComponent(AIControlComponent));
		if (!controlComp)
			return null;
		
		AIAgent agent = controlComp.GetAIAgent();
		if (!agent)
			return null;
		
		return SCR_AICharacterSettingsComponent.Cast(agent.FindComponent(SCR_AICharacterSettingsComponent));
	}
	
	//---------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);

		// Add default settings
		foreach (auto s : m_aDefaultSettings)
		{
			s.Internal_ConstructedAtProperty(SCR_EAISettingOrigin.DEFAULT, SCR_EAISettingFlags.SETTINGS_COMPONENT);
			AddSetting(s, false, false);
		}
	}
}