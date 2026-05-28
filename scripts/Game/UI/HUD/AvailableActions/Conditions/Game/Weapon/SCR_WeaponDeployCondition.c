[BaseContainerProps()]
class SCR_WeaponDeployCondition : SCR_AvailableActionCondition
{
	protected bool m_bResultCache;
	protected int m_iCounter;

	//! number of execution during which we want to use cached results to avoid checking CanDeployWeapon too often
	protected const int SLEEP_COUNT = 3;

	//------------------------------------------------------------------------------------------------
	override bool IsAvailable(notnull SCR_AvailableActionsConditionData data)
	{
		if (m_iCounter < 1)
		{
			CharacterControllerComponent controller = data.GetCharacterController();
			m_bResultCache = GetReturnResult(!controller.GetIsWeaponDeployed() && controller.CanDeployWeapon());
		}

		if (m_iCounter < SLEEP_COUNT)
			m_iCounter++;
		else
			m_iCounter = 0;

		return m_bResultCache;
	}
}
