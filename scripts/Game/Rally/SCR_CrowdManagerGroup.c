[EntityEditorProps(category: "GameScripted/Rally", description: "Crowd Manager Group for Race", dynamicBox: true, visible: false)]
class SCR_CrowdManagerGroupClass : SCR_AIGroupClass
{
	const static float SIGN_CHANCE = 0.1;
}

class SCR_CrowdManagerGroup : SCR_AIGroup
{
	protected TagSystem m_TagSystem;
	protected PlayerManager m_PlayerManager;
	protected AICommunicationComponent m_CommsComp;
	protected ref ScriptInvokerBase<SCR_AIOnAnimationBehaviorAction> m_OnAnimationBehaviorActionInvoker;

	ref array<AIAgent> m_aGesturingAgents = {};

	//------------------------------------------------------------------------------------------------
	//! Creates a crowd possible bystanders to control via waypoints
	int CreateCrowd(vector searchPosition, float radius, int aiLimit = -1)
	{
		if (!m_TagSystem)
			InitiateTagSystem();

		if (!m_PlayerManager)
			InitiatePlayerManager();

		array<IEntity> entities = {};
		m_TagSystem.GetTagsInRange(entities, searchPosition, radius, ETagCategory.NameTag);

		int entitiesCount = entities.Count();
		if (aiLimit == -1)
			aiLimit = entitiesCount;
		else
			aiLimit = Math.Min(entitiesCount, aiLimit);

		for (int i = 0; i < aiLimit; i++)
		{
			int index = entities.GetRandomIndex();
			if (!entities[index]) // is null
			{
				entities.Remove(index);
				aiLimit = Math.Min(entitiesCount, aiLimit + 1);
				continue;
			}

			if (m_PlayerManager.GetPlayerIdFromControlledEntity(entities[index]) > 0) // is playercharacter
			{
				aiLimit = Math.Min(entitiesCount, aiLimit + 1);
				entities.Remove(index);
				continue;
			}

			AIControlComponent ctrlComp = AIControlComponent.Cast(entities[index].FindComponent(AIControlComponent));
			if (!ctrlComp)
			{
				entities.Remove(index);
				aiLimit = Math.Min(entitiesCount, aiLimit + 1);
				continue;
			}

			AIAgent agent = ctrlComp.GetAIAgent();
			if (!agent)
			{
				entities.Remove(index);
				aiLimit = Math.Min(entitiesCount, aiLimit + 1);
				continue;
			}

			AddAgent(agent);
			entities.Remove(index);
		}

		return GetAgentsCount();
	}

	//------------------------------------------------------------------------------------------------
	//! Disolves a crowd possible bystanders to control via waypoints
	bool DissolveCrowd(bool cancelGestures = true)
	{
		if (GetAgentsCount() == 0)
			return false;

		if (cancelGestures && !m_aGesturingAgents.IsEmpty())
			AbortMood();

		m_aGesturingAgents.Clear();

		array<AIAgent> agents = {};
		GetAgents(agents);

		foreach (AIAgent agent : agents)
		{
			RemoveAgent(agent);
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! moves crowd from one group to other
	bool GetCrowd(SCR_CrowdManagerGroup otherGroup, bool cancelOldGestures = true)
	{
		DissolveCrowd(cancelOldGestures);

		m_aGesturingAgents.Copy(otherGroup.m_aGesturingAgents);
		array<AIAgent> agents = {};
		otherGroup.GetAgents(agents);
		if (agents.IsEmpty())
			return false;

		foreach (AIAgent agent : agents)
		{
			if (!agent)
				continue;

			otherGroup.RemoveAgent(agent);
			AddAgent(agent);
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! updates mood of the group members
	void UpdateMood(int mood, bool cancelPrevious = true)
	{
		array<AIAgent> agents = {};
		GetAgents(agents);
		if (agents.IsEmpty()) //group can be dormant
			return;

		if (cancelPrevious && !m_aGesturingAgents.IsEmpty())
			AbortMood();

		int howMany = HowManyAffected(mood);

		SCR_ECharacterGestureType cheerType;
		if (Math.RandomFloat01() < SCR_CrowdManagerGroupClass.SIGN_CHANCE)
		{
			cheerType = SCR_ECharacterGestureType.SIGN;
		}
		else if (mood > 50)
		{
			cheerType = SCR_ECharacterGestureType.CHEER;
		}
		else
		{
			cheerType = SCR_ECharacterGestureType.BOO;
			mood = 100 - mood;
		}

		m_aGesturingAgents.Clear();

		for (int i=0; i < howMany; i++)
		{
			int index = agents.GetRandomIndex();
			m_aGesturingAgents.Insert(agents[index]);
			agents.Remove(index);
		}

		SendGestureOrder(m_aGesturingAgents, cheerType, mood);
	}
	
	//------------------------------------------------------------------------------------------------
	//! cancels the mood of the group members
	void AbortMood()
	{
		GetOnAnimationBehaviorAction_Group().Invoke(this, false, true, 0); //cancelling on all spectators their animations
	}
	
	//------------------------------------------------------------------------------------------------
	//! calculates how many crowd members are affected by given mood, mood close to 100 strongly positive, mood close to 0 strongly negative
	protected int HowManyAffected(int mood)
	{
		int lowerBound, upperBound;
		int numAgents = GetAgentsCount();

		if (mood == 100 || mood == 0)
		{
			return numAgents;
		}
		else if (mood > 80)
		{
			lowerBound = mood - 20;
			upperBound = mood;
		}
		else if (mood < 20)
		{
			lowerBound = 100 - mood -20;
			upperBound = 100 - mood;
		}
		else
		{
			lowerBound = Math.Max(mood - 10, 0);
			upperBound = Math.Min(mood + 10, 100);
		}

		float fraction = Math.Max(numAgents * Math.RandomInt(lowerBound, upperBound) / 100, 1);
		return Math.Floor(fraction);
	}

	//------------------------------------------------------------------------------------------------
	//! Init of PlayerManager
	protected void InitiatePlayerManager()
	{
		m_PlayerManager = GetGame().GetPlayerManager();
		if (!m_PlayerManager)
		{
			Print("SCR_CrowdManagerGroup: PlayerManager is not present in the world, cannot find bystanders.", LogLevel.ERROR);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Init of tagsystem
	protected void InitiateTagSystem()
	{
		ChimeraWorld world = ChimeraWorld.CastFrom(GetGame().GetWorld());
		m_TagSystem = TagSystem.Cast(world.FindSystem(TagSystem));
		if (!m_TagSystem)
		{
			Print("SCR_CrowdManagerGroup: TagManager is not present in the world, cannot find bystanders.", LogLevel.ERROR);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Init of comms component
	protected void InitiateComms()
	{
		m_CommsComp = GetCommunicationComponent();
		if (!m_CommsComp)
			Print("SCR_CrowdManagerGroup: Group does not have comms comp, bystanders cant talk.", LogLevel.ERROR);
	}

	//------------------------------------------------------------------------------------------------
	//! Init of invoker that manages animations played by this group's agents
	ScriptInvokerBase<SCR_AIOnAnimationBehaviorAction> GetOnAnimationBehaviorAction_Group()
	{
		if (!m_OnAnimationBehaviorActionInvoker)
			m_OnAnimationBehaviorActionInvoker = new ScriptInvokerBase<SCR_AIOnAnimationBehaviorAction>();

		return m_OnAnimationBehaviorActionInvoker;
	}

	//------------------------------------------------------------------------------------------------
	//! Sending AI message to agent
	protected void SendGestureOrder(notnull array<AIAgent> agentsToSend, SCR_ECharacterGestureType gestureType, int mood)
	{
		if (!m_CommsComp)
			InitiateComms();
		
		ScriptInvokerBase<SCR_AIOnAnimationBehaviorAction> relatedInvoker = GetOnAnimationBehaviorAction_Group();

#ifdef AI_DEBUG
		PrintFormat("Sending gesture %1 with mood %2 to %3 agents", typename.EnumToString(SCR_ECharacterGestureType, gestureType), mood, agentsToSend.Count(), level: LogLevel.NORMAL);
#endif // AI_DEBUG

		foreach (AIAgent agent : agentsToSend)
		{
			SCR_ChimeraAIBystander bystander = SCR_ChimeraAIBystander.Cast(agent);
			if (!bystander)	
				continue;
			
			bystander.GetOnAnimationBehaviorAction().Invoke(agent, false, true, 0);
			SCR_AIOrder_Gesture msg = SCR_AIOrder_Gesture.Create(gestureType, mood, relatedInvoker);
			msg.SetReceiver(bystander);
			m_CommsComp.RequestBroadcast(msg, bystander);
		}
	}
}
