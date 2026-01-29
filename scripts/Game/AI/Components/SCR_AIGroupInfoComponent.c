[ComponentEditorProps(category: "GameScripted/AI", description: "Component for AI checking state of group")]
class SCR_AIGroupInfoComponentClass : SCR_AIInfoBaseComponentClass
{
}

enum EGroupControlMode
{
	NONE = 0,
	IDLE, 					///< Group has no waypoints and does not engage an enemy
	AUTONOMOUS, 			///< Group behaves autonomously, e.g. engaging an enemy
	FOLLOWING_WAYPOINT,		///< Group is following a waypoint
	LAST,
}

class SCR_AIGroupInfoComponent : SCR_AIInfoBaseComponent
{
	const float GRENADE_THROW_TIMEOUT_MS = 4000; // How long grenades should be disallowed after last usage
	const float ILLUM_FLARE_MIN_TIMEOUT_MS = 25*1000; // Min time illum flares should be disallowed after last usage
	const float ILLUM_FLARE_MAX_TIMEOUT_MS = 40*1000; // Max time illum flares should be disallowed after last usage
	
	private EGroupControlMode m_eGroupControlMode;
	protected SCR_AIGroupUtilityComponent m_UtilityComponent;
		
	protected ref ScriptInvoker Event_OnControlModeChanged = new ScriptInvoker();
	
	protected float m_fTimeGrenadeThrowAllowed_ms;
	
	protected float m_fTimeIllumFlareAllowed_ms;
	
	protected AIAgent m_GrenadeThrowAgent;
	
	protected ECharacterStance m_eStanceLimit = ECharacterStance.STAND; ///< upper stance limit set by group leader's stance in ActivityFollow for recruited AIs to copy;

	//---------------------------------------------------------------------------------------------------	
	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);
		SCR_AIGroup group = SCR_AIGroup.Cast(owner);
		if (!group)
			return;	
		
		m_UtilityComponent = SCR_AIGroupUtilityComponent.Cast(group.FindComponent(SCR_AIGroupUtilityComponent));
	};	
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] soldierAgent
	//! \return
	bool IsGrenadeThrowAllowed(AIAgent soldierAgent)
	{
		if (soldierAgent == m_GrenadeThrowAgent)
			return true;
		float time_ms = GetGame().GetWorld().GetWorldTime();
		return time_ms > m_fTimeGrenadeThrowAllowed_ms;
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsIllumFlareAllowed()
	{
		float time_ms = GetGame().GetWorld().GetWorldTime();
		return time_ms > m_fTimeIllumFlareAllowed_ms;
	}
	
	//------------------------------------------------------------------------------------------------
	void OnIllumFlareUsed()
	{
		float time_ms = GetGame().GetWorld().GetWorldTime();
		
		float threat;
		if (m_UtilityComponent)
			threat = m_UtilityComponent.GetThreatMeasure();
		
		float timeout = ILLUM_FLARE_MAX_TIMEOUT_MS - ILLUM_FLARE_MIN_TIMEOUT_MS;
		timeout *= (1 - threat) * 0.8 + Math.RandomFloatInclusive(0, 0.2);
		
		m_fTimeIllumFlareAllowed_ms = time_ms + ILLUM_FLARE_MIN_TIMEOUT_MS + timeout;
	}	
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] soldierAgent
	void OnAgentSelectedGrenade(AIAgent soldierAgent)
	{
		float time_ms = GetGame().GetWorld().GetWorldTime();
		m_fTimeGrenadeThrowAllowed_ms = time_ms + GRENADE_THROW_TIMEOUT_MS;
		m_GrenadeThrowAgent = soldierAgent;
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		SetEventMask(owner, EntityEvent.INIT);
	}	
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] currControlMode
	void SetGroupControlMode(EGroupControlMode currControlMode) ///< This is informative property, does not set the behavior of group to the respective state!
	{
		#ifdef AI_DEBUG
		AddDebugMessage(string.Format("SetGroupControlMode: %1", typename.EnumToString(EGroupControlMode, currControlMode)), msgType:EAIDebugMsgType.INFO);
		#endif
		
		if (currControlMode == m_eGroupControlMode)
			return;
		
		if (!Replication.IsServer())
			return;
		
		
		RplSetGroupControlMode(currControlMode);
		Rpc(RplSetGroupControlMode, currControlMode);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RplSetGroupControlMode(EGroupControlMode currControlMode)
	{
		m_eGroupControlMode = currControlMode;
		Event_OnControlModeChanged.Invoke(currControlMode);
		
		SCR_AIWorld aiWorld = SCR_AIWorld.Cast(GetGame().GetAIWorld());
		if (aiWorld)
			aiWorld.GetOnControlModeChanged().Invoke(AIGroup.Cast(GetOwner()), currControlMode);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	EGroupControlMode GetGroupControlMode()
	{
		return m_eGroupControlMode; 
	}	
	
	//------------------------------------------------------------------------------------------------
	//! \return
	ScriptInvoker GetOnControlModeChanged()
	{
		return Event_OnControlModeChanged;
	}	
	
	//======================================== RPL ========================================\\
	override bool RplSave(ScriptBitWriter writer)
    {	
        writer.WriteIntRange(m_eGroupControlMode, 0, EGroupControlMode.LAST-1);
		
        return true;
	}

	//------------------------------------------------------------------------------------------------
    override bool RplLoad(ScriptBitReader reader)
    {
		EGroupControlMode groupControlMode;
		
        reader.ReadIntRange(groupControlMode, 0, EGroupControlMode.LAST-1);
		
		RplSetGroupControlMode(groupControlMode);
		
        return true;
    }
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] stance
	void SetStanceLimit(ECharacterStance stance)
	{	
		m_eStanceLimit = stance;
	}
	
	//------------------------------------------------------------------------------------------------
	ECharacterStance GetStanceLimit()
	{	
		return m_eStanceLimit;
	}
		
	//------------------------------------------------------------------------------------------------
	//! \param[in] stance
	ECharacterStance GetAllowedStance(ECharacterStance stance)
	{
		switch (m_eStanceLimit)
		{
			case ECharacterStance.STAND:
				return stance;
			case ECharacterStance.CROUCH:
			{
				if (stance == ECharacterStance.STAND)
					stance = m_eStanceLimit;
				return stance;
			}
			case ECharacterStance.PRONE:
				return ECharacterStance.PRONE;
		}
		return stance;
	}
};