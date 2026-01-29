//------------------------------------------------------------------------------------------------
//! Entity states for nametags
//! Order affects state priority with DEFAULT state being the lowest
enum ENameTagEntityState
{
	DEFAULT		 	= 1,	// alive
	FOCUSED 		= 1<<1,	// focused
	AI_SUBORDINATE	= 1<<2, // used for AIs that are under players command through commanding system
	GROUP_MEMBER	= 1<<3,	// part of the same squad
	GROUP_LEADER	= 1<<4, // squad leader
	UNCONSCIOUS 	= 1<<5,	// unconscious
	VON 			= 1<<6,	// voice over network
	DEAD 			= 1<<7,	// dead
	HIDDEN 			= 1<<8	// tag is hidden
};

//------------------------------------------------------------------------------------------------
//! Nametag flags
enum ENameTagFlags
{
	UNUSED 				= 1,			
	VISIBLE_PASS 		= 1<<1,		// this tag passes visiblity conditions, although it still might not be visible due to tag limit
	CLEANUP 			= 1<<2,		// irreversible cleanup
	VISIBLE 			= 1<<3,		// tag visibility (can still be animating/fading when not visible)
	DISABLED 			= 1<<4,		// reversible cleanup
	DELETED 			= 1<<5,		// irreversible cleanup
	UPDATE_DISABLE 		= 1<<6,		// tag is not visible, in the process of being disabled, update until transitions are finished
	VEHICLE_DISABLE 	= 1<<7,		// reversible cleanup for vehicle which no longer contains passengers 
	FADE_TIMER 			= 1<<8,		// run fade timer for obstruction check
	OBSTRUCTED			= 1<<9,		// LOS trace result obstructed 
	VEHICLE 			= 1<<10,	// tag owner is in a vehicle
	TURRET 				= 1<<11,	// tag owner in in a turret which is not part of a vehicle
	ENT_TYPE_UPDATE 	= 1<<12,	// request entity type update
	NAME_UPDATE			= 1<<13		// request name update
};

//------------------------------------------------------------------------------------------------
//! Determines which offset is used for setting tag position 
enum ENameTagPosition
{
	HEAD = 0,
	BODY
};

//------------------------------------------------------------------------------------------------
//! Tag entity type
enum ENameTagEntityType
{
	PLAYER,     // player character
	AI,			// AI character
	VEHICLE		// vehicle
};

//------------------------------------------------------------------------------------------------
//! Nametag data
class SCR_NameTagData : Managed
{	
	const string HEAD_BONE = "head";
	const string SPINE_BONE = "Neck1";
	const vector HEAD_OFFSET = "0 0.3 0";			// tag visual position offset for head
	const vector BODY_OFFSET = "0 -0.1 0";			// tag visual position offset for body
	
	ENameTagFlags m_Flags;
	ENameTagEntityState m_eEntityStateFlags;
	ENameTagEntityState m_ePriorityEntityState;
	ENameTagPosition m_eAttachedTo;
	ENameTagPosition m_eAttachedToLast;
	ENameTagEntityType m_eType;
	
	bool m_bIsCurrentPlayer;		// is controlled player
	int m_iZoneID;					// nametag zone ID
	int m_iGroupID;
	int m_iPlayerID;
	int m_iSpineBone;
	int m_iHeadBone;
	float m_fTimeSliceUpdate;		// timed update for distance/trace checks, start at 1 for initial update
	float m_fTimeSliceVON;			// delay before switching out of VON state
	float m_fTimeSlicePosChange;	// for lerping when nametag position changes
	float m_fTimeSliceCleanup;		// if tag is out of zone range for set amount of time, it will clean itself up
	float m_fTimeSliceFade;			// measuring time between visibility traces to determine whether the tag should fade
	float m_fTimeSliceVisibility;	// Slice for animation changes to visibility
	float m_fDistance;				// distance from player to this entity, pow of 2 of the real distance for calculation purposes
	float m_fOpacityFade;			// opacity fade based on distance, is value between 0.1-1 (percentage), a secondary effect to global opacity setting
	float m_fVisibleOpacity = 1;	// cached default opacity
	float m_fAngleToScreenCenter;	// angle between this entity and the center of the screen used for visibility limiting
	vector m_vTagScreenPos;			// tag screen pos (2D) - in reference resolution (not screen size) values
	vector m_vTagWorldPos;			// tag world pos (with offset)
	vector m_vTagWorldPosLast;		// previous tag world pos for lerping 
	vector m_vEntHeadPos;			// ent head pos, for LOS checks and head placement
	vector m_vEntWorldPos;			// ent world pos, for visibility angle checks and body placement
	string m_sName;					// entity name or name formatting
	ref array<string> m_aNameParams;// Params for name formatting eg: Firstname, Alias (can be an empty string), Surname
	
	IEntity m_Entity;			
	Widget m_NameTagWidget;										// tag visiblity setting is done on this level because setting it on root with negative values conflicts with render ZOrder
	SCR_VehicleTagData m_VehicleParent;
	SCR_CharacterControllerComponent m_CharController;
	protected SCR_NameTagDisplay m_NTDisplay;
	protected BaseCompartmentSlot m_VehicleCompartment; 		// vehicle compartment slot if entity is in a vehicle
	protected SCR_GroupsManagerComponent m_GroupManager;
	protected SCR_PossessingManagerComponent m_PossessingManager;
	
	// configurable data
	float m_fDeadEntitiesCleanup;	
	float m_fTagFadeSpeed;
		
					
	ref Widget m_aNametagElements[10];	// nametag layout elements
	//------------------------------------------------------------------------------------------------
	//! Update the highest priority entity state (as ordered in enum)
	void UpdatePriorityEntityState()
	{
		int flag = 1; 
		int highestMatch = 1;
		
		for ( int i = 0; i < 31; i++ )
		{
			// Stop if impossibly high flag
			if (m_eEntityStateFlags < flag)
			{
				m_ePriorityEntityState = highestMatch;
				return;
			}
			
			if (m_eEntityStateFlags & flag)
				highestMatch = flag;
			 				
			flag = flag << 1;
		}
		
		m_ePriorityEntityState = ENameTagEntityState.DEFAULT;
	}
		
	//------------------------------------------------------------------------------------------------
	//! Add entity state flag
	//! \param state is the flag to be added
	void ActivateEntityState(ENameTagEntityState state)
	{	
		// flag NOT set
		if (~m_eEntityStateFlags & state)
		{
			m_eEntityStateFlags |= state;
			UpdatePriorityEntityState();
			
			m_NTDisplay.UpdateTagElements(this);
		
			// If dead, prep delayed callback for auto deactivation after a set time		
			if (state == ENameTagEntityState.DEAD && ( ~m_Flags & ENameTagFlags.CLEANUP) )
			{
				m_Flags |= ENameTagFlags.CLEANUP;
				SetTagPosition(ENameTagPosition.HEAD);
				
				ScriptCallQueue queue = GetGame().GetCallqueue();
				if (!queue)
					return;
				
				queue.CallLater(Cleanup, m_fDeadEntitiesCleanup * 1000, false, true);
			}
		}
		
		// VON screen fade after a set time, outside of the falg set condition to reset timer
		if (state == ENameTagEntityState.VON )
			m_fTimeSliceVON = 0;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Remove ENameTagEntityState flag
	//! \param state is the flag to be removed
	void DeactivateEntityState(ENameTagEntityState state)
	{
		// flag set
		if (m_eEntityStateFlags & state)
		{
			m_eEntityStateFlags &= ~state;
			UpdatePriorityEntityState();
			
			m_NTDisplay.UpdateTagElements(this);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set visibility of nametag widget
	//! \param widget is the target widget
	//! \param visible controls whether the widget should be made visible or invisible
	//! \param animate controls if the widget should fade in/out 
	void SetVisibility(Widget widget, bool visible, float visibleOpacity, bool animate = true)
	{
		if (!widget)
			return;
						
		float targetVal;
		if (visible)
		{
			m_NameTagWidget.SetVisible(true);
			if (widget == m_NameTagWidget)
				m_fVisibleOpacity = visibleOpacity;
			
			m_Flags |= ENameTagFlags.VISIBLE;
			m_Flags &= ~ENameTagFlags.UPDATE_DISABLE;
			m_Flags &= ~ENameTagFlags.DISABLED;
			targetVal = visibleOpacity;
		}
		else 
			targetVal = 0;
		
		if (m_fTagFadeSpeed == 0)
			animate = false;
		
		if (animate)
			AnimateWidget.Opacity(widget, targetVal, m_fTagFadeSpeed);
		else 
		{
			AnimateWidget.StopAnimation(widget, WidgetAnimationOpacity);
			widget.SetOpacity(targetVal);
			if (targetVal >= 0.1)
				widget.SetVisible(true);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Resize nametag element
	void ResizeElement(Widget widget, float targetVal)
	{
		float size[2] = {targetVal, targetVal};
		AnimateWidget.Size(widget, size, m_fTagFadeSpeed);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Determine type of nametag entity
	void UpdateEntityType()
	{
		if (ChimeraCharacter.Cast(m_Entity))
		{				
			bool playerIDIsMainEnt = false;
			
			m_iPlayerID = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(m_Entity);
			if (m_iPlayerID == 0 && m_PossessingManager)
			{
				m_iPlayerID = m_PossessingManager.GetIdFromMainEntity(m_Entity);	// in case this is a main entity of someone whos currently possessing, consider it that player
				if (m_iPlayerID > 0)
					playerIDIsMainEnt = true;
			}
			
			if (m_iPlayerID > 0)
			{
				m_eType = ENameTagEntityType.PLAYER;
				
				if (!playerIDIsMainEnt && m_PossessingManager)
				{
					if (m_PossessingManager.IsPossessing(m_iPlayerID))		// possessed AI entity should keep its name
						m_eType = ENameTagEntityType.AI;
				}
				
				if (m_GroupManager)
				{
					SCR_AIGroup group = m_GroupManager.GetPlayerGroup(m_iPlayerID);
					SetGroup(group);
				}
			}
			else 	// non players are considered AI
			{
				m_eType = ENameTagEntityType.AI;
				CheckAISubordinate(m_Entity);				
			}
		}
		else if (Vehicle.Cast(m_Entity))
		{
			m_eType = ENameTagEntityType.VEHICLE;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get/update nametag name
	//! \param[out] name Name or formatting of name
	//! \param[out] names If uses formating: Firstname, Alias and Surname (Alias can be an empty string)
	void GetName(out string name, out notnull array<string> nameParams)
	{		
		if (m_eType == ENameTagEntityType.PLAYER)
		{
			PlayerManager playerMgr = GetGame().GetPlayerManager();
			if (playerMgr)
				m_sName = SCR_PlayerNamesFilterCache.GetInstance().GetPlayerDisplayName(m_iPlayerID);
			else 
				m_sName = "No player manager!"
		}
		else if (m_eType == ENameTagEntityType.AI)
		{
			SCR_CharacterIdentityComponent scrCharIdentity = SCR_CharacterIdentityComponent.Cast(m_Entity.FindComponent(SCR_CharacterIdentityComponent));
			if (scrCharIdentity)
			{
				scrCharIdentity.GetFormattedFullName(m_sName, m_aNameParams);
			}
			else
			{
				CharacterIdentityComponent charIdentity = CharacterIdentityComponent.Cast(m_Entity.FindComponent(CharacterIdentityComponent));
				if (charIdentity && charIdentity.GetIdentity())
					m_sName = charIdentity.GetIdentity().GetName();
				else 
					m_sName = "No character identity!";
			}
		}
		
		name = m_sName;
		nameParams.Copy(m_aNameParams);
	}
	
	BaseCompartmentSlot GetVehicleCompartment()
	{
		return m_VehicleCompartment;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set what is the nametag position attached to
	//! \param pos is the new tag position
	//! \param gradualChange controls whether the position change will be instant (false) or gradually lerped
	void SetTagPosition(ENameTagPosition pos, bool gradualChange = true)
	{
		m_eAttachedTo = pos;
		m_fTimeSlicePosChange = 0;
		m_vTagWorldPosLast = m_vTagWorldPos;
		
		if (!gradualChange)
			m_eAttachedToLast = pos;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set group state if it matches with current player group
	void SetGroup(SCR_AIGroup group)
	{
		if (group)
		{
			m_iGroupID = group.GetGroupID();
			if (m_bIsCurrentPlayer)
			{
				m_NTDisplay.CleanupAllTags();	// cleanup other tags because we need to compare groups again
				return;
			}
		
			if (m_NTDisplay.m_CurrentPlayerTag.m_iGroupID == m_iGroupID)
			{
				if (group.IsPlayerLeader(m_iPlayerID))
				{
					ActivateEntityState(ENameTagEntityState.GROUP_LEADER);
				}
				else
				{
					ActivateEntityState(ENameTagEntityState.GROUP_MEMBER);
					DeactivateEntityState(ENameTagEntityState.GROUP_LEADER);
				}
			}
		}
		else
		{
			m_iGroupID = -1;
			if (m_eEntityStateFlags & ENameTagEntityState.GROUP_MEMBER)
				DeactivateEntityState(ENameTagEntityState.GROUP_MEMBER);
			
			if (m_eEntityStateFlags & ENameTagEntityState.GROUP_LEADER)
				DeactivateEntityState(ENameTagEntityState.GROUP_LEADER);
			
			if (m_bIsCurrentPlayer)
				m_NTDisplay.CleanupAllTags();
		}
		
		if (m_VehicleParent && m_VehicleParent.m_MainTag == this)	// update vehicle parent as well if this tag is its main tag
			m_VehicleParent.m_Flags |= ENameTagFlags.NAME_UPDATE;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set AI_SUBORDINATE if it falls under current players command
	void CheckAISubordinate(IEntity entity)
	{
		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(entity);
		if (!entity)
			return;
		
		SCR_GroupsManagerComponent groupManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupManager)
			return;
		
		int playerID = GetGame().GetPlayerController().GetPlayerId();
		SCR_AIGroup playerGroup = groupManager.GetPlayerGroup(playerID);
		
		if (!playerGroup || !playerGroup.IsAIControlledCharacterMember(character))
		{
			DeactivateEntityState(ENameTagEntityState.AI_SUBORDINATE);
			return;
		}
		
		ActivateEntityState(ENameTagEntityState.AI_SUBORDINATE);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Add this tag as occupant to a vehicle tag
	protected void AddAsVehicleOccupant(IEntity vehicle, BaseCompartmentSlot slot, ENameTagFlags flag)
	{
		m_Flags |= flag;
		m_VehicleCompartment = slot;
		m_NTDisplay.OnNewVehicleOccupant(vehicle, this);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Remove this tag as occupant to a vehicle tag
	void RemoveVehicleOccupant(IEntity vehicle)
	{
		m_NTDisplay.OnLeaveVehicleOccupant(vehicle, this);
		m_VehicleCompartment = null;
		m_Flags &= ~ENameTagFlags.VEHICLE;
	}
	
	//------------------------------------------------------------------------------------------------
	//! SCR_CompartmentAccessComponent event
	void OnVehicleEntered( IEntity vehicle, BaseCompartmentManagerComponent manager, int mgrID, int slotID )
	{	
		BaseCompartmentSlot compSlot = manager.FindCompartment(slotID, mgrID);
		if (!compSlot)
			return;
	
		IEntity occupant = compSlot.GetOccupant();	// only add if the added entity is the owner of this tag
		if (occupant != m_Entity)
			return;
		
		while (vehicle)		// compartments such as turrets have vehicles as parents
		{
			if (Vehicle.Cast(vehicle))
			{
				AddAsVehicleOccupant(vehicle, compSlot, ENameTagFlags.VEHICLE);
				break;
			}
			else if (Turret.Cast(vehicle) && !vehicle.GetParent())	// f.e. deployed machinegun position can be a turret without parent vehicle
			{
				m_Flags |= ENameTagFlags.TURRET;
				break;
			}
			else
				vehicle = vehicle.GetParent();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! SCR_CompartmentAccessComponent event
	void OnVehicleLeft( IEntity vehicle, BaseCompartmentManagerComponent manager, int mgrID, int slotID )
	{		
		BaseCompartmentSlot compSlot = manager.FindCompartment(slotID, mgrID);
		
		if (compSlot != m_VehicleCompartment)			// only remove if the comp slot subject is this tag's compartment
			return;
				
		while (vehicle)
		{
			if (Vehicle.Cast(vehicle))
			{
				RemoveVehicleOccupant(vehicle);
				break;
			}
			else if (Turret.Cast(vehicle) && !vehicle.GetParent())	// f.e. deployed machinegun position can be a turret without parent vehicle
			{
				m_Flags &= ~ENameTagFlags.TURRET;
				break;
			}
			else 
				vehicle = vehicle.GetParent();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! VoNComponent event, only used for "Current player" tag
	void OnReceivedVON(int playerId, BaseTransceiver receiver, int frequency, float quality)
	{				
		IEntity character = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
		if (!character)
			return;
		
		SCR_NameTagData data = m_NTDisplay.GetEntityNameTag(character);
		if (data)
		{
			data.ActivateEntityState(ENameTagEntityState.VON);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! SCR_CharacterController event
	void OnLifeStateChanged(ECharacterLifeState previousLifeState, ECharacterLifeState newLifeState)
	{
		if (newLifeState == ECharacterLifeState.INCAPACITATED)
			ActivateEntityState(ENameTagEntityState.UNCONSCIOUS);
		else
			DeactivateEntityState(ENameTagEntityState.UNCONSCIOUS);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Cleanup
	//! \param removeFromArray determines whether the tag is removed from main array, this is not desired when entrire array is being cleaned up
	void Cleanup(bool removeFromArray = true)
	{
		m_Flags |= ENameTagFlags.DELETED;
		
		if (m_NTDisplay)
			m_NTDisplay.CleanupTag(this, removeFromArray);
		
		ScriptCallQueue queue = GetGame().GetCallqueue();	// in case this was already set for timed cleanup
		if (queue)
			queue.Remove(Cleanup);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Initial check of active states for JIP
	//!	/return false if invalid/should be cleaned up
	protected bool UpdateEntityStateFlags()
	{
		m_Flags = ENameTagFlags.DISABLED | ENameTagFlags.NAME_UPDATE;	// this has a default flag because if tag never reaches a visibile state, it needs a disable flag for clean up
				
		if (m_GroupManager)
		{
			SCR_AIGroup group = m_GroupManager.GetPlayerGroup(m_iPlayerID);
			SetGroup(group);
		}
		
		SCR_CompartmentAccessComponent compartmentAccess = SCR_CompartmentAccessComponent.Cast(m_Entity.FindComponent(SCR_CompartmentAccessComponent));	// in vehicle
		if (compartmentAccess && compartmentAccess.IsInCompartment())
		{
			IEntity vehicle;
			BaseCompartmentSlot compSlot = compartmentAccess.GetCompartment();
			if (compSlot)
				vehicle = compSlot.GetOwner();
			
			while (vehicle)		// compartments such as turrets have vehicles as parents
			{
				if (Vehicle.Cast(vehicle))
				{
					AddAsVehicleOccupant(vehicle, compSlot, ENameTagFlags.VEHICLE);
					break;
				}
				else if (Turret.Cast(vehicle) && !vehicle.GetParent())	// f.e. deployed machinegun position can be a turret without parent vehicle
				{
					m_Flags |= ENameTagFlags.TURRET;
					break;
				}
				else 
				{
					vehicle = vehicle.GetParent();
				}
			}
		}	
				
		if (m_CharController && m_CharController.IsUnconscious())
			ActivateEntityState(ENameTagEntityState.UNCONSCIOUS);
			
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Update tag position
	void UpdateTagPos()
	{
		vector matPos[4];
		Animation anim = m_Entity.GetAnimation();
		anim.GetBoneMatrix(m_iSpineBone, matPos);
		m_vEntWorldPos = m_Entity.CoordToParent(matPos[3]);
		anim.GetBoneMatrix(m_iHeadBone, matPos);
		m_vEntHeadPos = m_Entity.CoordToParent(matPos[3]);
		
		if (m_eAttachedTo == ENameTagPosition.HEAD)
		{
			m_vTagWorldPos = m_vEntHeadPos + HEAD_OFFSET;
		}
		else if (m_eAttachedTo == ENameTagPosition.BODY)
		{
			m_vTagWorldPos = m_vEntWorldPos + BODY_OFFSET;
		}

		if (m_eType != ENameTagEntityType.PLAYER && GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(m_Entity) > 0)
		{
			m_Flags |= ENameTagFlags.ENT_TYPE_UPDATE;
			m_Flags |= ENameTagFlags.NAME_UPDATE;
		}
	}
		
	//------------------------------------------------------------------------------------------------
	//! Init default tag data
	protected void InitDefaults()
	{
		m_eEntityStateFlags = ENameTagEntityState.HIDDEN | ENameTagEntityState.DEFAULT;
	 	m_ePriorityEntityState = ENameTagEntityState.HIDDEN;
		m_eAttachedTo = ENameTagPosition.HEAD;
		m_eAttachedToLast = ENameTagPosition.HEAD;
		
		m_iZoneID = -1;
		m_iGroupID = -1;
		m_iPlayerID = -1;
		m_fTimeSliceUpdate = 1.0;
		m_fTimeSliceVON = 0;
		m_fTimeSlicePosChange = 0;
		m_fTimeSliceCleanup = 0;
		m_fTimeSliceFade = 0;
		m_fTimeSliceVisibility = 0;
		m_fDistance = 0;
		m_fOpacityFade = 1;
		m_sName = string.Empty;
		m_aNameParams = {};
	}
	
	//------------------------------------------------------------------------------------------------
	//! Init configurable data
	protected void InitData(SCR_NameTagConfig config)
	{
		// configurables
		if (m_NTDisplay)
		{
			SCR_NameTagRulesetBase ruleset = config.m_aVisibilityRuleset;
			
			m_fDeadEntitiesCleanup = ruleset.m_fDeadEntitiesCleanup;
			
			if (ruleset.m_fTagFadeTime == 0)
				m_fTagFadeSpeed = 0;
			else
				m_fTagFadeSpeed = 1/ruleset.m_fTagFadeTime; // convert multiplier to seconds
		}
		
		UpdateEntityType();
		
		if (ChimeraCharacter.Cast(m_Entity))
		{				
			// Vehicle enter/leave event
			SCR_CompartmentAccessComponent accessComp = SCR_CompartmentAccessComponent.Cast( m_Entity.FindComponent(SCR_CompartmentAccessComponent) );
			if (accessComp)
			{
				accessComp.GetOnCompartmentEntered().Insert(OnVehicleEntered);
				accessComp.GetOnCompartmentLeft().Insert(OnVehicleLeft);
			}
		}

		if (m_Entity)
		{
			
			m_iSpineBone = m_Entity.GetAnimation().GetBoneIndex(SPINE_BONE);
			m_iHeadBone = m_Entity.GetAnimation().GetBoneIndex(HEAD_BONE);
		}
		
		GetName(m_sName, m_aNameParams);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Init nametag data class, the widget is held and reused, data has to be reinitialized 
	//! \param display is nametag info display
	//! \param entity is nametag subject
	//! \return false if nametag is not valid and should be cleaned up
	bool InitTag(SCR_NameTagDisplay display, IEntity entity, SCR_NameTagConfig config, bool IsCurrentPlayer = false)
	{		
		m_bIsCurrentPlayer = IsCurrentPlayer;
		m_Entity = entity;
		m_NTDisplay = display;
		
		ChimeraCharacter ent = ChimeraCharacter.Cast(entity);
		if (ent)
		{
			m_CharController = SCR_CharacterControllerComponent.Cast(ent.FindComponent(SCR_CharacterControllerComponent));
			if (!m_CharController || m_CharController.IsDead())
				return false;	
			
			m_CharController.m_OnLifeStateChanged.Insert(OnLifeStateChanged);
			
			if (m_bIsCurrentPlayer)		// we only need VON received event for current player to set VON status icons
			{
				SCR_VoNComponent vonComp = SCR_VoNComponent.Cast( ent.FindComponent(SCR_VoNComponent) );
				if (vonComp)
					vonComp.m_OnReceivedVON.Insert(OnReceivedVON);
			}
		}
			
		InitDefaults();			
		InitData(config);
				
		if (!UpdateEntityStateFlags())
			return false;
	
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Uninitialize class when its being moved to unsued tags in order to be reused
	void ResetTag()
	{
		if (m_CharController)
			m_CharController.m_OnLifeStateChanged.Remove(OnLifeStateChanged);
		
		if (m_bIsCurrentPlayer && m_Entity)
		{
			SCR_VoNComponent vonComp = SCR_VoNComponent.Cast( m_Entity.FindComponent(SCR_VoNComponent) );
			if (vonComp)
				vonComp.m_OnReceivedVON.Remove(OnReceivedVON);
		}
		
		if (ChimeraCharacter.Cast(m_Entity))
		{				
			// Vehicle enter/leave event
			SCR_CompartmentAccessComponent accessComp = SCR_CompartmentAccessComponent.Cast( m_Entity.FindComponent(SCR_CompartmentAccessComponent) );
			if (accessComp)
			{
				accessComp.GetOnCompartmentEntered().Remove(OnVehicleEntered);
				accessComp.GetOnCompartmentLeft().Remove(OnVehicleLeft);
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_NameTagData(ResourceName layout, Widget rootWidget)
	{				
		m_NameTagWidget = GetGame().GetWorkspace().CreateWidgets(layout, rootWidget);
		if (m_NameTagWidget)	
			m_NameTagWidget.SetVisible(false);
		
		m_GroupManager = SCR_GroupsManagerComponent.Cast(GetGame().GetGameMode().FindComponent(SCR_GroupsManagerComponent));
		m_PossessingManager = SCR_PossessingManagerComponent.Cast(GetGame().GetGameMode().FindComponent(SCR_PossessingManagerComponent));
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_NameTagData()
	{
		ResetTag();
	}
};
