[BaseContainerProps()]
class SCR_ScenarioFrameworkTaskStatusCondition : SCR_ScenarioFrameworkActivationConditionBase
{
	[Attribute(desc: "Layer task(s) to check for condition. If multiple layer tasks are specified, they must all have an acceptable state.")]
	ref SCR_ScenarioFrameworkGet m_Getter;

	[Attribute(desc: "If the layer task is any of these states, the condition will be statisfied", defvalue: SCR_ETaskState.COMPLETED.ToString(), uiwidget: UIWidgets.ComboBox, enumType: SCR_ETaskState)]
	ref array<SCR_ETaskState> m_aAcceptableTaskStates;

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
		if (entityWrapper)
			return EvaluateEntityAsTaskState(entityWrapper.GetValue());

		// Check if multiple entities are referenced
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
			if (!EvaluateEntityAsTaskState(ent))
				return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Evaluates if given entity is a SCR_ScenarioFrameworkLayerTask, then calls EvaluateTaskState on it
	//! \param[in] entity Evaluates entity as ScenarioFrameworkLayerTask, checks if it exists, then evaluates its state.
	//! \return whether the condition is satisfied for the given task in Scenario Framework.
	protected bool EvaluateEntityAsTaskState(IEntity entity)
	{
		if (!entity)
		{
			Print(string.Format("ScenarioFramework Condition: Entity was null for condition %1.", this), LogLevel.ERROR);
			return false;
		}

		SCR_ScenarioFrameworkLayerTask task = SCR_ScenarioFrameworkLayerTask.Cast(entity.FindComponent(SCR_ScenarioFrameworkLayerTask));
		if (!task)
		{
			Print(string.Format("ScenarioFramework Condition: entity %2 had no SCR_ScenarioFrameworkLayerTask for condition %1.", this, entity), LogLevel.ERROR);
			return false;
		}

		return EvaluateTaskState(task);
	}

	//------------------------------------------------------------------------------------------------
	//! Checks if the layer's state is acceptable.
	//! \param[in] layer A Scenerio Framework layer task
	//! \return true if state is acceptable.
	protected bool EvaluateTaskState(notnull SCR_ScenarioFrameworkLayerTask layer)
	{
		SCR_ETaskState state = layer.GetLayerTaskState();
		return m_aAcceptableTaskStates.Contains(state);
	}
}
