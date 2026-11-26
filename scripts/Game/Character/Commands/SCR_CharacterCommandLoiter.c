enum ELoiterCommandState // TODO: SCR_
{
	LOITERING = 0,
	EXITING = 1,
	DONE = 2
}

class SCR_CharacterCommandLoiter : CharacterCommandScripted
{
	protected CharacterAnimationComponent m_pCharAnimComponent;
	protected ChimeraCharacter m_pCharacter;
	protected CharacterInputContext m_pBaseInputCtx;
	protected SCR_ScriptedCharacterInputContext m_pScrInputCtx;
	protected SCR_ScriptedCommandsStaticTable m_pStaticTable;
	protected SCR_CharacterCommandHandlerComponent m_pCommandHandler;
	protected ref SCR_LoiterCustomAnimData m_customAnimData;
	protected RplComponent m_rplComponent;
	protected ELoiterCommandState m_eState = -1;
	protected bool m_bWasTag, m_bUsingGraphAttachment;
	protected float m_fVelocityThreshold = 0.2;
	protected int m_iLoiterAbortType = -1;		// -1 is soft exit.
	//! Set when the AI took the character back while it was loitering. The loiter is stopped by the owner, which can be
	//! a later frame: the server becomes the owner again only after the ownership transfer back to it has completed.
	protected bool m_bStopWhenOwner;
	
	//------------------------------------------------------------------------------------------------
	// constructor
	void SCR_CharacterCommandLoiter(BaseAnimPhysComponent pAnimPhysComponent, ChimeraCharacter pCharacter, CharacterInputContext pBaseInputCtx, 
		SCR_ScriptedCharacterInputContext pScrInputCtx, SCR_ScriptedCommandsStaticTable pStaticTable, SCR_CharacterCommandHandlerComponent pScrCommandHandler,
		SCR_LoiterCustomAnimData customAnimData)
	{
		m_pCharAnimComponent = CharacterAnimationComponent.Cast(pAnimPhysComponent);
		m_pCharacter = pCharacter;
		m_pBaseInputCtx = pBaseInputCtx;
		m_pScrInputCtx = pScrInputCtx;
		m_pStaticTable = pStaticTable;
		m_pCommandHandler = pScrCommandHandler;
		m_customAnimData = SCR_LoiterCustomAnimData.Cast(customAnimData.Clone());
	}
	//------------------------------------------------------------------------------------------------
	SCR_ScriptedCharacterInputContext GetScriptedInputContext()
	{
		return m_pScrInputCtx;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnActivate()
	{
		if (!m_pCharacter)
			return;
		
		m_bWasTag = false;
		if (!m_rplComponent)
			m_rplComponent = m_pCharacter.GetRplComponent();

		if (m_pScrInputCtx.m_iLoiteringType < 0)
		{
			SetFlagFinished(true);
			return;
		}
		
		if (m_customAnimData)
		{
			if (m_customAnimData.m_sGraphBindingName != string.Empty && m_customAnimData.m_sGraphName != string.Empty)
			{
				SCR_CharacterControllerComponent charController = SCR_CharacterControllerComponent.Cast(m_pCharacter.FindComponent(SCR_CharacterControllerComponent));
				if (charController)
				{
					charController.GetOnAnimationEvent().Remove(OnAnimationEvent);
					charController.GetOnAnimationEvent().Insert(OnAnimationEvent);
					charController.m_OnLifeStateChanged.Remove(OnLifeStateChanged);
					charController.m_OnLifeStateChanged.Insert(OnLifeStateChanged);					
				}
				
				PreAnim_SetAttachment(m_customAnimData.m_sGraphBindingName, m_customAnimData.m_sGraphName, m_customAnimData.m_sGraphInstanceName, "MasterControl");
				m_bUsingGraphAttachment = true;
			}
			
			m_customAnimData.SpawnItem(m_pCharacter);
		}

		SwitchState(ELoiterCommandState.LOITERING);
	}

	//------------------------------------------------------------------------------------------------
	void FreeOccupiedActions()
	{
		if (!m_pScrInputCtx)
			return;
		
		IEntity loiterEnt = m_pScrInputCtx.GetLoiterEntity();
		if (!loiterEnt)
			return;
		
		BaseActionsManagerComponent actMgr = BaseActionsManagerComponent.Cast(loiterEnt.FindComponent(BaseActionsManagerComponent));
		if (!actMgr)
			return;
		
		array<BaseUserAction> actions = {};
		actMgr.GetActionsList(actions);
		
		foreach (BaseUserAction act : actions)
		{
			SCR_LoiterUserAction loiterAct = SCR_LoiterUserAction.Cast(act);
			if (loiterAct && loiterAct.IsOccupiedBy(m_pCharacter))
				loiterAct.ReleaseOccupant();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_LoiterCustomAnimData GetLoiterCustomAnimData()
	{
		return m_customAnimData;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnDeactivate()
	{
		// If loitering was terminated without properly releasing the ownership of the loiter action, do it now
		FreeOccupiedActions();
		
		// Another command replaced the loiter (e.g. a fall). It is finished here, and the owner tells the other machines.
		bool wasActive = m_eState == ELoiterCommandState.LOITERING || m_eState == ELoiterCommandState.EXITING;
		if (m_eState == ELoiterCommandState.LOITERING)
			StopLoitering(true);
		else if (m_eState == ELoiterCommandState.EXITING)
			DetachGraphAttachmentAndFinish(true);
		
		if (wasActive)
			RequestFinish();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnAnimationEvent(AnimationEventID animEventType, AnimationEventID animUserString, int intParam, float timeFromStart, float timeToEnd)
	{
		if (!m_customAnimData)
			return;
		
		string eventName = GameAnimationUtils.GetEventString(animEventType);
		if (eventName == "ItemHide")
			m_customAnimData.DeleteItem();
		else if (eventName == "Gesture_End")
			RequestFinish();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void DetachGraphAttachmentAndFinish(bool deleteItem)
	{
		if (!m_customAnimData || !m_pCommandHandler)
			return;
		
		if (deleteItem)
			m_customAnimData.DeleteItem();
		
		if (m_bUsingGraphAttachment)
			PreAnim_SetAttachment(m_customAnimData.m_sGraphBindingName, string.Empty, string.Empty, string.Empty);

		FinishLoiter();
		
		SCR_CharacterControllerComponent charController = SCR_CharacterControllerComponent.Cast(m_pCharacter.FindComponent(SCR_CharacterControllerComponent));
		if (charController)
		{
			charController.GetOnAnimationEvent().Remove(OnAnimationEvent);
			charController.m_OnLifeStateChanged.Remove(OnLifeStateChanged);
		}
	}
		
	//------------------------------------------------------------------------------------------------
	protected void OnLifeStateChanged(ECharacterLifeState previousLifeState, ECharacterLifeState newLifeState)
	{
		if (newLifeState != ECharacterLifeState.ALIVE)
		{
			DetachGraphAttachmentAndFinish(true);
			RequestFinish();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsLoiterCommandFinished()
	{
		return (m_eState == -1 || m_eState == ELoiterCommandState.DONE);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Finishes the loiter when the loiter finish input action is performed. A loiter that was not stopped on this
	//! machine (e.g. the owner's loiter was replaced by another command) plays the fast exit first, a graph attachment
	//! is detached.
	void PerformFinish()
	{
		if (m_eState == ELoiterCommandState.LOITERING && !m_bUsingGraphAttachment)
			StopLoitering(true);
		
		if (!IsLoiterCommandFinished())
			DetachGraphAttachmentAndFinish(false);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return True on the machine that controls the character and decides the loiter transitions.
	protected bool IsOwner()
	{
		return !m_rplComponent || m_rplComponent.IsOwner();
	}
	
	//------------------------------------------------------------------------------------------------
	//! On the owner, requests finishing the loiter on every machine through the loiter finish input action.
	protected void RequestFinish()
	{
		if (!IsOwner())
			return;
		
		SCR_CharacterControllerComponent charController = SCR_CharacterControllerComponent.Cast(m_pCharacter.GetCharacterController());
		if (charController)
			charController.RequestFinishLoitering();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Called on the server when the AI takes the character back, e.g. after a GM stops possessing it.
	//! At this point ownership transfer back to the server is not complete yet.
	override void OnPossess()
	{
		if (m_eState == ELoiterCommandState.LOITERING)
			m_bStopWhenOwner = true;
	}

	//------------------------------------------------------------------------------------------------
	override bool ShouldForceFreeLook()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool IsRootMotionControlled()
	{
		return m_pScrInputCtx.m_bLoiteringRootMotion;
	}
	
	//------------------------------------------------------------------------------------------------
	override void PrePhysUpdate(float pDt)
	{
		switch (m_eState)
		{
			case ELoiterCommandState.LOITERING:
			{
				// Only the owner decides when the loiter stops, the other machines follow its loiter input actions
				if (!IsOwner())
					break;
				
				// Requested again every frame until the stop is performed, the request is refused while one is pending
				if (m_bStopWhenOwner)
				{
					m_pCommandHandler.StopLoitering(true);
					break;
				}
				
				bool isTag = m_pCharAnimComponent.IsPrimaryTag(m_pStaticTable.m_IsLoiteringTag);
				m_bWasTag = isTag || m_bWasTag;
				
				bool cancelLoiterInputs = false;
				
				PlayerController playerController = GetGame().GetPlayerController();
				if (playerController && playerController.GetControlledEntity() == m_pCharacter)
				{
					InputManager iManager = GetGame().GetInputManager();
					cancelLoiterInputs = iManager.GetActionValue("CharacterFire") || iManager.GetActionValue("CharacterSprint") || iManager.GetActionValue("CharacterRaiseWeapon") || iManager.GetActionValue("CharacterWeaponADS") || iManager.GetActionValue("CharacterReload");
				}
							
				bool shouldStopLoitering;
				if (m_pCharacter)
 				{
					CharacterControllerComponent characterControllerComponent = m_pCharacter.GetCharacterController();
					if (characterControllerComponent && m_rplComponent && m_rplComponent.IsOwner())
					{
						float fallSpeed = characterControllerComponent.GetVelocity().Length();
						bool isEntityLinked = m_pCharAnimComponent.PhysicsIsLinked();
						if (isEntityLinked)
							fallSpeed = m_pCharAnimComponent.GetLinkedEntity().GetPhysics().GetVelocity().Length();
																	
						if (characterControllerComponent.IsFalling() || characterControllerComponent.IsSwimming() || (isEntityLinked && fallSpeed > m_fVelocityThreshold))
						{
							if (!m_customAnimData || m_customAnimData.m_bAllowEnvironmentExit)
								shouldStopLoitering = true;
							// Stop animation immediately
						}
					}			
				}

				if (shouldStopLoitering || (m_bWasTag && !isTag) || (cancelLoiterInputs && !m_pScrInputCtx.m_bLoiteringDisablePlayerInput))
				{
					// Falling or standing on something that moves stops immediately
					m_pCommandHandler.StopLoitering(shouldStopLoitering);
				}
			}
			break;
			case ELoiterCommandState.EXITING:
			{
				// Only the owner decides when the exit is over, the other machines follow its loiter finish input action
				if (m_bUsingGraphAttachment || !IsOwner())
					break;
				
				bool isTag = m_pCharAnimComponent.IsPrimaryTag(m_pStaticTable.m_IsLoiteringTag) || m_pCharAnimComponent.IsSecondaryTag(m_pStaticTable.m_IsLoiteringTag);
				if (!isTag)
					RequestFinish();
			}
			break;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param newState
	void SwitchState(ELoiterCommandState newState)
	{
		if (newState == m_eState)
			return;
		
		// The loiter is over and the graph got its exit value when the exit started. Writing the control variable
		// again would put the graph back into the loiter animation on a machine that never received the stop.
		if (newState == ELoiterCommandState.DONE)
		{
			m_eState = newState;
			return;
		}
		
		TAnimGraphCommand customCommand = -1;
		int customCommandValue = -1;
		if (m_customAnimData)
		{
			if (!m_customAnimData.m_sCommandName.IsEmpty())
				m_customAnimData.m_iCommandBindID = m_pCharAnimComponent.BindCommand(m_customAnimData.m_sCommandName);
			
			if (m_customAnimData.m_iCommandBindID > -1)
			{
				customCommand = m_customAnimData.m_iCommandBindID;
				customCommandValue = m_customAnimData.m_iCommandValue;
			}
			// is control variable used? - transition in anim graph should do the trick, no command executed
			if (!m_customAnimData.m_sControlVariableName.IsEmpty())
				m_customAnimData.m_iControlVariableID = m_pCharAnimComponent.BindVariableInt(m_customAnimData.m_sControlVariableName);
			
			if (m_customAnimData.m_iControlVariableID > -1)
			{
				m_pCharAnimComponent.SetVariableInt(m_customAnimData.m_iControlVariableID, m_customAnimData.m_iControlVariableValue);

				// if we are using command we will need to execute the switch below aswell so we forego to early return
				if (m_customAnimData.m_iCommandBindID < 0)
				{
					m_eState = newState;
					if (m_customAnimData.m_iControlVariableValue == -2)
						DetachGraphAttachmentAndFinish(true); // dont wait for animation event and end NOW

					return;
				}
			}
		}
		
		switch (newState)
		{
			case ELoiterCommandState.LOITERING:
			{
				m_bWasTag = false;

				if (customCommand != -1)
					m_pCharAnimComponent.CallCommand4I(customCommand, 0, customCommandValue, 0, 0, 0.0);
				else
					m_pCharAnimComponent.CallCommand4I(m_pStaticTable.m_CommandGesture, 0, m_pScrInputCtx.m_iLoiteringType, 0, 0, 0.0);
			}
			break;
			case ELoiterCommandState.EXITING:
			{
				if (customCommand == -1)
				{
					m_pCharAnimComponent.CallCommand(m_pStaticTable.m_CommandGesture, m_iLoiterAbortType, 0.0);
					if (m_iLoiterAbortType == -2)
					{
						DetachGraphAttachmentAndFinish(true);
						return;
					}
				}
				else
				{
					m_pCharAnimComponent.CallCommand(customCommand, customCommandValue, 0.0);
					// in case of fast abort, we dont wait for any animation to end and set state to DONE
					if (customCommandValue == -2)
					{
						DetachGraphAttachmentAndFinish(true);
						return;
					}
				}

			}
			break;
		}
		
		m_eState = newState;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param terminateFast should be true when going into alerted or combat state.
	void StopLoitering(bool terminateFast, SCR_LoiterCustomAnimData customAnimData = null)
	{
		if (m_eState != ELoiterCommandState.LOITERING)
			return;
		
		// The loiter is stopping, a stop requested by OnPossess is no longer needed
		m_bStopWhenOwner = false;
		
		if (customAnimData)
		{
			m_customAnimData = customAnimData;
		}
		
		if (m_customAnimData)
		{
			// Animation graph convention: the int value is -2 for fast and -1 for normal OUT animation
			if (terminateFast)
				m_iLoiterAbortType = -2;
			else
				m_iLoiterAbortType = -1;
			
			// Depending if the Loiter is controlled by Command or GraphIntVariable we set proper value
			// Control variable stop values are negative. -1 or a non-negative value (the start value, used when the caller
			// provided no stop data) means no specific stop was requested, so m_iLoiterAbortType is used. Otherwise the graph
			// would keep playing the start animation after the command finished.
			if (m_customAnimData.m_iCommandBindID != -1 || !m_customAnimData.m_sCommandName.IsEmpty())
				m_customAnimData.m_iCommandValue = m_iLoiterAbortType;
			else if ((m_customAnimData.m_iControlVariableID != -1 || !m_customAnimData.m_sControlVariableName.IsEmpty()) && m_customAnimData.m_iControlVariableValue >= -1)
				m_customAnimData.m_iControlVariableValue = m_iLoiterAbortType;
		}
		
		SwitchState(ELoiterCommandState.EXITING);
	}

	//------------------------------------------------------------------------------------------------
	void FinishLoiter()
	{
		// Technically we do not need to reset this flag because every new loiter requests starts
		// a new loiter command.
		m_bStopWhenOwner = false;
		
		SetFlagFinished(true);
		m_pScrInputCtx.m_iLoiteringType = -1;
		m_bUsingGraphAttachment = false;
		if (m_customAnimData)
			m_customAnimData.DeleteItem();
		
		SwitchState(ELoiterCommandState.DONE);
	}
}
