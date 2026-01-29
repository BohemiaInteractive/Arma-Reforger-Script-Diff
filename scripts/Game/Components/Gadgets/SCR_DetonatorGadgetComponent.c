class SCR_DetonatorGadgetComponentClass : SCR_GadgetComponentClass
{
	[Attribute("100", UIWidgets.Auto, desc: "If connected charges are outside of this range then they will not detonate. -1 == unlimited", params: "-1 inf")]
	protected float m_fMaxDetonationRange;

	//------------------------------------------------------------------------------------------------
	float GetmaxDetonationRange()
	{
		return m_fMaxDetonationRange;
	}
}

class SCR_DetonatorGadgetComponent : SCR_GadgetComponent
{
	[RplProp(onRplName: "OnNumberOfConnectedChargesChanged")]
	protected int m_iNumberOfConnectedCharges;

	[RplProp()]
	protected ref array<RplId> m_aConnectedCharges = {};

	protected float m_fVisibilityCheckDelay;
	protected RplComponent m_RplComp;
	protected ChimeraCharacter m_User;
	protected AnimationEventID m_iDetonatorTriggeredEventID = -1;

	protected const string TRIGGER_ANIMATION_EVENT_NAME = "DetonatorTriggered";
	protected const string ANIMATION_BIND_COMMAND = "CMD_Item_Action";
	protected const int UPDATE_WIRES_VISIBILITY_DELAY = 500;

	//------------------------------------------------------------------------------------------------
	protected void OnNumberOfConnectedChargesChanged()
	{
		ShowConnectedWiresForChargesInRange();
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	int GetNumberOfConnectedCharges()
	{
		return m_iNumberOfConnectedCharges;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	array<RplId> GetConnectedCharges()
	{
		return m_aConnectedCharges;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] explosiveChargeComp
	//! \return
	bool IsAttachedToTheDetonator(SCR_ExplosiveChargeComponent explosiveChargeComp)
	{
		return m_aConnectedCharges.Contains(Replication.FindId(explosiveChargeComp));
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] explosiveChargeCompId
	//! \return
	bool IsAttachedToTheDetonator(RplId explosiveChargeCompId)
	{
		return m_aConnectedCharges.Contains(explosiveChargeCompId);
	}

	//------------------------------------------------------------------------------------------------
	//! Validates if provided charge is on the list and in usable range of that detonator
	//! \param[in] pos position from which distance will be measured
	//! \param[in] explosiveChargeComp explosive charge component of the object to which distance will be checked
	//! \return true if explosive charge can be detonated from that position in case that detonators' range is set to -1 then it will always be true
	bool IsChargeInRange(vector pos, SCR_ExplosiveChargeComponent explosiveChargeComp)
	{
		IEntity owner = GetOwner();
		SCR_DetonatorGadgetComponentClass data = SCR_DetonatorGadgetComponentClass.Cast(GetComponentData(owner));
		if (!data)
			return true;

		if (data.GetmaxDetonationRange() == -1)//range is unlimited
			return true;

		if (!explosiveChargeComp)
			return false;

		if (pos == vector.Zero)
			pos = GetOwner().GetOrigin();

		vector chargePos = explosiveChargeComp.GetOwner().GetOrigin();
		if (vector.Distance(pos, chargePos) > data.GetmaxDetonationRange())
			return false;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Validates if provided charge is on the list and in usable range of that detonator
	//! \param[in] pos position from which distance will be measured
	//! \param[in] explosiveChargeCompId replication id of the explosive charge component of the object to which distance will be checked
	//! \return true if explosive charge can be detonated from that position in case that detonators' range is set to -1 then it will always be true
	bool IsChargeInRange(vector pos, RplId explosiveChargeCompId)
	{
		IEntity owner = GetOwner();
		SCR_DetonatorGadgetComponentClass data = SCR_DetonatorGadgetComponentClass.Cast(GetComponentData(owner));
		if (!data)
			return true;

		if (data.GetmaxDetonationRange() == -1)//range is unlimited
			return true;

		if (!m_aConnectedCharges.Contains(explosiveChargeCompId))
			return false;

		SCR_ExplosiveChargeComponent explosiveChargeComp = SCR_ExplosiveChargeComponent.Cast(Replication.FindItem(explosiveChargeCompId));
		if (!explosiveChargeComp)
			return false;

		if (pos == vector.Zero)
			pos = owner.GetOrigin();

		return IsChargeInRange(pos, explosiveChargeComp);
	}

	//------------------------------------------------------------------------------------------------
	override void ToggleActive(bool state, SCR_EUseContext context)
	{
		if (m_eUseMask == SCR_EUseContext.NONE || (m_eUseMask & context) == 0)
			return;

		if (m_iNumberOfConnectedCharges < 1)
			return;

		if (!m_User)
		{
			m_User = m_CharacterOwner;

			if (!m_User)
				return;
		}

		ChimeraCharacter character = m_CharacterOwner;
		if (character)
		{
			SCR_CharacterControllerComponent charController = SCR_CharacterControllerComponent.Cast(character.GetCharacterController());
	
			if (!charController || charController.GetLifeState() != ECharacterLifeState.ALIVE)
				return;
	
			if (!charController.CanPlayItemGesture() && charController.GetInspectEntity() != GetOwner())
				return;
		}

		RplComponent rplComponent = RplComponent.Cast(m_User.FindComponent(RplComponent));
		if (!rplComponent || !rplComponent.IsOwner())
			return;					// NOT owner of the character in possession of this gadget

		// Client side
		rplComponent = RplComponent.Cast(GetOwner().FindComponent(RplComponent));
		if (rplComponent && rplComponent.IsProxy())
			OnToggleActive(state);	// activate client side to avoid server delay

		// Sync
		SCR_GadgetManagerComponent.GetGadgetManager(m_User).AskToggleGadget(this, state);
	}

	//------------------------------------------------------------------------------------------------
	override void OnToggleActive(bool state)
	{
		if (!m_User)
		{
			if (!m_CharacterOwner)
				return;

			m_User = m_CharacterOwner;
			if (!m_User)
				return;
		}

		if (!m_CharacterOwner)
		{
			DetonateExplosiveCharge();
			PlaySound(SCR_SoundEvent.SOUND_DETONATOR_DETONATE_CHARGES);
			return;
		}

		SCR_CharacterControllerComponent charController = SCR_CharacterControllerComponent.Cast(m_User.GetCharacterController());
		charController.GetOnAnimationEvent().Insert(OnAnimationEvent);
		charController.m_OnItemUseEndedInvoker.Insert(OnAnimationEnded);

		CharacterAnimationComponent pAnimationComponent = m_User.GetAnimationComponent();
		int itemActionId = pAnimationComponent.BindCommand(ANIMATION_BIND_COMMAND);

		ItemUseParameters animParams = new ItemUseParameters();
		animParams.SetEntity(GetOwner());
		animParams.SetAllowMovementDuringAction(true);
		animParams.SetKeepInHandAfterSuccess(true);
		animParams.SetCommandID(itemActionId);
		animParams.SetCommandIntArg(1);

		charController.TryUseItemOverrideParams(animParams);
	}

	//------------------------------------------------------------------------------------------------
	//! Callback called when animation event happens.
	//! \param[in] animEventType ID of animation event.
	//! \param[in] animUserString String parameter of animation event
	//! \param[in] intParam int parameter of animation event
	//! \param[in] timeFromStart time from Start of animation event
	//! \param[in] timeToEnd time to end of animation event
	protected void OnAnimationEvent(AnimationEventID animEventType, AnimationEventID animUserString, int intParam, float timeFromStart, float timeToEnd)
	{
		if (animEventType == m_iDetonatorTriggeredEventID)
			DetonateExplosiveCharge();
	}

	//------------------------------------------------------------------------------------------------
	//! Callback called when animation of using the item ends
	//! \param[in] item animated item
	//! \param[in] successful if animation was able to finish or was interrupted
	//! \param[in] animParams parameters that were used to play this animation
	protected void OnAnimationEnded(IEntity item, bool successful, ItemUseParameters animParams)
	{
		if (!m_User)
			return;

		SCR_CharacterControllerComponent charController = SCR_CharacterControllerComponent.Cast(m_User.GetCharacterController());
		m_User = null;

		charController.GetOnAnimationEvent().Remove(OnAnimationEvent);
		charController.m_OnItemUseEndedInvoker.Remove(OnAnimationEnded);
	}

	//------------------------------------------------------------------------------------------------
	//! Trigger detonation of explosive charge if there is some connected and in range.
	protected void DetonateExplosiveCharge()
	{
		if (!m_User)
			return;

		vector myPos = GetOwner().GetOrigin();
		SCR_ExplosiveTriggerComponent trigger;
		SCR_ExplosiveChargeComponent explosiveChargeComp;
		bool isAuthority = m_RplComp && m_RplComp.Role() == RplRole.Authority;
		for (int i = m_iNumberOfConnectedCharges -1; i >= 0; i--)
		{
			explosiveChargeComp = SCR_ExplosiveChargeComponent.Cast(Replication.FindItem(m_aConnectedCharges[i]));
			if (!explosiveChargeComp)
			{
				m_aConnectedCharges.Remove(i);
				continue;
			}

			if (!IsChargeInRange(myPos, explosiveChargeComp))
				continue;

			trigger = explosiveChargeComp.GetTrigger();
			if (!trigger)
				continue;

			trigger.SetUser(m_User);
			if (isAuthority)
				trigger.UseTrigger();

			m_aConnectedCharges.Remove(i);
		}

		m_iNumberOfConnectedCharges = m_aConnectedCharges.Count();
		ShowConnectedWiresForChargesInRange();
		if (isAuthority)
			Replication.BumpMe();

		SCR_CharacterControllerComponent charController = SCR_CharacterControllerComponent.Cast(m_User.GetCharacterController());
		charController.GetOnAnimationEvent().Remove(OnAnimationEvent);
	}

	//------------------------------------------------------------------------------------------------
	//! Method meant for client side triggering of the action from outside of the gadget toggle mechanic
	void ActivateAction(notnull IEntity pUserEntity)
	{
		m_User = ChimeraCharacter.Cast(pUserEntity);
		ActivateAction();
	}

	//------------------------------------------------------------------------------------------------
	//! Action listener callback
	protected override void ActivateAction()
	{
		ToggleActive(false, SCR_EUseContext.FROM_ACTION);
	}

	//------------------------------------------------------------------------------------------------
	//! Play local sound from this entity
	//! \param[in] soundName
	protected void PlaySound(string soundName)
	{
		SoundComponent soundComp = SoundComponent.Cast(GetOwner().FindComponent(SoundComponent));
		
		if (soundComp)
		{
			soundComp.SoundEvent(soundName);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Add new charge to the list of connected charges
	void ConnectNewCharge(RplId explosiveChargeCompId, bool shouldReplicate = true)
	{
		if (m_aConnectedCharges.Contains(explosiveChargeCompId))
			return;

		m_aConnectedCharges.Insert(explosiveChargeCompId);
		m_iNumberOfConnectedCharges++;
		UpdateWiresVisibility(true);

		if (shouldReplicate && m_RplComp && m_RplComp.Role() == RplRole.Authority)
			Replication.BumpMe();
	}

	//------------------------------------------------------------------------------------------------
	//! Remove provided charge from the listed of connected charges
	void RemoveChargeFromTheList(RplId chargeCompToDisconnect, bool shouldReplicate = true)
	{
		int index = m_aConnectedCharges.Find(chargeCompToDisconnect);
		if (index == -1)
			return;

		m_aConnectedCharges.Remove(index);
		m_iNumberOfConnectedCharges--;
		UpdateWiresVisibility(m_iNumberOfConnectedCharges);

		if (shouldReplicate && m_RplComp && m_RplComp.Role() == RplRole.Authority)
			Replication.BumpMe();
	}

	//------------------------------------------------------------------------------------------------
	//! Removes replaced trigger id and appends in its place provided replacement while preventing
	//! addition of the same RplId indefinitely
	void ReplaceChargeFromTheList(RplId chargeCompToReplace, RplId replaceWith = RplId.Invalid())
	{
		if (!m_aConnectedCharges.Contains(chargeCompToReplace))
			return;

		ConnectNewCharge(replaceWith, false);
		RemoveChargeFromTheList(chargeCompToReplace, false);

		if (m_RplComp && m_RplComp.Role() == RplRole.Authority)
			Replication.BumpMe();
	}

	//------------------------------------------------------------------------------------------------
	//! Remove all charges from the listed of connected charges
	void RemoveAllChargesFromTheList()
	{
		RplId detonatorId = Replication.FindId(this);
		if (!detonatorId.IsValid())
			return;

		SCR_ExplosiveChargeComponent explosiveChargeComp;
		foreach (RplId chargeCompId : m_aConnectedCharges)
		{
			if (!chargeCompId.IsValid())
				continue;

			explosiveChargeComp = SCR_ExplosiveChargeComponent.Cast(Replication.FindItem(chargeCompId));
			if (!explosiveChargeComp)
				continue;

			explosiveChargeComp.ReplaceDetonatorFromTheList(detonatorId);
		}

		m_aConnectedCharges.Clear();
		m_iNumberOfConnectedCharges = 0;
		UpdateWiresVisibility(false);

		PlaySound(SCR_SoundEvent.SOUND_DETONATOR_DISCONNECT_WIRES);
	}

	//------------------------------------------------------------------------------------------------
	//! Change visibility of sloted wires
	protected void UpdateWiresVisibility(bool visible = false)
	{
		SlotManagerComponent slotManagerComp = SlotManagerComponent.Cast(GetOwner().FindComponent(SlotManagerComponent));
		if (!slotManagerComp)
			return;

		array<EntitySlotInfo> slots = {};
		if (slotManagerComp.GetSlotInfos(slots) < 1)
			return;

		IEntity wireEntity;
		foreach (EntitySlotInfo slot : slots)
		{
			if (!slot)
				continue;

			wireEntity = slot.GetAttachedEntity();
			if (!wireEntity)
				return;

			if (visible)
				wireEntity.SetFlags(EntityFlags.VISIBLE);
			else
				wireEntity.ClearFlags(EntityFlags.VISIBLE);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Checks if there are any charges in range and if so then will show connected wires
	protected void ShowConnectedWiresForChargesInRange()
	{
		bool shoudShowWires;
		vector ownerPos = GetOwner().GetOrigin();
		SCR_ExplosiveChargeComponent explosiveChargeComp;
		foreach (RplId chargeCompId : m_aConnectedCharges)
		{
			if (!chargeCompId.IsValid())
				continue;

			explosiveChargeComp = SCR_ExplosiveChargeComponent.Cast(Replication.FindItem(chargeCompId));
			if (!explosiveChargeComp)
				continue;

			if (IsChargeInRange(ownerPos, explosiveChargeComp))
			{
				shoudShowWires = true;
				break;
			}
		}

		UpdateWiresVisibility(shoudShowWires);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] deployed
	protected void OnDeployedStateChanged(bool newState, SCR_BaseDeployableInventoryItemComponent component)
	{
		ShowConnectedWiresForChargesInRange();
	}

	//------------------------------------------------------------------------------------------------
	//! Get gadget type
	//! \return Returns gadget type
	override EGadgetType GetType()
	{
		return EGadgetType.DETONATOR;
	}

	//------------------------------------------------------------------------------------------------
	//! Gadget has a raised animation version
	//! \return Returns true if raisable
	override bool CanBeRaised()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Set gadget mode
	//! \param[in] mode is the target mode being switched to
	//! \param[in] charOwner
	protected override void ModeSwitch(EGadgetMode mode, IEntity charOwner)
	{
		super.ModeSwitch(mode, charOwner);
		if (mode == EGadgetMode.IN_HAND)
			ActivateGadgetUpdate();
	}

	//------------------------------------------------------------------------------------------------
	//! Clear gadget mode
	//! \param[in] mode is the mode being cleared
	protected override void ModeClear(EGadgetMode mode)
	{
		super.ModeClear(mode, charOwner);
		if (mode == EGadgetMode.IN_HAND)
			DeactivateGadgetUpdate();
	}

	//------------------------------------------------------------------------------------------------
	override void ActivateGadgetUpdate()
	{
		ShowConnectedWiresForChargesInRange();

		if (m_CharacterOwner == SCR_PlayerController.GetLocalControlledEntity())
		{
			m_fVisibilityCheckDelay = GetGame().GetWorld().GetWorldTime() + 500;
			super.ActivateGadgetUpdate();
		}
	}

	//------------------------------------------------------------------------------------------------
	override void DeactivateGadgetUpdate()
	{
		super.DeactivateGadgetUpdate();
		UpdateWiresVisibility(m_iNumberOfConnectedCharges);
		m_fVisibilityCheckDelay = 0;
	}

	//------------------------------------------------------------------------------------------------
	override void Update(float timeSlice)
	{
		super.Update(timeSlice);
		if (GetGame().GetWorld().GetWorldTime() > m_fVisibilityCheckDelay)
		{
			m_fVisibilityCheckDelay = GetGame().GetWorld().GetWorldTime() + UPDATE_WIRES_VISIBILITY_DELAY;
			ShowConnectedWiresForChargesInRange();
		}
	}

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);
		m_RplComp = RplComponent.Cast(owner.FindComponent(RplComponent));
		m_iDetonatorTriggeredEventID = GameAnimationUtils.RegisterAnimationEvent(TRIGGER_ANIMATION_EVENT_NAME);
		UpdateWiresVisibility(false);
		SCR_BaseDeployableInventoryItemComponent deployableComp = SCR_BaseDeployableInventoryItemComponent.Cast(owner.FindComponent(SCR_BaseDeployableInventoryItemComponent));
		if (deployableComp)
			deployableComp.GetOnDeployedStateChanged().Insert(OnDeployedStateChanged);
	}

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		SetEventMask(owner, EntityEvent.INIT);
	}

	//------------------------------------------------------------------------------------------------
	override bool RplSave(ScriptBitWriter writer)
	{
		writer.WriteInt(m_iNumberOfConnectedCharges);
		foreach (RplId chargeCompId : m_aConnectedCharges)
		{
			writer.WriteRplId(chargeCompId);
		}

		return super.RplSave(writer);
	}

	//------------------------------------------------------------------------------------------------
	override bool RplLoad(ScriptBitReader reader)
	{
		reader.ReadInt(m_iNumberOfConnectedCharges);
		for (int i; i < m_iNumberOfConnectedCharges; i++)
		{
			RplId rplIdOutput;
			reader.ReadRplId(rplIdOutput);
			if (rplIdOutput.IsValid())
				m_aConnectedCharges.Insert(rplIdOutput);
		}

		return super.RplLoad(reader);
	}
}
