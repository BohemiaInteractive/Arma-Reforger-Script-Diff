class SCR_ArmExplosiveChargeWithTimedFuzeAction : SCR_ExplosiveChargeAction
{
	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		if (!super.CanBeShownScript(user))
			return false;

		if (m_ChargeComp.GetUsedFuzeType() != SCR_EFuzeType.NONE)
			if (m_ChargeComp.GetUsedFuzeType() == SCR_EFuzeType.TIMED)
				return false;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{
		if (!super.CanBePerformedScript(user))
			return false;

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

		super.PerformAction(pOwnerEntity, pUserEntity);
	}

	//------------------------------------------------------------------------------------------------
	override void ProcesFinished(IEntity item, bool successful, ItemUseParameters animParams)
	{
		super.ProcesFinished(item, successful, animParams);
		if (!successful)
			return;

		if (!m_ChargeComp)
			return;

		m_ChargeComp.ArmWithTimedFuze();
		if (m_ChargeComp.GetTrigger())
			m_ChargeComp.GetTrigger().SetUser(m_ActionUser);
	}
}
