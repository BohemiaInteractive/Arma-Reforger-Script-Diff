class SCR_ChangeBallisticShellTypeAction : SCR_ChangeBallisticPageAction
{
	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		m_BallisticTableComp.ChangeShellType(m_iStep);
	}
}