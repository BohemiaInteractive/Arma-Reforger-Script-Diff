class SCR_BIKIBulletHelper
{
	//------------------------------------------------------------------------------------------------
	//! \param[in] bulletContainer
	//! \return
	static float GetInitialSpeed(notnull BaseContainer bulletContainer)
	{
		IEntityComponentSource shellMoveComponentSource = SCR_BaseContainerTools.FindComponentSource(bulletContainer, ShellMoveComponent);
		if (!shellMoveComponentSource)
			return 0;

		float result;
		shellMoveComponentSource.Get("InitSpeed", result);
		return result;
	}
}
