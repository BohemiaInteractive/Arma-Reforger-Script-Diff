[BaseContainerProps()]
/*!
Base container class for editor context actions

For implementation of a new action inherit either SCR_GeneralContextAction, SCR_HoverEntityContextAction, or SCR_SelectedEntitiesContextAction
*/
class SCR_BaseContextAction : SCR_BaseEditorAction
{
	[Attribute()]
	private bool m_bIsServer;
	
	override bool IsServer()
	{
		return m_bIsServer;
	}
};