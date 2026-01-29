[BaseContainerProps()]
class SCR_ScenarioFrameworkAIThreatStateCondition : SCR_ScenarioFrameworkActivationConditionBase
{
	[Attribute(desc: "AI Entity to evaluate. Leave empty to use entity spawned by this slot.")]
	ref SCR_ScenarioFrameworkGet m_Entity;
	
	[Attribute(defvalue: EAIThreatState.THREATENED.ToString(), UIWidgets.ComboBox, "What threat state should be checked by this condition. If used on a group, return true when at least one member of group has this threat state.", "", ParamEnumArray.FromEnum(EAIThreatState), category: "Common")]
	EAIThreatState m_eAIThreatState;

	//------------------------------------------------------------------------------------------------
	override bool Init(IEntity entity)
	{
		// Here you can debug specific Condition instance.
		// This can be also adjusted during runtime via Debug Menu > ScenarioFramework > Condition Inspector
		if (m_bDebug)
			Print("[SCR_ScenarioFrameworkAIThreatStateCondition.Init] Condition invoked with debug flag (" + __FILE__ + " L" + __LINE__ + ")", LogLevel.WARNING);
		
		SCR_AIGroup targetAIGroup;
		
		if (!m_Entity)
		{
			SCR_ScenarioFrameworkSlotAI slotAI = SCR_ScenarioFrameworkSlotAI.Cast(entity.FindComponent(SCR_ScenarioFrameworkSlotAI));
			SCR_ScenarioFrameworkSlotTaskAI slotTaskAI = SCR_ScenarioFrameworkSlotTaskAI.Cast(entity.FindComponent(SCR_ScenarioFrameworkSlotTaskAI));
			
			if (slotAI)
				targetAIGroup = slotAI.m_AIGroup;
			else if (slotTaskAI)
				targetAIGroup = slotTaskAI.m_AIGroup;
		}
		else
		{
			SCR_ScenarioFrameworkParam<IEntity> entityWrapper = SCR_ScenarioFrameworkParam<IEntity>.Cast(m_Entity.Get());
			if (!entityWrapper)
			{
				Print(string.Format("ScenarioFramework Condition: Entity not found for condition %1.", this), LogLevel.ERROR);
				return false;
			}
			
			ChimeraCharacter chimeraChar = ChimeraCharacter.Cast(entityWrapper.GetValue());
			if (!chimeraChar)
			{
				Print(string.Format("ScenarioFramework Condition: Entity not found for condition %1.", this), LogLevel.ERROR);
				return false;
			}
			
			CharacterControllerComponent charControllerComp = chimeraChar.GetCharacterController();
			if (!charControllerComp)
				return false;
			
			ChimeraAIControlComponent AIControlComponenet = ChimeraAIControlComponent.Cast(charControllerComp.GetAIControlComponent());
			if (!AIControlComponenet)
				return false;
			
			AIAgent agent = AIControlComponenet.GetAIAgent();
			if (!agent)
				return false;
			
			targetAIGroup = SCR_AIGroup.Cast(agent.GetParentGroup());
		}
		
		if (!targetAIGroup)
		{
			Print(string.Format("ScenarioFramework Action: AI Group not found for Action %1.", this), LogLevel.ERROR);
			return false;
		}
		
		array<AIAgent> agents = {};
		targetAIGroup.GetAgents(agents);
		
		SCR_AIInfoComponent infoComponent;
		SCR_AIThreatSystem threatSystem;
		SCR_ChimeraAIAgent chimeraAgent;
		
		foreach (int i, AIAgent agent : agents)
		{
			chimeraAgent = SCR_ChimeraAIAgent.Cast(agent);
			if (!chimeraAgent)
				continue;
			
			infoComponent = chimeraAgent.m_InfoComponent;
			if (!infoComponent)
				continue;
			
			threatSystem = infoComponent.GetThreatSystem();
			if (!threatSystem)
				continue;
			
			if (threatSystem.GetState() == m_eAIThreatState)
				return true;
		}
		
		return false;
	}
}