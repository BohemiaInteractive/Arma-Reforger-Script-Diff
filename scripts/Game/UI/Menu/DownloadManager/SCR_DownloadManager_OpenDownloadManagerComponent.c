/*!
Component which opens download manager when button is activated
*/
class SCR_DownloadManager_OpenDownloadManagerComponent : ScriptedWidgetComponent
{
	override void HandlerAttached(Widget w)
	{
		SCR_InputButtonComponent n = SCR_InputButtonComponent.Cast(w.FindHandler(SCR_InputButtonComponent));
		n.m_OnActivated.Insert(SCR_DownloadManager_Dialog.Create);
	}
};