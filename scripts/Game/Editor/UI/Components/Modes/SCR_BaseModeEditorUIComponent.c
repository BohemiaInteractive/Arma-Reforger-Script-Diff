class SCR_BaseModeEditorUIComponent : MenuRootSubComponent // SCR_BaseEditorUIComponent
{
	//---
	override protected void HandlerAttachedScripted(Widget w)
	{
		SCR_HintManagerComponent.HideHint();
	}
}
