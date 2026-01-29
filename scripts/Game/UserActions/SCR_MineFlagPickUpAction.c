//------------------------------------------------------------------------------------------------
class SCR_MineFlagPickUpAction : SCR_PickUpItemAction
{
	protected ref ScriptInvoker<SCR_MineFlagPickUpAction> m_OnItemPickUp;
	
	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnItemPickUp()
	{
		if (!m_OnItemPickUp)
			m_OnItemPickUp = new ScriptInvoker();
		
		return m_OnItemPickUp;
	}
	
	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity) 
	{
		super.PerformAction(pOwnerEntity, pUserEntity);
		
		if (m_OnItemPickUp)
			m_OnItemPickUp.Invoke(this);
	}
};