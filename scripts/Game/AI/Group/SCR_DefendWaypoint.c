class SCR_DefendWaypointClass: SCR_TimedWaypointClass
{
};

//----------------------------------------------------------------------------------------
class SCR_DefendWaypoint : SCR_TimedWaypoint
{
	[Attribute("0", UIWidgets.Object, "Fast init - units will be spawned on their defensive locations")];
	protected bool m_bFastInit;
	
	[Attribute("", UIWidgets.Object, "Defend presets")];
	protected ref array<ref SCR_DefendWaypointPreset> m_aDefendPresets;
	
	protected int m_iCurrentDefendPreset;
	
	//----------------------------------------------------------------------------------------
	bool GetFastInit()
	{
		return m_bFastInit;
	}
	
	//----------------------------------------------------------------------------------------
	void SetFastInit(bool isFastInit)
	{
		m_bFastInit = isFastInit;
	}
	
	//----------------------------------------------------------------------------------------
	override SCR_AIWaypointState CreateWaypointState(SCR_AIGroupUtilityComponent groupUtilityComp)
	{
		return new SCR_AIDefendWaypointState(groupUtilityComp, this);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Clears Defend Presets
	void ClearDefendPresets()
	{
		if (m_aDefendPresets)
			m_aDefendPresets.Clear();
	}

	//----------------------------------------------------------------------------------------
	SCR_DefendWaypointPreset GetCurrentDefendPreset()
	{
		if (!m_aDefendPresets.IsIndexValid(m_iCurrentDefendPreset))
			return null;
		return m_aDefendPresets[m_iCurrentDefendPreset];
	}
	
	//----------------------------------------------------------------------------------------
	int GetCurrentDefendPresetIndex()
	{
		return m_iCurrentDefendPreset;
	}
	
	//----------------------------------------------------------------------------------------
	bool SetCurrentDefendPreset(int newDefendPresetIndex)
	{
		if (m_aDefendPresets.IsIndexValid(newDefendPresetIndex))
		{ 
			m_iCurrentDefendPreset = newDefendPresetIndex;
			return true;
		}
		return false;
	}
	
	//----------------------------------------------------------------------------------------
	bool AddDefendPreset(SCR_DefendWaypointPreset preset)
	{
		if (!m_aDefendPresets.Contains(preset))
		{
			m_aDefendPresets.Insert(preset);
			return true;
		}
		return false;
	}
	
	//----------------------------------------------------------------------------------------
	bool RemoveDefendPreset(int presetIndex)
	{
		if (m_aDefendPresets.IsIndexValid(presetIndex))
		{
			m_aDefendPresets.Remove(presetIndex);
			return true;
		}
		return false;
	}
};

//----------------------------------------------------------------------------------------
class SCR_AIDefendWaypointState : SCR_AIWaypointState
{
	override void OnDeselected()
	{
		super.OnDeselected();
		
		SCR_AIGroup myGroup = m_Utility.m_Owner;
		
		if (!myGroup)
			return;
		
		array<AIAgent> groupMembers = {};
		myGroup.GetAgents(groupMembers);
				
		AICommunicationComponent mailbox = myGroup.GetCommunicationComponent();
		if (!mailbox)
			return;
		
		foreach (AIAgent receiver: groupMembers)
		{
			if (receiver)
			{
				SCR_ChimeraAIAgent chimeraAgent = SCR_ChimeraAIAgent.Cast(receiver);
				if (!chimeraAgent)
					continue;
				
				SCR_AIInfoComponent aiInfo = chimeraAgent.m_InfoComponent;
				if (!aiInfo || !aiInfo.HasUnitState(EUnitState.IN_TURRET))
					continue;
			
				ChimeraCharacter character = ChimeraCharacter.Cast(receiver.GetControlledEntity());
				if (character && character.IsInVehicle())
				{
					SCR_AIMessage_GetOut msg2 = new SCR_AIMessage_GetOut();
					
					msg2.SetText("Waypoint was deselected, leave vehicle");
					msg2.SetReceiver(receiver);
					mailbox.RequestBroadcast(msg2, receiver);
				}
			}
		}
		myGroup.ReleaseCompartments();
	}
};