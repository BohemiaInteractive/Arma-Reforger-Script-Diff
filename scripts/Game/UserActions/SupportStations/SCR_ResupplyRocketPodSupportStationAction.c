class SCR_ResupplyRocketPodSupportStationAction : SCR_ResupplyVehicleWeaponSupportStationAction
{
	protected RocketEjectorMuzzleComponent m_RocketMuzzleComp;
	
	protected int m_iTotalBarrelCount;
	protected int m_iFullBarrelCount;
	
	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{	
		//~ Not valid or has no parent
		if (!m_RocketMuzzleComp)
			return false;
		
		return super.CanBeShownScript(user);		
	}
		
	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{
		if (!super.CanBePerformedScript(user))
			return false;
		
		if (!m_RocketMuzzleComp.CanReloadNextBarrel())
		{
			SetCanPerform(false, ESupportStationReasonInvalid.RESUPPLY_INVENTORY_FULL);
			return false;
		}
		
		m_iFullBarrelCount = 0;
		
		for (int i = 0; i < m_iTotalBarrelCount; i++)
		{
			if (!m_RocketMuzzleComp.CanReloadBarrel(i))
				m_iFullBarrelCount++;
		}
		
		if (m_iFullBarrelCount < m_iTotalBarrelCount)
			return true;
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	RocketEjectorMuzzleComponent GetRocketMuzzle()
	{
		return m_RocketMuzzleComp;
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void DelayedInit(IEntity owner)
	{
		super.DelayedInit(owner);
		
		m_RocketMuzzleComp = RocketEjectorMuzzleComponent.Cast(GetOwner().FindComponent(RocketEjectorMuzzleComponent));
		if (!m_RocketMuzzleComp)
			return;
		
		m_iTotalBarrelCount = m_RocketMuzzleComp.GetBarrelsCount();
	}
	
	//------------------------------------------------------------------------------------------------
	protected override string GetActionStringParam()
	{		
		if (!m_bCanPerform || m_iTotalBarrelCount <= 1)
			return string.Empty;

		return WidgetManager.Translate(X_OUTOF_Y_FORMATTING, m_iFullBarrelCount, m_iTotalBarrelCount);
	}
}