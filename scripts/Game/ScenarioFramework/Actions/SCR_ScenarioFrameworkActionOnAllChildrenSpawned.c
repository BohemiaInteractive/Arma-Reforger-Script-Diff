[BaseContainerProps(), SCR_ContainerAIActionTitle()]
class SCR_ScenarioFrameworkActionOnAllChildrenSpawned : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Getter for scenarioFramework layerBase (or inherited class), which will be checked")];
	ref SCR_ScenarioFrameworkGet m_Getter;

	[Attribute(desc: "Actions to be called")]
	ref array<ref SCR_ScenarioFrameworkActionBase> m_aActions;
	
	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;
		
		//Process Array
		SCR_ScenarioFrameworkParam<array<IEntity>> entityArrayWrapper = SCR_ScenarioFrameworkParam<array<IEntity>>.Cast(m_Getter.Get());
		if (entityArrayWrapper)
		{
			array<IEntity> entities = {};
			entities = entityArrayWrapper.GetValue();
				
			if (entities.IsEmpty())
			{
				Print(string.Format("ScenarioFramework Action: Array Getter for %1 has no elements to work with.", this), LogLevel.ERROR);
				return;
			}
				
			foreach (IEntity entity : entities)
			{
				ProcessEntity(entity);
			}
				
			return;
		}
		
		//Process single entity
		SCR_ScenarioFrameworkParam<IEntity> entityWrapper = SCR_ScenarioFrameworkParam<IEntity>.Cast(m_Getter.Get());
		IEntity entity = entityWrapper.GetValue();
		if (!entity)
		{
			Print(string.Format("ScenarioFramework Action: Entity not found for Action %1.", this), LogLevel.ERROR);
			return;
		}
	
		ProcessEntity(entity);
	}
	
	//------------------------------------------------------------------------------------------------
	void ProcessEntity(IEntity ent)
	{
		SCR_ScenarioFrameworkLayerBase layerBase = SCR_ScenarioFrameworkLayerBase.Cast(ent.FindComponent(SCR_ScenarioFrameworkLayerBase));
		if (!layerBase)
		{
			//Pass entities directly to Actions
			foreach (SCR_ScenarioFrameworkActionBase action : m_aActions)
			{
				action.OnActivate(ent);
			}
			
			return;
		}
		
		if (layerBase.GetIsInitiated())
			ActivateActions(layerBase);
		
		layerBase.GetOnAllChildrenSpawned().Insert(ActivateActions);
			
	}
	
	//------------------------------------------------------------------------------------------------
	void ActivateActions(notnull SCR_ScenarioFrameworkLayerBase layerBase)
	{
		foreach (SCR_ScenarioFrameworkActionBase action : m_aActions)
		{
			action.OnActivate(layerBase.GetOwner());
		}
	}
}