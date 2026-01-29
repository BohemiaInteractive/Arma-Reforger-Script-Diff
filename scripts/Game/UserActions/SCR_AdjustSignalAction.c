class SCR_AdjustSignalAction : ScriptedSignalUserAction
{
	//! Adjustment step of normalized value
	[Attribute(defvalue: "0.1", desc: "Adjustment step")]
	protected float m_fAdjustmentStep;

	//! Flag for enabling adjustment with scroll wheel
	[Attribute(desc: "If action should wait for player to use their scroll wheel in order to change value")]
	protected bool m_bManualAdjustment;

	//! Flag for restarting the process when end value is reached
	[Attribute(desc: "Determines if this action will start from the begining when max value is reached - or from the other side if Adjustment Step is below 0")]
	protected bool m_bLoopAction;

	//! Name of action to control the input
	[Attribute(defvalue: "SelectAction", desc: "Input action for increase")]
	protected string m_sActionIncrease;

	//! Name of action to control the input
	[Attribute(desc: "Input action for decrease")]
	protected string m_sActionDecrease;

	//! Action start sound event name
	[Attribute(desc: "Action start sound event name")]
	protected string m_sActionStartSoundEvent;

	//! Action canceled sound event name
	[Attribute(desc: "Action canceled sound event name")]
	protected string m_sActionCanceledSoundEvent;

	//! Movement sound event name
	[Attribute(desc: "Movement sound event name")]
	protected string m_sMovementSoundEvent;

	//! Movement stop sound event name
	[Attribute(desc: "Movement stop sound event name")]
	protected string m_sMovementStopSoundEvent;
	
	//! Should action only be Visible when player is inside the vehicle
	[Attribute(desc: "Show this action only when a player is inside a vehicle")]
	protected bool m_bOnlyInVehicle;
	
	//! Should the action only be visible for the Pilot/Driver
	[Attribute(desc: "Enable if only the Pilot/Driver should see this action. OnlyInVehicle needs to be true for this to work!")]
	protected bool m_bPilotOnly;
	
	//! Normalized current value
	protected float m_fTargetValue;

	//! Interacted with by main entity. Allows reading input actions.
	protected bool m_bIsAdjustedByPlayer;

	//! Sound component on owner entity
	protected SoundComponent m_SoundComponent;

	//! Last lerp value
	protected float m_fLerpLast;

	//! Movement sound AudioHandle
	protected AudioHandle m_MovementAudioHandle;

	//------------------------------------------------------------------------------------------------
	//!	Used to tell if this action is meant to be adjustable with usage of the scroll wheel
	bool IsManuallyAdjusted()
	{
		return m_bManualAdjustment;
	}

	//------------------------------------------------------------------------------------------------
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		m_SoundComponent = SoundComponent.Cast(pOwnerEntity.FindComponent(SoundComponent));
		if (GetActionDuration() != 0)
			m_fAdjustmentStep /= Math.AbsFloat(GetActionDuration());

		m_fTargetValue = Math.Clamp(SCR_GetCurrentValue(), SCR_GetMinimumValue(), SCR_GetMaximumValue());
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		if (!m_bLoopAction && !m_bManualAdjustment)
		{
			if (m_fAdjustmentStep > 0 && SCR_GetCurrentValue() >= SCR_GetMaximumValue())
				return false;

			if (m_fAdjustmentStep < 0 && SCR_GetCurrentValue() <= SCR_GetMinimumValue())
				return false;
		}
		
		// Check if player is inside a vehicle
		if (!m_bOnlyInVehicle)
			return true;
		
		// See if character is in vehicle
		ChimeraCharacter character = ChimeraCharacter.Cast(user);
		if (!character)
			return false;
	
		// We cannot be pilot nor interior, if we are not seated in vehicle at all.
		if (!character.IsInVehicle())
			return false;
	
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
		if (m_bOnlyInVehicle && slot.GetOwner().GetRootParent() != GetOwner().GetRootParent())
			return false;
		
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Temporary fix for an issue of SetSendActionDataFlag not working properly from PerformAction
	protected void ToggleActionBypass()
	{
		HandleAction(1);
	}

	//------------------------------------------------------------------------------------------------
	override void PerformContinuousAction(IEntity pOwnerEntity, IEntity pUserEntity, float timeSlice)
	{
		if (!m_bManualAdjustment)
			HandleAction(timeSlice);
	}

	//------------------------------------------------------------------------------------------------
	override void OnActionStart(IEntity pUserEntity)
	{
		if (m_SoundComponent && m_sActionStartSoundEvent != string.Empty)
			m_SoundComponent.SoundEvent(m_sActionStartSoundEvent);

		m_bIsAdjustedByPlayer = SCR_PlayerController.GetLocalControlledEntity() == pUserEntity;

		if (!m_bIsAdjustedByPlayer)
			return;

		if (m_fTargetValue < SCR_GetMinimumValue() || m_fTargetValue > SCR_GetMaximumValue())
			m_fTargetValue = Math.Clamp(SCR_GetCurrentValue(), SCR_GetMinimumValue(), SCR_GetMaximumValue());

		if (!GetActionDuration())
			ToggleActionBypass();

		if (!m_bManualAdjustment)
			return;

		if (!m_sActionIncrease.IsEmpty())
			GetGame().GetInputManager().AddActionListener(m_sActionIncrease, EActionTrigger.VALUE, HandleAction);

		if (!m_sActionDecrease.IsEmpty())
			GetGame().GetInputManager().AddActionListener(m_sActionDecrease, EActionTrigger.VALUE, HandleActionDecrease);
	}

	//------------------------------------------------------------------------------------------------
	override void OnActionCanceled(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		// Play sound
		if (m_SoundComponent && m_sActionCanceledSoundEvent != string.Empty)
				m_SoundComponent.SoundEvent(m_sActionCanceledSoundEvent);

		if (!m_bIsAdjustedByPlayer)
			return;

		m_bIsAdjustedByPlayer = false;

		if (!m_bManualAdjustment)
			return;

		if (!m_sActionIncrease.IsEmpty())
			GetGame().GetInputManager().RemoveActionListener(m_sActionIncrease, EActionTrigger.VALUE, HandleAction);

		if (!m_sActionDecrease.IsEmpty())
			GetGame().GetInputManager().RemoveActionListener(m_sActionDecrease, EActionTrigger.VALUE, HandleActionDecrease);
	}

	//------------------------------------------------------------------------------------------------
	//! Increment target value
	//! \param[in] value multiplayer which will be applied to the step value
	protected void HandleAction(float value)
	{
		if (value == 0)
			return;

		if (m_bManualAdjustment)
			value /= Math.AbsFloat(value);

		value *= m_fAdjustmentStep;

		m_fTargetValue += value;
		if (m_bLoopAction)
		{
			if (value > 0 && float.AlmostEqual(SCR_GetCurrentValue(), SCR_GetMaximumValue()))
				m_fTargetValue = SCR_GetMinimumValue();
			else if (value < 0 && float.AlmostEqual(SCR_GetCurrentValue(), SCR_GetMinimumValue()))
				m_fTargetValue = SCR_GetMaximumValue();
		}

		// Round to adjustment step
		m_fTargetValue = Math.Round(m_fTargetValue / m_fAdjustmentStep) * m_fAdjustmentStep;

		// Limit to min/max value
		m_fTargetValue = Math.Clamp(m_fTargetValue, SCR_GetMinimumValue(), SCR_GetMaximumValue());

		if (!float.AlmostEqual(m_fTargetValue, SCR_GetCurrentValue()))
			SetSendActionDataFlag();
	}

	//------------------------------------------------------------------------------------------------
	//! Decrement target value
	//! \param[in] value multiplayer which will be applied to the step value
	protected void HandleActionDecrease(float value)
	{
		HandleAction(-value);
	}

	//------------------------------------------------------------------------------------------------
	//! Scripted version of the GetCurrentValue that can be overriden for custom handling
	protected float SCR_GetCurrentValue()
	{
		return GetCurrentValue();
	}

	//------------------------------------------------------------------------------------------------
	//! Scripted version of the GetMinimumValue that can be overriden for custom handling
	protected float SCR_GetMinimumValue()
	{
		return GetMinimumValue();
	}

	//------------------------------------------------------------------------------------------------
	//! Scripted version of the GetMaximumValue that can be overriden for custom handling
	protected float SCR_GetMaximumValue()
	{
		return GetMaximumValue();
	}

	//------------------------------------------------------------------------------------------------
	//! Plays movement and stop movement sound events
	protected void PlayMovementAndStopSound(float lerp)
	{
		if (!m_SoundComponent)
			return;

		if (m_fLerpLast == lerp)
			return;
		
		vector contextTransform[4];
		GetActiveContext().GetTransformationModel(contextTransform);

		if (float.AlmostEqual(lerp, 1))
		{
			if (!float.AlmostEqual(m_fLerpLast, 1))
			{
				m_SoundComponent.Terminate(m_MovementAudioHandle);
				if (m_sMovementStopSoundEvent != string.Empty)
					m_SoundComponent.SoundEventOffset(m_sMovementStopSoundEvent, contextTransform[3])
			}
		}
		else if (float.AlmostEqual(lerp, 0))
		{
			if (!float.AlmostEqual(m_fLerpLast, 0))
			{
				m_SoundComponent.Terminate(m_MovementAudioHandle);
				if (m_sMovementStopSoundEvent != string.Empty)
					m_SoundComponent.SoundEventOffset(m_sMovementSoundEvent, contextTransform[3])
			}
		}
		else
		{
			if (m_SoundComponent.IsFinishedPlaying(m_MovementAudioHandle) && m_sMovementSoundEvent != string.Empty)
				m_SoundComponent.SoundEventOffset(m_sMovementSoundEvent, contextTransform[3])
		}

		m_fLerpLast = lerp;
	}

	//------------------------------------------------------------------------------------------------
	//! Is the script broadcast to the server?
	override bool HasLocalEffectOnlyScript()
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! If HasLocalEffectOnly() is true this method tells if the server is supposed to broadcast this action to clients.
	override bool CanBroadcastScript()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Before performing the action the caller can store some data in it which is delivered to others.
	//! Only available for actions for which HasLocalEffectOnly returns false.
	override protected bool OnSaveActionData(ScriptBitWriter writer)
	{
		writer.WriteFloat(m_fTargetValue);
		SetSignalValue(m_fTargetValue);
		PlayMovementAndStopSound(Math.InverseLerp(SCR_GetMinimumValue(), SCR_GetMaximumValue(), m_fTargetValue));

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! If the one performing the action packed some data in it everybody receiving the action.
	//! Only available for actions for which HasLocalEffectOnly returns false.
	//! Only triggered if the sender wrote anyting to the buffer.
	override protected bool OnLoadActionData(ScriptBitReader reader)
	{
		if (m_bIsAdjustedByPlayer)
			return true;

		reader.ReadFloat(m_fTargetValue);
		SetSignalValue(m_fTargetValue);
		PlayMovementAndStopSound(Math.InverseLerp(SCR_GetMinimumValue(), SCR_GetMaximumValue(), m_fTargetValue));

		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override float GetActionProgressScript(float fProgress, float timeSlice)
	{
		if (IsManuallyAdjusted() && SCR_GetMaximumValue() - SCR_GetMinimumValue() != 0)
			return (m_fTargetValue - SCR_GetMinimumValue()) / (SCR_GetMaximumValue() - SCR_GetMinimumValue());
		
		return fProgress + timeSlice;
	}
}
