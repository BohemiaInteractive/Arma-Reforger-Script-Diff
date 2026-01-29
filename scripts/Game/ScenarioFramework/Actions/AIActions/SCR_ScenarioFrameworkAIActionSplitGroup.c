[BaseContainerProps(), SCR_ContainerAIActionTitle()]
class SCR_ScenarioFrameworkAIActionSplitGroup : SCR_ScenarioFrameworkAIAction
{
	[Attribute(desc: "SlotAI for new group")];
	ref SCR_ScenarioFrameworkGetLayerBase m_Getter;
	
	[Attribute(defvalue: "0", desc: "Determines, how much AI's should split to new group", params: "0 inf")]
	int m_iSplitCount;
	
	[Attribute(defvalue: "", desc: "Prefabs to transfer (optional)")]
	ref array<ResourceName> m_aPrefabFilter;
	
	[Attribute(defvalue: "{000CD338713F2B5A}Prefabs/AI/Groups/Group_Base.et")]
	ResourceName m_sGroupPrefab;
	
	//------------------------------------------------------------------------------------------------
	override void OnActivate()
	{
		SCR_ScenarioFrameworkSlotAI slotAI = GetSlotAI();
		if (!slotAI)
		{
			Print(string.Format("ScenarioFramework Action: No SlotAI found on for %1.", this), LogLevel.ERROR);
			return;
		}
		
		array<AIAgent> agents = {};
		m_AIGroup.GetAgents(agents);
		if (agents.IsEmpty())
			return;
		
		int ageitnCount = agents.Count();
		
		//filter out unallowed prefabs
		if (!m_aPrefabFilter)
		{
			IEntity controlledEntity;
			for (int i = agents.Count()-1; i >= 0; i--)
			{
				controlledEntity = agents[i].GetControlledEntity();
				if (!controlledEntity)
					continue;
			
				if (!m_aPrefabFilter.Contains(controlledEntity.GetPrefabData().GetPrefabName()))
					agents.Remove(i);
			}
		}
		
		ageitnCount = agents.Count();
		if (agents.Count() <= m_iSplitCount)
		{
			Print("SCR_ScenarioFrameworkAIActionSplitGroup: Original group doesn't have enough AI's", LogLevel.ERROR);
			return;
		}
		
		agents.Resize(m_iSplitCount);
		
		AIGroup group = AIGroup.Cast(GetGame().SpawnEntityPrefab(Resource.Load(m_sGroupPrefab)));
		if (!group)
			return;
		
		foreach (AIAgent agent : agents)
		{
			group.AddAgent(agent);
		}
		
		slotAI.m_Entity = group;
		slotAI.m_AIGroup = SCR_AIGroup.Cast(group);
	}
	
	//------------------------------------------------------------------------------------------------
	protected SCR_ScenarioFrameworkSlotAI GetSlotAI()
	{
		if (!m_Getter)
			return null;
		
		SCR_ScenarioFrameworkParam<IEntity> entityWrapper = SCR_ScenarioFrameworkParam<IEntity>.Cast(m_Getter.Get());
		if (!entityWrapper)
		{
			Print(string.Format("ScenarioFramework Action: Issue with Getter detected for Action %1.", this), LogLevel.ERROR);
			return null;
		}
		
		IEntity entity = entityWrapper.GetValue();
		if (!entity)
		{
			Print(string.Format("ScenarioFramework Action: Entity not found for Action %1.", this), LogLevel.ERROR);
			return null;
		}
		
		return SCR_ScenarioFrameworkSlotAI.Cast(entity.FindComponent(SCR_ScenarioFrameworkSlotAI));
	}
}