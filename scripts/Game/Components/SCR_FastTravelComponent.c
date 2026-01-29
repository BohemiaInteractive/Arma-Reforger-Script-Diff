[EntityEditorProps(category: "GameScripted/FastTravel", description: "Handles client > server communication for Fast travel. Should be attached to PlayerController.")]
class SCR_FastTravelComponentClass : ScriptComponentClass
{
}

class SCR_FastTravelComponent : ScriptComponent
{
	protected PlayerController m_PlayerController;

	protected RplId m_iDestinationId = RplId.Invalid();

	protected string m_sDestinationName;

	protected bool m_bIsDestinationValid;
	protected bool m_bDisableAfterUse;

	protected vector m_vTeleportCoordinates;

	[Attribute("300", desc: "Minimum delay between fast travels (seconds).", params: "0 inf 1")];
	protected int m_iCooldown;

	[Attribute("250", desc: "How far away from the target destination will player get teleported.", params: "0 inf 1")];
	protected int m_iDistanceToTarget;

	[Attribute("IEntity", desc: "Allowed class name of destination entity (inherited classes are also allowed).")];
	protected string m_sDestinationType;

	[RplProp(condition: RplCondition.OwnerOnly)]
	protected WorldTimestamp m_fNextTravelAvailableAt;

	static const int SPAWNING_RADIUS = 10;

	static const float FADE_DURATION = 1.5;
	static const float BLACKSCREEN_DURATION = 1.0;
	static const float CLOSE_MAP_DELAY = 0.75;

	//------------------------------------------------------------------------------------------------
	//! \return
	static SCR_FastTravelComponent GetLocalInstance()
	{
		PlayerController pc = GetGame().GetPlayerController();

		if (!pc)
			return null;

		return SCR_FastTravelComponent.Cast(pc.FindComponent(SCR_FastTravelComponent));
	}

	//------------------------------------------------------------------------------------------------
	//! Destination must be a replicated item with valid RplId
	void SetDestination(RplId id, string name)
	{
		m_iDestinationId = id;
		m_sDestinationName = name;
		m_bIsDestinationValid = Replication.FindItem(id) != null;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	string GetDestinationName()
	{
		return m_sDestinationName;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	int GetCooldown()
	{
		return m_iCooldown;
	}

	//------------------------------------------------------------------------------------------------
	//!
	void DisableAfterUse()
	{
		m_bDisableAfterUse = true;
	}

	//------------------------------------------------------------------------------------------------
	//! Toggle fast travel destination selection from the map.
	//! \param[in] enable
	//! \param[in] disableAfterUse
	static void ToggleMapDestinationSelection(bool enable, bool disableAfterUse = true)
	{
		SCR_FastTravelComponent comp = GetLocalInstance();

		if (!comp)
			return;

		if (enable)
			SCR_NotificationsComponent.SendLocal(ENotification.FASTTRAVEL_AVAILABLE);
		else
			SCR_NotificationsComponent.SendLocal(ENotification.FASTTRAVEL_UNAVAILABLE);

		SCR_MapEntity.GetOnSelectionChanged().Remove(comp.OnSelectionChanged);

		if (!enable)
			return;

		if (disableAfterUse)
			comp.DisableAfterUse();

		SCR_MapEntity.GetOnSelectionChanged().Insert(comp.OnSelectionChanged);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] item
	void OnSelectionChanged(MapItem item)
	{
		SCR_MapEntity mapEntity = SCR_MapEntity.GetMapInstance();

		if (!mapEntity)
			return;

		MapItem selectedItem = item;

		if (!selectedItem)
			selectedItem = mapEntity.GetHoveredItem();

		if (!selectedItem)
			return;

		IEntity entity = selectedItem.Entity();

		if (!entity || !entity.IsInherited(m_sDestinationType.ToType()))
			return;

		RplId id = FindDestinationId(entity);

		if (!id.IsValid())
				return;

		IEntity player = m_PlayerController.GetControlledEntity();

		if (player && mapEntity.IsOpen())
		{
			SCR_GadgetManagerComponent comp = SCR_GadgetManagerComponent.GetGadgetManager(player);

			if (comp)
				comp.RemoveHeldGadget();
		}

		SetDestination(id, string.Empty);
		FastTravel();
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] entity
	//! \return
	RplId FindDestinationId(IEntity entity)
	{
		RplId id = Replication.FindId(entity);

		if (id.IsValid())
			return id;

		RplComponent rpl = RplComponent.Cast(entity.FindComponent(RplComponent));

		if (!rpl)
			return RplId.Invalid();

		return rpl.Id();
	}

	//------------------------------------------------------------------------------------------------
	//!
	void FastTravel()
	{
		if (!m_PlayerController)
			return;

		Rpc(RpcAsk_FastTravel, m_iDestinationId);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_FastTravel(RplId destinationId)
	{
		if (!m_PlayerController)
			return;

		IEntity player = m_PlayerController.GetControlledEntity();

		if (!player)
			return;

		IEntity target = GetEntityByDestinationId(destinationId);

		if (!target)
			return;

		if (!ServerSanityCheck(target))
			return;

		ChimeraWorld world = GetGame().GetWorld();

		m_fNextTravelAvailableAt = world.GetServerTimestamp().PlusSeconds(GetCooldown());
		Replication.BumpMe();
		Rpc(RpcDo_FastTravel, CalculateDestinationVector(target));
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] id
	//! \return
	IEntity GetEntityByDestinationId(RplId id)
	{
		Managed destination = Replication.FindItem(id);

		if (!destination)
			return null;

		IEntity target;

		// We need to get target entity from the replicated item
		if (destination.IsInherited(IEntity))
			target = IEntity.Cast(destination);
		else if (destination.IsInherited(ScriptComponent))
			target = ScriptComponent.Cast(destination).GetOwner();
		else if (destination.IsInherited(BaseRplComponent))
			target = BaseRplComponent.Cast(destination).GetEntity();

		return target;
	}

	//------------------------------------------------------------------------------------------------
	//! Sanity check before the actual teleport
	//! \param[in] target
	protected bool ServerSanityCheck(notnull IEntity target)
	{
		ChimeraWorld world = GetGame().GetWorld();

		return (!m_fNextTravelAvailableAt || m_fNextTravelAvailableAt.LessEqual(world.GetServerTimestamp()));
	}

	//------------------------------------------------------------------------------------------------
	WorldTimestamp GetNextTransportTimestamp()
	{
		return m_fNextTravelAvailableAt;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] timestamp
	void SetNextTransportTimestamp(WorldTimestamp timestamp)
	{
		m_fNextTravelAvailableAt = timestamp;
		Replication.BumpMe();
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void RpcDo_FastTravel(vector position)
	{
		if (m_bDisableAfterUse)
			ToggleMapDestinationSelection(false);

		m_vTeleportCoordinates = position;

		// Wait for map to close and fade from black
		GetGame().GetCallqueue().CallLater(FadeOut, CLOSE_MAP_DELAY * 1000);
	}

	//------------------------------------------------------------------------------------------------
	protected void FadeOut()
	{
		SCR_ScreenEffectsManager manager = SCR_ScreenEffectsManager.GetScreenEffectsDisplay();
		if (manager)
		{
			SCR_FadeInOutEffect fade = SCR_FadeInOutEffect.Cast(manager.GetEffect(SCR_FadeInOutEffect));
			if (fade)
				fade.FadeOutEffect(true, FADE_DURATION);
		}

		// Wait for fade to black
		GetGame().GetCallqueue().CallLater(Teleport, (FADE_DURATION + BLACKSCREEN_DURATION) * 1000);
	}

	//------------------------------------------------------------------------------------------------
	protected void Teleport()
	{
		SCR_ChimeraCharacter player = SCR_ChimeraCharacter.Cast(m_PlayerController.GetControlledEntity());

		if (player && !player.IsInVehicle() && !player.GetCharacterController().IsDead())
		{
			SCR_Global.TeleportLocalPlayer(m_vTeleportCoordinates, SCR_EPlayerTeleportedReason.FAST_TRAVEL);
			SCR_NotificationsComponent.SendLocal(ENotification.FASTTRAVEL_DONE);
		}

		SCR_ScreenEffectsManager manager = SCR_ScreenEffectsManager.GetScreenEffectsDisplay();
		if (manager)
		{
			SCR_FadeInOutEffect fade = SCR_FadeInOutEffect.Cast(manager.GetEffect(SCR_FadeInOutEffect));
			if (fade)
				fade.FadeOutEffect(false, FADE_DURATION);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Find coordinates for player teleport some distance away from the target, azimuth relative to previous player location
	//! \param[in] destination
	protected vector CalculateDestinationVector(notnull IEntity destination)
	{
		return CalculateDestinationVector(destination.GetOrigin());
	}

	//------------------------------------------------------------------------------------------------
	protected vector CalculateDestinationVector(vector targetOrigin)
	{
		IEntity player = m_PlayerController.GetControlledEntity();

		if (!player)
			return vector.Zero;

		vector playerOrigin = m_PlayerController.GetControlledEntity().GetOrigin();
		vector newPlayerOrigin;

		newPlayerOrigin[1] = playerOrigin[1];

		float angle = Math.Atan2(playerOrigin[2] - targetOrigin[2], playerOrigin[0] - targetOrigin[0]);
		newPlayerOrigin[0] = targetOrigin[0] + m_iDistanceToTarget * Math.Cos(angle);
		newPlayerOrigin[2] = targetOrigin[2] + m_iDistanceToTarget * Math.Sin(angle);
		newPlayerOrigin[1] = SCR_TerrainHelper.GetTerrainY(newPlayerOrigin);

		SCR_WorldTools.FindEmptyTerrainPosition(newPlayerOrigin, newPlayerOrigin, SPAWNING_RADIUS);

		return newPlayerOrigin;
	}

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		SetEventMask(owner, EntityEvent.INIT);
	}

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		m_PlayerController = PlayerController.Cast(owner);

		if (!m_PlayerController)
			Print("SCR_FastTravelComponent must be attached to PlayerController!", LogLevel.ERROR);
	}

	//------------------------------------------------------------------------------------------------
	// destructor
	void ~SCR_FastTravelComponent()
	{
		SCR_MapEntity.GetOnSelectionChanged().Remove(OnSelectionChanged);
	}
}
