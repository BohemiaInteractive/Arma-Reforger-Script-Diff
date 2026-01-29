[BaseContainerProps(), SCR_ContainerMedicalActionTitle()]
class SCR_ScenarioFrameworkMedicalAction
{
	[Attribute(desc: "If set to true, when this action gets activated, it will break the breakpoint in the Script Editor in CanActivate method from which you can step out to the OnActivate method and debug this specific action. This can be also set during runtime via Debug Menu > ScenarioFramework > Action Inspector")]
	bool m_bDebug;
	
	SCR_CharacterDamageManagerComponent m_DamageManager;

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] character
	void Init(SCR_ChimeraCharacter character)
	{
		// Here you can step out to the OnActivate method and debug this specific action if m_bDebug attribute is set to true.
		// This can be also adjusted during runtime via Debug Menu > ScenarioFramework > Action Inspector
		if (m_bDebug)
			Print("[SCR_ScenarioFrameworkMedicalAction.Init] debug line (" + __FILE__ + " L" + __LINE__ + ")", LogLevel.WARNING);
		
		m_DamageManager = SCR_CharacterDamageManagerComponent.Cast(character.GetDamageManager());
		if (!m_DamageManager)
		{
			Print(string.Format("ScenarioFramework Action: Character Damage Manager Component not found for Action %1.", this), LogLevel.ERROR);
			return;
		}

		OnActivate();
	}

	//------------------------------------------------------------------------------------------------
	void OnActivate()
	{
		// Here you can step out to the OnActivate method and debug this specific action if m_bDebug attribute is set to true.
		// This can be also adjusted during runtime via Debug Menu > ScenarioFramework > Action Inspector
		if (m_bDebug)
			Print("[SCR_ScenarioFrameworkMedicalAction.OnActivate] debug line (" + __FILE__ + " L" + __LINE__ + ")", LogLevel.WARNING);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[out] subActions
	ref array<ref SCR_ScenarioFrameworkActionBase> GetSubActions();
}