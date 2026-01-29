[BaseContainerProps()]
class SCR_ScenarioFrameworkADSCondition : SCR_ScenarioFrameworkActivationConditionBase
{
	[Attribute(desc: "Entity to check")]
	ref SCR_ScenarioFrameworkGet m_Getter;

	[Attribute(desc: "Entity(s) must have this aim down sights state")]
	protected bool m_bMustBeADS;

	//------------------------------------------------------------------------------------------------
	override bool Init(IEntity entity)
	{
		// Here you can debug specific Condition instance.
		// This can be also adjusted during runtime via Debug Menu > ScenarioFramework > Condition Inspector
		if (m_bDebug)
			Print("[SCR_ScenarioFrameworkADSCondition.Init] debug line (" + __FILE__ + " L" + __LINE__ + ")", LogLevel.WARNING);

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
				if (!EvaluateEntityStance(ent))
					return false;
			}

			return true;
		}

		IEntity WrappedEntity = entityWrapper.GetValue();
		if (!WrappedEntity)
		{
			PrintFormat("ScenarioFramework Condition: Entity not found for condition %1.", this, level: LogLevel.ERROR);
			return false;
		}

		return EvaluateEntityStance(WrappedEntity);
	}

	//------------------------------------------------------------------------------------------------
	//! Evaluates if the entity has the required aim down sights state.
	//! \param[in] entitiy Entity to check. If entity has no SCR_CharacterControllerComponent, then true is returned. This allows the condition to still work if the getter requrns multiple types of entities.
	//! \return true if the character's ADS matches the required ADS.
	protected bool EvaluateEntityStance(notnull IEntity entitiy)
	{
		SCR_CharacterControllerComponent controllerComponent = SCR_CharacterControllerComponent.Cast(entitiy.FindComponent(SCR_CharacterControllerComponent));
		if (!controllerComponent)
			return true;

		bool isADS = controllerComponent.IsWeaponADS();
		return m_bMustBeADS == isADS;
	}
}
