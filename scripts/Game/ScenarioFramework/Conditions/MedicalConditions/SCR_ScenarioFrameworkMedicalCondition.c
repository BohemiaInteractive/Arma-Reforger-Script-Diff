[BaseContainerProps()]
class SCR_ScenarioFrameworkMedicalCondition : SCR_ScenarioFrameworkActivationConditionBase
{
	[Attribute(desc: "Entity to check.")]
	ref SCR_ScenarioFrameworkGet m_Getter;

	[Attribute(desc: "Character must have all of these medical conditions.")]
	ref array<ref SCR_ScenarioFrameworkMedicalConditionBase> m_aRequiredMedicalConditions;
	
	//------------------------------------------------------------------------------------------------
	override bool Init(IEntity entity)
	{
		// Here you can debug specific Condition instance.
		// This can be also adjusted during runtime via Debug Menu > ScenarioFramework > Condition Inspector
		if (m_bDebug)
			Print("[SCR_ScenarioFrameworkMedicalCondition.Init] Condition invoked with debug flag (" + __FILE__ + " L" + __LINE__ + ")", LogLevel.WARNING);
		
		if (!m_Getter)
			return false;

		SCR_ScenarioFrameworkParam<IEntity> entityWrapper = SCR_ScenarioFrameworkParam<IEntity>.Cast(m_Getter.Get());
		if (!entityWrapper)
		{
			SCR_ScenarioFrameworkParam<array<IEntity>> entityArrayWrapper = SCR_ScenarioFrameworkParam<array<IEntity>>.Cast(m_Getter.Get());
			if (!entityArrayWrapper)
			{
				PrintFormat("ScenarioFramework Condition: Issue with Getter detected for condition %1.", this, level: LogLevel.ERROR);
				return false;
			}

			array<IEntity> entities = entityArrayWrapper.GetValue();

			if (entities.IsEmpty())
			{
				PrintFormat("ScenarioFramework Condition: Array Getter for %1 has no elements to work with.", this, level: LogLevel.ERROR);
				return false;
			}

			foreach (IEntity ent : entities)
			{
				if (!EvaluateMedicalConditions(ent))
					return false;
			}

			return true;
		}

		IEntity wrappedEntity = entityWrapper.GetValue();
		if (!wrappedEntity)
		{
			Print(string.Format("ScenarioFramework Condition: Entity not found for condition %1.", this), LogLevel.ERROR);
			return false;
		}

		return EvaluateMedicalConditions(wrappedEntity);
	}

	//------------------------------------------------------------------------------------------------
	bool EvaluateMedicalConditions(notnull IEntity ent)
	{
		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(ent);
		if (!character)
			return true;
		
		SCR_CharacterDamageManagerComponent damageManager = SCR_CharacterDamageManagerComponent.Cast(character.GetDamageManager());
		if (!damageManager)
			return true;
		
		foreach (SCR_ScenarioFrameworkMedicalConditionBase requiredCondition : m_aRequiredMedicalConditions)
		{
			if (!requiredCondition.Init(character, damageManager))
				return false;
		}
		return true;	
	}
}