class SCR_AIChangeCompartment : AITaskScripted
{
	protected static const int COMPARTMENT_INIT 			= 0;
	protected static const int COMPARTMENT_WITH_CORPSE 		= 1;
	protected static const int COMPARTMENT_EMPTYING 		= 2;
	protected static const int COMPARTMENT_EMPTY 			= 3;
	protected static const int COMPARTMENT_CHANGING			= 4;
	protected static const int COMPARTMENT_CHANGED			= 5;
	protected static const int NO_COMPARTMENT_AVAILABLE		= 6;
	
	[Attribute("0", UIWidgets.EditBox, "Which compartment type owner switches to", "", ParamEnumArray.FromEnum(ECompartmentType))]
	ECompartmentType m_eCompartmentType;
	
	protected static const string TARGET_VEHICLE_PORT = "TargetVehicle";
	protected static const string COMPARTMENT_TYPE_PORT = "CompartmentType";
	protected static const string TELEPORT_IN_PORT = "TeleportInside";
	
	protected static ref TStringArray s_aVarsIn = { TARGET_VEHICLE_PORT, COMPARTMENT_TYPE_PORT, TELEPORT_IN_PORT };
	override TStringArray GetVariablesIn() { return s_aVarsIn; }
	
	protected SCR_CompartmentAccessComponent m_compartmentAccess;
	protected SCR_CompartmentAccessComponent m_compartmentAccessOccupant;
	protected ChimeraCharacter m_target;
	protected int m_compartmentSwitchState;
	protected BaseCompartmentSlot m_compartmentSlot;
	protected IEntity m_vehicle;
	protected bool m_TeleportInside;
	
	//----------------------------------------------------------------------------------------------------------------------------------------
	override void OnInit(AIAgent owner)
	{
		m_compartmentSwitchState = COMPARTMENT_INIT;
		m_compartmentSlot = null;
		m_target = ChimeraCharacter.Cast(owner.GetControlledEntity());
		if (m_target)
			m_compartmentAccess = SCR_CompartmentAccessComponent.Cast(m_target.GetCompartmentAccessComponent());
	}
	
	//----------------------------------------------------------------------------------------------------------------------------------------
	override void OnEnter(AIAgent owner)
	{
		if (!GetVariableIn(TARGET_VEHICLE_PORT, m_vehicle))
			m_vehicle = m_compartmentAccess.GetVehicleIn(m_target);
		
		if (!GetVariableIn(TELEPORT_IN_PORT, m_TeleportInside))
			m_TeleportInside = false;
		
		ECompartmentType compartmentType;
		if (GetVariableIn(COMPARTMENT_TYPE_PORT,compartmentType))
			m_eCompartmentType = compartmentType;
	}
	
	//----------------------------------------------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		if (!m_target || !m_compartmentAccess)
			return ENodeResult.FAIL;
		
		return UpdateCompartmentSwitchState(m_compartmentSwitchState, m_compartmentSwitchState);
	}
	
	//----------------------------------------------------------------------------------------------------------------------------------------
	//! Returns node result and state of the compartment switching process 
	ENodeResult UpdateCompartmentSwitchState(int currentState, out int newState)
	{
		switch (currentState)
		{
			case COMPARTMENT_INIT: 
			{
				if (!m_vehicle)
				{
					newState = NO_COMPARTMENT_AVAILABLE;
					return ENodeResult.FAIL;
				}
				SCR_BaseCompartmentManagerComponent compMan = SCR_BaseCompartmentManagerComponent.Cast(m_vehicle.FindComponent(SCR_BaseCompartmentManagerComponent));
				if (!compMan)
				{
					newState = NO_COMPARTMENT_AVAILABLE;
					return ENodeResult.FAIL;
				}
				
				array<BaseCompartmentSlot> compartmentSlots = {};
				compMan.GetCompartmentsOfType(compartmentSlots, m_eCompartmentType);
				
				foreach (BaseCompartmentSlot slot : compartmentSlots)
				{
					if (slot.IsOccupied())
					{
						ChimeraCharacter occupant = ChimeraCharacter.Cast(slot.GetOccupant());
						if (!occupant)
							continue;
						CharacterControllerComponent contr = occupant.GetCharacterController();
						if (!contr)
							continue;
						if (contr.GetLifeState() == ECharacterLifeState.ALIVE)
							continue;
						m_compartmentAccessOccupant = SCR_CompartmentAccessComponent.Cast(occupant.GetCompartmentAccessComponent());
						if (!m_compartmentAccessOccupant)
							continue;
						newState = COMPARTMENT_WITH_CORPSE;
					}
					else
						newState = COMPARTMENT_EMPTY;
					m_compartmentSlot = slot;
				}
				if(!m_compartmentSlot)
				{
					newState = NO_COMPARTMENT_AVAILABLE;
					return ENodeResult.FAIL;
				}
				return ENodeResult.RUNNING;
			}
			case COMPARTMENT_WITH_CORPSE:
			{
				vector teleportLocation[4];
				m_compartmentAccessOccupant.FindSuitableTeleportLocation(teleportLocation);
				m_compartmentAccessOccupant.GetOutVehicle_NoDoor(teleportLocation, false, false);
				newState = COMPARTMENT_EMPTYING;
				return ENodeResult.RUNNING;
			}
			case COMPARTMENT_EMPTYING:
			{
				if (!m_compartmentSlot.IsOccupied())
					newState = COMPARTMENT_EMPTY;
				return ENodeResult.RUNNING;
			}
			case COMPARTMENT_EMPTY:
			{
				m_compartmentAccess.GetInVehicle(m_vehicle, m_compartmentSlot, m_TeleportInside, -1, ECloseDoorAfterActions.INVALID, false);
				newState = COMPARTMENT_CHANGING;
				return ENodeResult.RUNNING;
			}
			case COMPARTMENT_CHANGING:
			{
				if (m_compartmentSlot.IsOccupied())
				{
					newState = COMPARTMENT_CHANGED;
					return ENodeResult.SUCCESS;
				}
				return ENodeResult.RUNNING;
			}
			default:
				return ENodeResult.RUNNING;	
		}
		return ENodeResult.FAIL;
	}
	
	//----------------------------------------------------------------------------------------------------------------------------------------
	bool EjectOccupant(ChimeraCharacter occupant)
	{
		CompartmentAccessComponent acc = occupant.GetCompartmentAccessComponent();
		return acc.GetOutVehicle(EGetOutType.TELEPORT, -1, ECloseDoorAfterActions.INVALID, false);
	}
	
	//----------------------------------------------------------------------------------------------------------------------------------------
	static override bool VisibleInPalette() { return true; }
	
	//----------------------------------------------------------------------------------------------------------------------------------------
	static override bool CanReturnRunning() { return true; }
	
	//----------------------------------------------------------------------------------------------------------------------------------------
	static override string GetOnHoverDescription()
	{
		return "ChangeCompartment: Changes compartment within the vehicle and keeps running until the character is in the desired slottype. \nIf slot is occupied by corpse, it kicks the corpse out";
	}
};