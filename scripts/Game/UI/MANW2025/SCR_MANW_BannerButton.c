class SCR_MANW_BannerButton : SCR_ScriptedWidgetComponent
{
	//------------------------------------------------------------------------------------------------
	override bool OnClick(Widget w, int x, int y, int button)
	{		
		SCR_MANW_Dialogs.CreateBannerDialog();
		
		return super.OnClick(w, x, y, button);
	}
}