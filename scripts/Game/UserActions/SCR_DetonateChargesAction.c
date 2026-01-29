class SCR_DetonateChargesAction : SCR_RemoteDetonatorAction
{
	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		if (!m_DetonatorComp)
			return;

		m_DetonatorComp.ActivateAction(pUserEntity);
	}
}
