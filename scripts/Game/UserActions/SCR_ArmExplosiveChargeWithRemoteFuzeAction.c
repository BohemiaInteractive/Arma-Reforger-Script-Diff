class SCR_ArmExplosiveChargeWithRemoteFuzeAction : SCR_ExplosiveChargeAction
{
	protected SCR_DetonatorGadgetComponent m_HeldDetonatorComp;
	protected bool m_bInProgress;

	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		if (!super.CanBeShownScript(user))
			return false;

		if (m_bInProgress)
			return false;

		ChimeraCharacter character = ChimeraCharacter.Cast(user);
		if (!character)
			return false;

		CharacterControllerComponent charController = character.GetCharacterController();
		if (!charController || charController.IsChangingItem())
			return false;

		IEntity item = charController.GetAttachedGadgetAtLeftHandSlot();
		if (!item)
			return false;

		m_HeldDetonatorComp = SCR_DetonatorGadgetComponent.Cast(item.FindComponent(SCR_DetonatorGadgetComponent));
		if (!m_HeldDetonatorComp)
			return false;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{
		if (!super.CanBePerformedScript(user))
			return false;

		if (!m_HeldDetonatorComp)
			return false;

		if (m_HeldDetonatorComp.IsAttachedToTheDetonator(m_ChargeComp.GetRplId()))
		{
			UIInfo actionInfo = GetUIInfo();
			if (!actionInfo)
				return false;

			SetCannotPerformReason(actionInfo.GetDescription());
			return false;
		}

		if (m_ChargeComp.GetUsedFuzeType() != SCR_EFuzeType.NONE)
		{
			SetCannotPerformReason(m_sDifferentFuzeInUseText);
			return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		if (!pUserEntity || !m_ChargeComp)
			return;

		if (!m_ChargeComp.GetRplId().IsValid())
			return;

		ChimeraCharacter character = ChimeraCharacter.Cast(pUserEntity);
		if (!character)
			return;

		CharacterControllerComponent charController = character.GetCharacterController();
		if (!charController || charController.IsChangingItem())
			return;

		IEntity item = charController.GetAttachedGadgetAtLeftHandSlot();
		if (!item)
			return;

		m_HeldDetonatorComp = SCR_DetonatorGadgetComponent.Cast(item.FindComponent(SCR_DetonatorGadgetComponent));
		if (!m_HeldDetonatorComp)//we need to do this as other clients will also execute this code and they need to have newest detonator
			return;

		super.PerformAction(pOwnerEntity, pUserEntity);
	}

	//------------------------------------------------------------------------------------------------
	override void AnimateUsage(IEntity pUserEntity)
	{
		m_bInProgress = true;
		super.AnimateUsage(pUserEntity);
	}

	//------------------------------------------------------------------------------------------------
	override void ProcesFinished(IEntity item, bool successful, ItemUseParameters animParams)
	{
		super.ProcesFinished(item, successful, animParams);
		m_bInProgress = false;
		if (!successful)
			return;

		if (!m_HeldDetonatorComp || !m_ChargeComp)
			return;

		m_HeldDetonatorComp.ConnectNewCharge(m_ChargeComp.GetRplId());

		if (m_ChargeComp.GetTrigger())
			m_ChargeComp.GetTrigger().SetUser(m_ActionUser);

		RplId detonatorId = Replication.FindId(m_HeldDetonatorComp);
		if (detonatorId.IsValid())
			m_ChargeComp.ConnectDetonator(SCR_EFuzeType.REMOTE, detonatorId);
	}

	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		UIInfo actionInfo = GetUIInfo();
		if (!actionInfo)
			return false;

		if (!m_HeldDetonatorComp)
			return false;

		InventoryItemComponent detonatorIIC = InventoryItemComponent.Cast(m_HeldDetonatorComp.GetOwner().FindComponent(InventoryItemComponent));
		if (!detonatorIIC)
			return false;

		UIInfo detonatorUiInfo = detonatorIIC.GetUIInfo();
		if (!detonatorUiInfo)
			return false;

		outName = WidgetManager.Translate(actionInfo.GetName(), detonatorUiInfo.GetName());

		return true;
	}
}
