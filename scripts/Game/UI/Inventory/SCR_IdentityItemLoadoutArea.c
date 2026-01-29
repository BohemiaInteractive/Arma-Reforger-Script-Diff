[BaseContainerProps(), SCR_BaseContainerCustomTitleResourceName("m_sName", true)]
class SCR_IdentityItemLoadoutArea : SCR_LoadoutArea
{	
	//------------------------------------------------------------------------
	override bool IsValid()
	{
		SCR_IdentityManagerComponent identityManager = SCR_IdentityManagerComponent.GetInstance();
		
		return identityManager && identityManager.IsIdentityItemSlotEnabled();
	}
};