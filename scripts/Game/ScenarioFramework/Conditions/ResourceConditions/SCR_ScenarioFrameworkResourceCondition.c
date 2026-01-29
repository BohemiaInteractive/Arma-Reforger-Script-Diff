[BaseContainerProps()]
class SCR_ScenarioFrameworkResourceCondition : SCR_ScenarioFrameworkActivationConditionBase
{
	[Attribute(desc: "Entity to check.")]
	ref SCR_ScenarioFrameworkGet m_Getter;
	
	[Attribute(defvalue: "EResourceType.INVALID.ToString()", desc: "Resource Type", uiwidget: UIWidgets.ComboBox, enumType: EResourceType)]
	EResourceType m_eResourceType;

	[Attribute(desc: "Required resource related conditions.")]
	ref array<ref SCR_ScenarioFrameworkResourceConditionBase> m_aRequiredResourceConditions;
	
	//------------------------------------------------------------------------------------------------
	override bool Init(IEntity entity)
	{
		// Here you can debug specific Condition instance.
		// This can be also adjusted during runtime via Debug Menu > ScenarioFramework > Condition Inspector
 		if (m_bDebug)
			Print("[SCR_ScenarioFrameworkResourceCondition.Init] Condition invoked with debug flag (" + __FILE__ + " L" + __LINE__ + ")", LogLevel.WARNING);
		
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
				if (!EvaluateResourceConditions(ent))
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

		return EvaluateResourceConditions(wrappedEntity);
	}

	//------------------------------------------------------------------------------------------------
	bool EvaluateResourceConditions(notnull IEntity ent)
	{
		SCR_ResourceComponent resourceComponent = SCR_ResourceComponent.Cast(ent.FindComponent(SCR_ResourceComponent));
		if (!resourceComponent)
			return false;
		
		foreach (SCR_ScenarioFrameworkResourceConditionBase requiredCondition : m_aRequiredResourceConditions)
		{
			if (!requiredCondition.Init(ent, resourceComponent, m_eResourceType))
				return false;
		}
		return true;	
	}
}