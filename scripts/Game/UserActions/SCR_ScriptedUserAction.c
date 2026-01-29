void UserActionEventListener(IEntity user, ScriptedUserAction action, EUserActionEvent e);
typedef func UserActionEventListener;

//! A scripted action class having optional logic to check if vehicle is valid
class SCR_ScriptedUserAction : ScriptedUserAction
{
	[Attribute("1", desc: "If action is Continuous and duration is less than 0 than the UI process bar goes to 0 again if it reaches the perform action amount")]
	protected bool m_bLoopAction;

	[Attribute("0.25", desc: "If action is looping this will be used to delay the action after it reaches 100% to prevent the action from instantly looping", params: "0 inf")]
	protected float m_fLoopActionHoldDuration;

	[Attribute("0", desc: "Using this action should automatically lower player weapon")]
	protected bool m_bLowerWeaponOnUse;

	[Attribute("0", desc: "When can the action be shown regarding the vehicle you are in. Note only checked for vehicle that the action is attached to. IGNORE will never check vehicle state", uiwidget: UIWidgets.SearchComboBox, enums: ParamEnumArray.FromEnum(EUserActionInVehicleState))]
	protected EUserActionInVehicleState m_eShownInVehicleState;

	[Attribute("1", desc: "Whether this action can only be performed by occupants of the same vehicle (if in vehicle)", uiwidget: UIWidgets.CheckBox)]
	protected bool m_bSameVehicleOnly;

	protected UserActionContext m_LastUserActionContext; //~ Last user action context when the action was selected. used for getting the action position

	//~ Keeps track of action duration if LoopUpdate is used
	protected float m_fLoopProgress;
	protected float m_fLoopProgressSmoothVelocity;

	//================================================== CAN SHOW ==================================================\\
	override bool CanBeShownScript(IEntity user)
	{
		if (m_eShownInVehicleState == EUserActionInVehicleState.IGNORE)
			return true;

		//~ Only check when character
		ChimeraCharacter character = ChimeraCharacter.Cast(GetOwner());
		if (!character)
		{
			character = ChimeraCharacter.Cast(user);
			if (!character)
				return true;
		}

		//~ Only check when has compartment access
		CompartmentAccessComponent compAccess = character.GetCompartmentAccessComponent();
		if (!compAccess)
			return true;

		//~ Never show when getting in or out vehicle
		if (compAccess.IsGettingIn() || compAccess.IsGettingOut())
			return false;

		//~ Is character in vehicle?
		bool isCharacterInVehicle = character.IsInVehicle();

		//~ Check if correct state
		switch (m_eShownInVehicleState)
		{
			//~ Character should not be in vehicle
			case EUserActionInVehicleState.NOT_IN_VEHICLE :
			{
				if (isCharacterInVehicle)
					return false;

				break;
			}
			//~ Check if character is in current vehicle and is in any position
			case EUserActionInVehicleState.IN_VEHICLE_ANY :
			{
				if (!isCharacterInVehicle)
					return false;

				break;
			}
			//~ Check if character is in current vehicle and pilot
			case EUserActionInVehicleState.IN_VEHICLE_PILOT :
			{
				if (!isCharacterInVehicle || compAccess.GetCompartment().GetType() != ECompartmentType.PILOT)
					return false;

				break;
			}
			//~ Check if character is in current vehicle and cargo
			case EUserActionInVehicleState.IN_VEHICLE_CARGO :
			{
				if (!isCharacterInVehicle || compAccess.GetCompartment().GetType() != ECompartmentType.CARGO)
					return false;

				break;
			}
			//~ Check if character is in current vehicle and turret
			case EUserActionInVehicleState.IN_VEHICLE_TURRET :
			{
				if (!isCharacterInVehicle || compAccess.GetCompartment().GetType() != ECompartmentType.TURRET)
					return false;

				break;
			}
		}

		//~ Disallow actions from being performed from the outside of a vehicle or the other way around, unless they are in the same vehicle (if set)
		if (isCharacterInVehicle && m_bSameVehicleOnly)
			return IsSameVehicleOrNone(character);

		//~ All passed so return true
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected bool IsSameVehicleOrNone(notnull IEntity user)
	{

		// See if user can even access compartments
		CompartmentAccessComponent userCompAccess;

		// Use cached getter for character, but don't hinder logic for non-character users
		ChimeraCharacter userCharacter = ChimeraCharacter.Cast(user);
		if (userCharacter)
			userCompAccess = userCharacter.GetCompartmentAccessComponent();
		else
			userCompAccess = CompartmentAccessComponent.Cast(user.FindComponent(CompartmentAccessComponent));

		if (!userCompAccess)
			return true; // Not really relevant

		IEntity owner = GetOwner();
		CompartmentAccessComponent ownerAccess = CompartmentAccessComponent.Cast(owner.FindComponent(CompartmentAccessComponent));
		if (!ownerAccess)
		{
			// If owner does not have comparment access, perhaps it is a vehicle itself?
			Vehicle vehicle = Vehicle.Cast(owner);
			if (!vehicle)
				return true; // No access, not relevant either

			IEntity userVehicle = userCompAccess.GetVehicleIn(user);
			return userVehicle == vehicle; // Same vehicle
		}

		// If both can access compartments, see if they are within the same vehicle
		IEntity userVehicle = userCompAccess.GetVehicleIn(user);
		IEntity ownerVehicle = ownerAccess.GetVehicleIn(owner);
		return userVehicle == ownerVehicle;
	}

	//================================================== GETTERS ==================================================\\
	/*!
	Get action local position
	Used for such things like playing audio at the correct location on action use
	\return action local position
	*/
	vector GetLocalPositionAction()
	{
		if (!m_LastUserActionContext)
			return vector.Zero;

		return GetOwner().CoordToLocal(m_LastUserActionContext.GetOrigin());
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Get action world position
	Used for such things like playing audio at the correct location on action use
	\return action world position
	*/
	vector GetWorldPositionAction()
	{
		if (!m_LastUserActionContext)
			return GetOwner().GetOrigin();

		return m_LastUserActionContext.GetOrigin();
	}

	//------------------------------------------------------------------------------------------------
	//~ Update for looping actions. Make sure duration is set to less than 0 and perform continues is true.
	//~ Will return true if a loop was successfully completed
	protected bool LoopActionUpdate(float timeSlice)
	{
		float lastLoopProgress = m_fLoopProgress;

		m_fLoopProgress -= timeSlice;

		bool bLoopDone = false;

		//~ Is loop done ?
		if (lastLoopProgress > 0 && m_fLoopProgress <= 0)
			bLoopDone = true; // Loop Done

		// Is hold duration done ?
		if (m_fLoopProgress < -m_fLoopActionHoldDuration)
		{
			// Hold duration (after loop done) is over.
			m_fLoopProgress = Math.AbsFloat(GetActionDuration()) + m_fLoopActionHoldDuration + m_fLoopProgress;
		}

		return bLoopDone;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	\return progress value for action progressbar UI.
	*/
	override float GetActionProgressScript(float fProgress, float timeSlice)
	{
		fProgress = fProgress + timeSlice;

		if (!IsActionLooping())
			return fProgress;

		float actionDuration = Math.AbsFloat(GetActionDuration());
		float totalDuration = actionDuration + m_fLoopActionHoldDuration;

		fProgress = Math.Min(Math.Mod(fProgress, totalDuration), actionDuration); // updated every frame
		float fTargetProgress = Math.Min(totalDuration - (m_fLoopProgress + m_fLoopActionHoldDuration), actionDuration); // update every fixed frame

		return Math.SmoothCD(fProgress, fTargetProgress, m_fLoopProgressSmoothVelocity, 0.01, 1000, timeSlice);
	}

	//------------------------------------------------------------------------------------------------
	/*!
	\return True if action process bar should reset if the action is continues. Else return false
	*/
	bool IsActionLooping()
	{
		return m_bLoopAction && ShouldPerformPerFrame() && GetActionDuration() < 0;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	\return Duration of loop action being held at the end of the looping of the action
	*/
	float GetLoopActionHoldDuration()
	{
		return m_fLoopActionHoldDuration;
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnActionSelected()
	{
		m_LastUserActionContext = GetActiveContext();
	}

	//------------------------------------------------------------------------------------------------
	override void OnActionStart(IEntity pUserEntity)
	{
		m_LastUserActionContext = GetActiveContext();

		//~ Used for looping actions
		if (ShouldPerformPerFrame())
			m_fLoopProgress = Math.AbsFloat(GetActionDuration());

		if (m_bLowerWeaponOnUse)
		{
			if (SCR_PlayerController.GetLocalControlledEntity() != pUserEntity)
			{
				RplComponent rplComp = RplComponent.Cast(pUserEntity.FindComponent(RplComponent));
				if (rplComp && !rplComp.IsOwner())
					return;
			}

			ChimeraCharacter character = ChimeraCharacter.Cast(pUserEntity);
			if (!character)
				return;

			CharacterControllerComponent contr = character.GetCharacterController();
			if (!contr)
				return;

			if (contr.CanPartialLower() && !contr.IsPartiallyLowered())
				contr.SetPartialLower(true);
		}
	}
}

//------------------------------------------------------------------------------------------------
//~ When can the action be shown
enum EUserActionInVehicleState
{
	IGNORE = 0,
	NOT_IN_VEHICLE = 10,
	IN_VEHICLE_ANY = 20,
	IN_VEHICLE_PILOT = 30,
	IN_VEHICLE_TURRET = 40,
	IN_VEHICLE_CARGO = 50,
}

