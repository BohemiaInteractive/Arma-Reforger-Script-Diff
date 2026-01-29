class SCR_ChangeComparmentAction : SCR_CompartmentUserAction
{
	[Attribute("0", desc: "If enabled 'Compartment Slot Name' will be ignored and the Pilot compartment will be used")]
	protected bool m_bUsePilotCompartment;

	[Attribute("SeatsRear", desc: "Name of slot that holds the compartment Prefab we want to access. Found on: SlotManagerComponent.")]
	protected string m_sCompartmentSlotName;

	protected ChimeraCharacter m_Character;

	// Store the last found free compartment slot
	protected BaseCompartmentSlot m_NonOccupiedSlot;

	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		m_Character = ChimeraCharacter.Cast(user);

		// We cannot be pilot nor interior, if we are not seated in vehicle at all.
		if (!m_Character.IsInVehicle())
			return false;

		// If the compartment has never been checked before, do it first
		if (!m_NonOccupiedSlot)
			m_NonOccupiedSlot = GetFreeCompartmentSlot();

		// If no free compartment slot has been found, don't show this action
		if (!m_NonOccupiedSlot)
			return false;

		// Check if the last stored free compartment this is occupied
		if (m_NonOccupiedSlot.GetOccupant())
			m_NonOccupiedSlot = GetFreeCompartmentSlot();

		return m_NonOccupiedSlot && !m_NonOccupiedSlot.GetOccupant();
	}

	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		if (!pOwnerEntity || !pUserEntity)
			return;

		m_Character = ChimeraCharacter.Cast(pUserEntity);
		if (!m_Character)
			return;

		BaseCompartmentSlot targetCompartment = m_NonOccupiedSlot;
		if (!targetCompartment)
			return;

		CompartmentAccessComponent compartmentAccess = m_Character.GetCompartmentAccessComponent();
		if (!compartmentAccess)
			return;

		if (!compartmentAccess.GetInVehicle(pOwnerEntity, targetCompartment, false, GetRelevantDoorIndex(pUserEntity), ECloseDoorAfterActions.INVALID, false))
			return;

		super.PerformAction(pOwnerEntity, pUserEntity);
	}

	//------------------------------------------------------------------------------------------------
	override int GetRelevantDoorIndex(IEntity caller)
	{
		if (m_iCompartmentDoorInfoIndex != -1)
			return m_iCompartmentDoorInfoIndex;

		PlayerController playerController = GetGame().GetPlayerController();
		if (!playerController || playerController.GetControlledEntity() != caller)
			return -1;

		InteractionHandlerComponent interactionHandler = InteractionHandlerComponent.Cast(playerController.FindComponent(InteractionHandlerComponent));
		if (!interactionHandler)
			return -1;

		UserActionContext actionCtx = interactionHandler.GetCurrentContext();
		if (actionCtx)
		{
			if (m_NonOccupiedSlot)
				return m_NonOccupiedSlot.PickDoorIndexForPoint(actionCtx.GetOrigin());
			else
				return GetFreeCompartmentSlot().PickDoorIndexForPoint(actionCtx.GetOrigin());
		}

		return -1;
	}

	//------------------------------------------------------------------------------------------------
	//! Check the defined Compartment for any free slots
	//! \return BaseCompartmentSlot if free slot was found, null otherwise
	protected BaseCompartmentSlot GetFreeCompartmentSlot()
	{
		Vehicle vehicle = Vehicle.Cast(GetOwner().GetRootParent());
		if (!vehicle)
			return null;

		SCR_BaseCompartmentManagerComponent compartmentManager;

		if (m_bUsePilotCompartment)
		{
			PilotCompartmentSlot pilotCompartmentSlot = GetPilotCompartmentSlot();
			if (pilotCompartmentSlot)
				return pilotCompartmentSlot;

			compartmentManager = SCR_BaseCompartmentManagerComponent.Cast(vehicle.FindComponent(SCR_BaseCompartmentManagerComponent));
		}
		else
		{
			SlotManagerComponent slotManagerComp = SlotManagerComponent.Cast(vehicle.FindComponent(SlotManagerComponent));
			if (!slotManagerComp)
				return null;

			EntitySlotInfo slotInfo = slotManagerComp.GetSlotByName(m_sCompartmentSlotName);
			if (!slotInfo)
				return null;

			IEntity entity = slotInfo.GetAttachedEntity();
			
			if (entity)
				compartmentManager = SCR_BaseCompartmentManagerComponent.Cast(entity.FindComponent(SCR_BaseCompartmentManagerComponent));
		}

		if (!compartmentManager)
			return null;

		array<BaseCompartmentSlot> outCompartments = {};
		compartmentManager.GetCompartments(outCompartments);

		// Get the first CompartmentSlot that is not occupied
		foreach (BaseCompartmentSlot comp : outCompartments)
		{
			if (!comp.GetOccupant())
				return comp;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! Get the Pilot compartment
	//! \return PilotCompartmentSlot if player sits in the pilot slot of the vehicle, null otherwise
	protected PilotCompartmentSlot GetPilotCompartmentSlot()
	{
		CompartmentAccessComponent compartmentAccess = m_Character.GetCompartmentAccessComponent();
		if (!compartmentAccess)
			return null;

		// Character is in compartment
		// that belongs to owner of this action
		BaseCompartmentSlot slot = compartmentAccess.GetCompartment();
		if (!slot)
			return null;

		return PilotCompartmentSlot.Cast(slot);
	}
}
