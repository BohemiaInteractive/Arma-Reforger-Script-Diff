class SCR_ExplosiveTriggerComponentClass : SCR_BaseTriggerComponentClass
{
}

class SCR_ExplosiveTriggerComponent : SCR_BaseTriggerComponent
{
	protected RplComponent m_RplComp;
	protected SCR_ExplosiveChargeComponent m_ExplosiveChargeComp;

	//------------------------------------------------------------------------------------------------
	SCR_ExplosiveChargeComponent GetExplosiveChargeComponent()
	{
		return m_ExplosiveChargeComp;
	}

	//------------------------------------------------------------------------------------------------
	//! Authority method that will setoff this trigger and replicate the effect to other clients
	void UseTrigger()
	{
		RplComponent rplComp = RplComponent.Cast(GetOwner().FindComponent(RplComponent));
		if (!rplComp || rplComp.IsProxy() && !rplComp.IsOwnerProxy())
			return;

		if (m_ExplosiveChargeComp)
			m_ExplosiveChargeComp.DisarmCharge();

		GetGame().GetCallqueue().CallLater(RPC_DoTrigger);
		Rpc(RPC_DoTrigger);
	}

	//------------------------------------------------------------------------------------------------
	override event protected void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);
		m_RplComp = RplComponent.Cast(owner.FindComponent(RplComponent));
	}
}
