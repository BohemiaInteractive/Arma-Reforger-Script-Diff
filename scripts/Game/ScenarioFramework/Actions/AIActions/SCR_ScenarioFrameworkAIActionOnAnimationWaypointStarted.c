[BaseContainerProps(), SCR_ContainerAIActionTitle()]
class SCR_ScenarioFrameworkAIActionOnAnimationWaypointStarted : SCR_ScenarioFrameworkAIAction
{
	[Attribute(desc: "SlotWaypoint that spawns waypoint")]
	ref SCR_ScenarioFrameworkGetLayerBase m_Getter;

	[Attribute(desc: "Actions that will be executed upon provided waypoint completion for provided AI group")]
	ref array<ref SCR_ScenarioFrameworkActionBase> m_aActionsOnWaypointCompleted;

	[Attribute(defvalue: "1", desc: "Remove the event on action completion")]
	bool m_bRemoveOnCompleted;

	[Attribute("0", UIWidgets.ComboBox, "Set which state of the animation should be evaluated.", "", ParamEnumArray.FromEnum(SCR_EAnimationStateToCheck))]
	SCR_EAnimationStateToCheck m_eEvaluationState;

	[Attribute("-1", "Index of animation that should trigger the event. Leave -1 and set m_eEvaluationState to evaluate only animation as a whole.", "")]
	protected int m_iAnimationIndex;

	SCR_AIAnimationWaypoint m_Waypoint;
	IEntity m_AISlotEntity;

	//------------------------------------------------------------------------------------------------
	override void Init(SCR_AIGroup targetAIGroup, IEntity entity)
	{
		if (entity)
		{
			SCR_ScenarioFrameworkLayerBase layer = SCR_ScenarioFrameworkLayerBase.Cast(entity.FindComponent(SCR_ScenarioFrameworkLayerBase));
			if (layer && (SCR_ScenarioFrameworkSlotAI.Cast(layer) || SCR_ScenarioFrameworkSlotTaskAI.Cast(layer)))
				m_AISlotEntity = entity;
		}

		super.Init(targetAIGroup, entity);
	}

	//------------------------------------------------------------------------------------------------
	override void OnActivate()
	{
		super.OnActivate();

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

		IEntity getterEntity = entityWrapper.GetValue();
		if (!getterEntity)
		{
			Print(string.Format("ScenarioFramework Action: Entity not found for Action %1.", this), LogLevel.ERROR);
			return;
		}

		SCR_ScenarioFrameworkLayerBase layer = SCR_ScenarioFrameworkLayerBase.Cast(getterEntity.FindComponent(SCR_ScenarioFrameworkLayerBase));
		if (!layer)
		{
			Print(string.Format("ScenarioFramework Action: Entity is not Layer Base for Action %1.", this), LogLevel.ERROR);
			return;
		}

		IEntity spawnedEntity = layer.GetSpawnedEntity();
		if (!spawnedEntity)
		{
			layer.GetOnAllChildrenSpawned().Insert(OnWaypointSpawned);
			return;
		}

		m_Waypoint = SCR_AIAnimationWaypoint.Cast(spawnedEntity);
		if (!m_Waypoint)
		{
			Print(string.Format("ScenarioFramework Action: Waypoint not found for Action %1.", this), LogLevel.ERROR);
			return;
		}

		m_Waypoint.GetOnAnimationBehaviorAction().Insert(OnAnimationStarted);
	}

	//------------------------------------------------------------------------------------------------
	void OnWaypointSpawned(SCR_ScenarioFrameworkLayerBase layer)
	{
		layer.GetOnAllChildrenSpawned().Remove(OnWaypointSpawned);

		IEntity spawnedEntity = layer.GetSpawnedEntity();
		if (!spawnedEntity)
		{
			Print(string.Format("ScenarioFramework Action: Entity not found for Action %1.", this), LogLevel.ERROR);
			return;
		}

		m_Waypoint = SCR_AIAnimationWaypoint.Cast(spawnedEntity);
		if (!m_Waypoint)
		{
			Print(string.Format("ScenarioFramework Action: Waypoint not found for Action %1.", this), LogLevel.ERROR);
			return;
		}

		m_Waypoint.GetOnAnimationBehaviorAction().Insert(OnAnimationStarted);
	}

	//------------------------------------------------------------------------------------------------
	void OnAnimationStarted(AIAgent agent, bool startingAnimation, int animationIndex)
	{
		if (!m_Waypoint)
			return;

		// The animation index wasn't set, so we run event based on the SCR_EAnimationStateToCheck setting, or the index fits.
		if (m_iAnimationIndex == -1 || m_iAnimationIndex == animationIndex)
			EvaluateAnimationState(startingAnimation);
	}

	//------------------------------------------------------------------------------------------------
	void EvaluateAnimationState(bool startingAnimation)
	{
		switch (m_eEvaluationState)
		{
			case SCR_EAnimationStateToCheck.NONE:
				break;

			case SCR_EAnimationStateToCheck.IN:
				if (startingAnimation)
					PerformActions();
				break;

			case SCR_EAnimationStateToCheck.OUT:
				if (!startingAnimation)
					PerformActions();
				break;

			case SCR_EAnimationStateToCheck.BOTH:
				PerformActions();
				break;
		}
	}

	//------------------------------------------------------------------------------------------------
	void PerformActions()
	{
		if (m_bRemoveOnCompleted)
			m_Waypoint.GetOnAnimationBehaviorAction().Remove(OnAnimationStarted);

		foreach (SCR_ScenarioFrameworkActionBase action : m_aActionsOnWaypointCompleted)
		{
			action.Init(m_AISlotEntity);
		}
	}

	//------------------------------------------------------------------------------------------------
	override array<ref SCR_ScenarioFrameworkActionBase> GetSubActions()
	{
		return m_aActionsOnWaypointCompleted;
	}
}