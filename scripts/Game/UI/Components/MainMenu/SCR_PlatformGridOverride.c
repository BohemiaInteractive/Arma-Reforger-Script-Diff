class SCR_PlatformGridOverride : ScriptedWidgetComponent
{
	[Attribute(uiwidget: UIWidgets.Flags, enums: ParamEnumArray.FromEnum(SCR_EPlatform))]
	private SCR_EPlatform m_eOverrideOnPlatform;

	[Attribute("1", params: "1 inf")]
	protected int m_iColumnSpan;

	[Attribute("1", params: "1 inf")]
	protected int m_iRowSpan;

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached (Widget w)
	{
		// Get current Platform in EPlatform enum
		EPlatform currentPlatform = System.GetPlatform();

		if ((m_eOverrideOnPlatform & (1 << currentPlatform)) != (1 << currentPlatform))
			return;

		GridSlot.SetColumnSpan(w, m_iColumnSpan);
		GridSlot.SetRowSpan(w, m_iRowSpan);
	}
}
