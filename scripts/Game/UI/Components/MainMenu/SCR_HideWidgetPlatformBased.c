enum SCR_EPlatform
{
	WINDOWS = 1 << EPlatform.WINDOWS,
	LINUX = 1 << EPlatform.LINUX,
	XBOX_ONE = 1 << EPlatform.XBOX_ONE,
	XBOX_ONE_S = 1 << EPlatform.XBOX_ONE_S,
	XBOX_ONE_X = 1 << EPlatform.XBOX_ONE_X,
	XBOX_SERIES_S = 1 << EPlatform.XBOX_SERIES_S,
	XBOX_SERIES_X = 1 << EPlatform.XBOX_SERIES_X,
	PS4 = 1 << EPlatform.PS4,
	PS5 = 1 << EPlatform.PS5,
	PS5_PRO = 1 << EPlatform.PS5_PRO,
	UNKNOWN = 1 << EPlatform.UNKNOWN,
}

class SCR_HideWidgetPlatformBased : ScriptedWidgetComponent
{
	[Attribute(uiwidget: UIWidgets.Flags, enums: ParamEnumArray.FromEnum(SCR_EPlatform))]
	private SCR_EPlatform m_eHideOnPlatform;

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		// Get current Platform in EPlatform enum
		EPlatform currentPlatform = System.GetPlatform();

		w.SetVisible(!(m_eHideOnPlatform & (1 << currentPlatform)) == (1 << currentPlatform));
	}
}
