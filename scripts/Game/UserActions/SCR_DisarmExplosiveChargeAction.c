class SCR_DisarmExplosiveChargeAction : SCR_ExplosiveChargeAction
{
	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		if (!super.CanBeShownScript(user))
			return false;

		if (m_ChargeComp.GetUsedFuzeType() == SCR_EFuzeType.NONE)
			return false;

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

		m_ChargeComp.DisarmCharge();
	}
}
