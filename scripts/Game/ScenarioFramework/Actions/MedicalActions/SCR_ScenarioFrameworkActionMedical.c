[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionMedical : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Target entity for Medical Action")];
	ref SCR_ScenarioFrameworkGet m_Getter;

	[Attribute(desc: "Medical actions that will be executed on target entity")];
	ref array<ref SCR_ScenarioFrameworkMedicalAction> m_aMedicalActions;

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		if (!m_Getter)
		{
			Print(string.Format("ScenarioFramework Action: Getter not found for Action %1.", this), LogLevel.ERROR);
			return;
		}

		SCR_ScenarioFrameworkParam<IEntity> entityWrapper = SCR_ScenarioFrameworkParam<IEntity>.Cast(m_Getter.Get());
		if (!entityWrapper)
		{
			Print(string.Format("ScenarioFramework Action: Issue with Getter detected for Action %1.", this), LogLevel.ERROR);
			return;
		}

		IEntity entity = entityWrapper.GetValue();
		if (!entity)
		{
			Print(string.Format("ScenarioFramework Action: Entity not found for Action %1.", this), LogLevel.ERROR);
			return;
		}

		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(entity);
		if (!character)
		{
			Print(string.Format("ScenarioFramework Action: Entity is not ChimeraCharacter for Action %1.", this), LogLevel.ERROR);
			return;
		}

		foreach (SCR_ScenarioFrameworkMedicalAction medicalAction : m_aMedicalActions)
		{
			medicalAction.Init(character);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	array<ref SCR_ScenarioFrameworkMedicalAction> GetSubActionsMedical()
	{
		return m_aMedicalActions;
	}
}