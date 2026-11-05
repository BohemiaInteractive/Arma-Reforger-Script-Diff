class SCR_ChangeBallisticShellTypeAction : SCR_ChangeDataPageAction
{
	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		return super.CanBeShownScript(user) && SCR_BallisticTableComponent.Cast(m_DataDisplayGadgetComp) != null;
	}

	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		SCR_BallisticTableComponent comp = SCR_BallisticTableComponent.Cast(m_DataDisplayGadgetComp);
		if (comp)
			comp.ChangeShellType(m_iStep);
	}
}