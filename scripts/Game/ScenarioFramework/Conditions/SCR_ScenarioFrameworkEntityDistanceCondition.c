[BaseContainerProps()]
class SCR_ScenarioFrameworkEntityDistanceCondition : SCR_ScenarioFrameworkActivationConditionBase
{
	[Attribute(desc: "Entity A")]
	ref SCR_ScenarioFrameworkGet m_GetterA;

	[Attribute(desc: "Entity B")]
	ref SCR_ScenarioFrameworkGet m_GetterB;

	[Attribute(defvalue: "0", desc: "Minimum distance between entities in metres (Inclusive).", params: "0 inf 0.1", precision: 1)]
	float m_fMinDistance;

	[Attribute(defvalue: "20", desc: "Maximum distance between entities in metres (Inclusive). -1 for infinity", params: "-1 inf 0.1", precision: 1)]
	float m_fMaxDistance;

	//------------------------------------------------------------------------------------------------
	override bool Init(IEntity entity)
	{
		// Here you can debug specific Condition instance.
		// This can be also adjusted during runtime via Debug Menu > ScenarioFramework > Condition Inspector
		if (m_bDebug)
			Print("[SCR_ScenarioFrameworkEntityDistanceCondition.Init] debug line (" + __FILE__ + " L" + __LINE__ + ")", LogLevel.WARNING);

		if (m_fMaxDistance < 0)
			m_fMaxDistance = float.INFINITY;
		if (m_fMinDistance > m_fMaxDistance)
		{
			Print(string.Format("ScenarioFramework Condition: Min distance %1 is larger than max distance %2. If you intended to exclude a range, then use the OR operator on two distance conditions instead.", m_fMinDistance, m_fMaxDistance, this), LogLevel.WARNING);
			return false;
		}
		
		if (!m_GetterA || !m_GetterB)
			return false;

		SCR_ScenarioFrameworkParam<IEntity> entityWrapperA = SCR_ScenarioFrameworkParam<IEntity>.Cast(m_GetterA.Get());
		if (!entityWrapperA)
		{
			Print(string.Format("ScenarioFramework Condition: Entity A not found for condition %1.", this), LogLevel.ERROR);
			return false;
		}

		SCR_ScenarioFrameworkParam<IEntity> entityWrapperB = SCR_ScenarioFrameworkParam<IEntity>.Cast(m_GetterB.Get());
		if (!entityWrapperB)
		{
			Print(string.Format("ScenarioFramework Condition: Entity B not found for condition %1.", this), LogLevel.ERROR);
			return false;
		}

		IEntity wrappedEntityA = entityWrapperA.GetValue();
		if (!wrappedEntityA)
		{
			Print(string.Format("ScenarioFramework Condition: Entity A not found for condition %1.", this), LogLevel.ERROR);
			return false;
		}

		IEntity wrappedEntityB = entityWrapperB.GetValue();
		if (!wrappedEntityB)
		{
			Print(string.Format("ScenarioFramework Condition: Entity B not found for condition %1.", this), LogLevel.ERROR);
			return false;
		}

		float distance = vector.Distance(wrappedEntityA.GetOrigin(), wrappedEntityB.GetOrigin());
		return distance >= m_fMinDistance && distance <= m_fMaxDistance;
	}
}
