[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionLoopOverNotRandomlySelectedLayers : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Use GetRandomLayerBase")]
	ref SCR_ScenarioFrameworkGet m_Getter;

	[Attribute(defvalue: "1", desc: "Which actions will be executed for each layer that was not randomly selected", UIWidgets.Auto)]
	ref array<ref SCR_ScenarioFrameworkActionBase> m_aActions;

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		if (!m_Getter)
		{
			Print(string.Format("ScenarioFramework Action: Missing Getter for action %1.", this), LogLevel.ERROR);
			return;
		}

		SCR_ScenarioFrameworkGetRandomLayerBase randomLayerGetter = SCR_ScenarioFrameworkGetRandomLayerBase.Cast(m_Getter);
		if (!randomLayerGetter)
		{
			Print(string.Format("ScenarioFramework Action: Used wrong Getter for Action %1. Use GetRandomLayerBase instead.", this), LogLevel.ERROR);
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

		string excludedEntity = entity.GetName();
		IEntity notSelectedEntity;

		foreach (string layer : randomLayerGetter.m_aNameOfLayers)
		{
			if (layer == excludedEntity)
				continue;

			notSelectedEntity = m_Getter.FindEntityByName(layer);
			if (!notSelectedEntity)
				continue;

			foreach (SCR_ScenarioFrameworkActionBase actions : m_aActions)
			{
				actions.OnActivate(notSelectedEntity);
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override array<ref SCR_ScenarioFrameworkActionBase> GetSubActions()
	{
		return m_aActions;
	}
}