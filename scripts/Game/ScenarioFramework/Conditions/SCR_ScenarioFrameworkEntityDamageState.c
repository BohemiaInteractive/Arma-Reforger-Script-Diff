[BaseContainerProps()]
class SCR_ScenarioFrameworEntitykDamageStateCondition : SCR_ScenarioFrameworkActivationConditionBase
{
	[Attribute(desc: "Entity to check.")]
	ref SCR_ScenarioFrameworkGet m_Getter;

	[Attribute(uiwidget: UIWidgets.SearchComboBox, enums: ParamEnumArray.FromEnum(EDamageState), desc: "Damage state to check. Returns true when the entity state match. If entity doesn't have damage manager, returns true. If array of entities is passed on, return false when at least one entity doesn't fit selected damage state.")]
	protected EDamageState m_eDamageState;

	//------------------------------------------------------------------------------------------------
	override bool Init(IEntity entity)
	{
		// Here you can debug specific Condition instance.
		// This can be also adjusted during runtime via Debug Menu > ScenarioFramework > Condition Inspector
		if (m_bDebug)
			Print("[SCR_ScenarioFrameworEntitykDamageStateCondition.Init] debug line (" + __FILE__ + " L" + __LINE__ + ")", LogLevel.WARNING);
		
		if (!m_Getter)
			return false;

		SCR_ScenarioFrameworkParam<IEntity> entityWrapper = SCR_ScenarioFrameworkParam<IEntity>.Cast(m_Getter.Get());
		if (!entityWrapper)
		{
			SCR_ScenarioFrameworkParam<array<IEntity>> entityArrayWrapper = SCR_ScenarioFrameworkParam<array<IEntity>>.Cast(m_Getter.Get());
			if (!entityArrayWrapper)
			{
				Print(string.Format("ScenarioFramework Condition: Issue with Getter detected for condition %1.", this), LogLevel.ERROR);
				return false;
			}

			array<IEntity> entities = entityArrayWrapper.GetValue();

			if (entities.IsEmpty())
			{
				Print(string.Format("ScenarioFramework Condition: Array Getter for %1 has no elements to work with.", this), LogLevel.ERROR);
				return false;
			}

			foreach (IEntity ent : entities)
			{
				if (!EvaluateEntityDamageState(ent))
					return false;
			}

			return true;
		}

		IEntity WrappedEntity = entityWrapper.GetValue();
		if (!WrappedEntity)
		{
			Print(string.Format("ScenarioFramework Condition: Entity not found for condition %1.", this), LogLevel.ERROR);
			return false;
		}

		return EvaluateEntityDamageState(WrappedEntity);

	}

	//------------------------------------------------------------------------------------------------
	bool EvaluateEntityDamageState(notnull IEntity ent)
	{
		SCR_DamageManagerComponent damageManager = SCR_DamageManagerComponent.GetDamageManager(ent);
		if (damageManager)
			return damageManager.GetState() == m_eDamageState;
		else
			return true;
	}
}
