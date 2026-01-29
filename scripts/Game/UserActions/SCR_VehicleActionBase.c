class SCR_VehicleActionBase : ScriptedUserAction
{
	//! Should action behave as a toggle
	[Attribute(uiwidget: UIWidgets.CheckBox, defvalue: "1", desc: "Should action behave as a toggle")]
	protected bool m_bIsToggle;

	//! Target state of the action, ignored if toggle
	[Attribute(uiwidget: UIWidgets.CheckBox, defvalue: "1", desc: "Target state of the action, ignored if toggle")]
	protected bool m_bTargetState;

	//! Description of action to toggle on
	[Attribute(uiwidget: UIWidgets.Auto, defvalue: "#AR-UserAction_State_On", desc: "Description of action to toggle on")]
	protected string m_sActionStateOn;

	//! Description of action to toggle off
	[Attribute(uiwidget: UIWidgets.Auto, defvalue: "#AR-UserAction_State_Off", desc: "Description of action to toggle off")]
	protected string m_sActionStateOff;

	//! Available only for entities seated in pilot compartment
	[Attribute(uiwidget: UIWidgets.CheckBox, defvalue: "1", desc: "Available only for entities seated in pilot compartment")]
	protected bool m_bPilotOnly;

	//! Available only for entities seated in a vehicle
	[Attribute(uiwidget: UIWidgets.CheckBox, desc: "Available only for entities seated in a vehicle")]
	protected bool m_bInteriorOnly;
	
	[Attribute(desc: "When anything is defined in here, this action will only be visible if the compartmet section of the compartment the player in in, is defined in here.")]
	protected ref array<int> m_aDefinedCompartmentSectionsOnly;
	
	[Attribute(desc: "When aynthing is defined in here, this action won't be visible if the compartment section of the compartment the player is in, is defined in here.")]
	protected ref array<int> m_aExcludeDefinedCompartmentSections;

	protected CompartmentControllerComponent m_VehicleController; // Commonly used by inheriting actions

	//------------------------------------------------------------------------------------------------
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		super.Init(pOwnerEntity, pManagerComponent);

		m_VehicleController = CompartmentControllerComponent.Cast(pOwnerEntity.FindComponent(CompartmentControllerComponent));
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		// See if character is in vehicle
		ChimeraCharacter character = ChimeraCharacter.Cast(user);
		if (!character)
			return false;

		// We cannot be pilot nor interior, if we are not seated in vehicle at all.
		if (!character.IsInVehicle())
			return !(m_bInteriorOnly || m_bPilotOnly);

		// See if character is in "this" (owner) vehicle
		CompartmentAccessComponent compartmentAccess = character.GetCompartmentAccessComponent();
		if (!compartmentAccess)
			return false;

		// Character is in compartment
		// that belongs to owner of this action
		BaseCompartmentSlot slot = compartmentAccess.GetCompartment();
		if (!slot)
			return false;

		// Check pilot only condition
		if (m_bPilotOnly)
		{
			if (!PilotCompartmentSlot.Cast(slot))
				return false;

			Vehicle vehicle = Vehicle.Cast(GetOwner().GetRootParent());
			if (vehicle && vehicle.GetPilotCompartmentSlot() != slot)
				return false;
		}

		// Check interior only condition
		if (m_bInteriorOnly && slot.GetOwner().GetRootParent() != GetOwner().GetRootParent())
			return false;
		
		int compartmentSection = slot.GetCompartmentSection();
		
		if (!m_aDefinedCompartmentSectionsOnly.IsEmpty() && !m_aDefinedCompartmentSectionsOnly.Contains(compartmentSection))
			return false;
			
		if (!m_aExcludeDefinedCompartmentSections.IsEmpty() && m_aExcludeDefinedCompartmentSections.Contains(compartmentSection))
			return false;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{
		return m_bIsToggle || (GetState() != m_bTargetState);
	}

	//------------------------------------------------------------------------------------------------
	//! Current state of the feature
	bool GetState()
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Script to change state of the feature
	void SetState(bool enable);

	//------------------------------------------------------------------------------------------------
	//! By default toggle the current state of the interaction
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		if (m_bIsToggle)
			SetState(!GetState());
		else
			SetState(m_bTargetState);
	}

	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		UIInfo uiInfo = GetUIInfo();
		string prefix;
		if (uiInfo)
			prefix = uiInfo.GetName();

		if (!prefix.IsEmpty())
			prefix += " ";

		if (GetState())
			outName = prefix + m_sActionStateOff;
		else
			outName = prefix + m_sActionStateOn;

		return true;
	}
}
