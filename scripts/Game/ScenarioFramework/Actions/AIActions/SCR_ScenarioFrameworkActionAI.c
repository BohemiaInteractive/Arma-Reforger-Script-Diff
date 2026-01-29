[BaseContainerProps(), SCR_ContainerAIActionTitle()]
class SCR_ScenarioFrameworkActionAI : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Target SlotAI for AI Action - if left empty, it will attempt to retrieve it from the Slot it is attached to. Additionally you can use Get Array Of LayerBases and this action will be executed on all of them.")];
	ref SCR_ScenarioFrameworkGet m_Getter;

	[Attribute(desc: "AI actions that will be executed on target AI")];
	ref array<ref SCR_ScenarioFrameworkAIAction> m_aAIActions;

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;
		
		if (!m_Getter)
		{
			ProcessEntity(object, object);
		}
		else
		{
			SCR_ScenarioFrameworkParam<IEntity> entityWrapper = SCR_ScenarioFrameworkParam<IEntity>.Cast(m_Getter.Get());
			if (!entityWrapper)
			{
				SCR_ScenarioFrameworkParam<array<IEntity>> entityArrayWrapper = SCR_ScenarioFrameworkParam<array<IEntity>>.Cast(m_Getter.Get());
				if (!entityArrayWrapper)
				{
					Print(string.Format("ScenarioFramework Action: Issue with Getter detected for Action %1.", this), LogLevel.ERROR);
					return;
				}
				
				array<IEntity> entities = {};
				entities = entityArrayWrapper.GetValue();
				
				if (entities.IsEmpty())
				{
					Print(string.Format("ScenarioFramework Action: Array Getter for %1 has no elements to work with.", this), LogLevel.ERROR);
					return;
				}
				
				foreach (IEntity entity : entities)
				{
					ProcessEntity(object, entity);
				}
				
				return;
			}
			
			IEntity entity = entityWrapper.GetValue();
			if (!entity)
			{
				Print(string.Format("ScenarioFramework Action: Entity not found for Action %1.", this), LogLevel.ERROR);
				return;
			}
	
			ProcessEntity(object, entity);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void ProcessEntity(IEntity object, IEntity entity)
	{
		if (!entity)
		{
			Print(string.Format("ScenarioFramework Action: Entity not found for Action %1.", this), LogLevel.ERROR);
			return;
		}
	
		SCR_ScenarioFrameworkSlotAI slotAI = SCR_ScenarioFrameworkSlotAI.Cast(entity.FindComponent(SCR_ScenarioFrameworkSlotAI));
		SCR_ScenarioFrameworkSlotTaskAI slotTaskAI = SCR_ScenarioFrameworkSlotTaskAI.Cast(entity.FindComponent(SCR_ScenarioFrameworkSlotTaskAI));
		if (!slotAI && !slotTaskAI)
		{
			Print(string.Format("ScenarioFramework Action: SlotAI not found for Action %1.", this), LogLevel.ERROR);
			return;
		}
		
		SCR_AIGroup targetAIGroup;
		if (slotAI)
			targetAIGroup = slotAI.m_AIGroup;
		else if (slotTaskAI)
			targetAIGroup = slotTaskAI.m_AIGroup;
		
		if (!targetAIGroup)
		{
			Print(string.Format("ScenarioFramework Action: AI Group not found for Action %1.", this), LogLevel.ERROR);
			return;
		}

		foreach (SCR_ScenarioFrameworkAIAction AIAction : m_aAIActions)
		{
			AIAction.Init(targetAIGroup, object);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	array<ref SCR_ScenarioFrameworkAIAction> GetSubActionsAI()
	{
		return m_aAIActions;
	}
}