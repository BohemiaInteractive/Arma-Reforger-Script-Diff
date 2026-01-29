[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(EEditableEntityState, "m_State")]
/** @ingroup Editor_Components_Entities
*/
/*!
*/
class SCR_PlayerDelegateEditableEntityFilter : SCR_BaseEditableEntityFilter
{
	protected void AddDelayed(SCR_EditableEntityComponent entity)
	{
		Add(entity);
	}
	override bool CanAdd(SCR_EditableEntityComponent entity)
	{
		SCR_EditablePlayerDelegateComponent delegate = SCR_EditablePlayerDelegateComponent.Cast(entity);
		if (!delegate)
			return false;
		
		if (delegate.GetPlayerID() == 0)
		{
			//--- Too early, ID not registered yet, try it on the next frame
			GetGame().GetCallqueue().CallLater(AddDelayed, 1, false, entity);
			return false;
		}
		
		return true;
	}
};