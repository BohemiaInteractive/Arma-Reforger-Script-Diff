/*!
Component to handle version mismatch tooltips
*/
class SCR_VersionMismatchTooltipComponent : SCR_ScriptedWidgetComponent
{
	protected const string TEXT_CURRENT_VERSION = "CurrentVersionMessage";
	protected const string TEXT_WRONG_VERSION = "WrongVersionMessage";

	protected RichTextWidget m_wWrongVersion;

	//------------------------------------------------------------------------------------------------
	protected override void HandlerAttached(Widget w)
	{
		RichTextWidget currentVersion = RichTextWidget.Cast(w.FindAnyWidget(TEXT_CURRENT_VERSION));
		if (currentVersion)
			currentVersion.SetText(GetGame().GetBuildVersion());

		m_wWrongVersion = RichTextWidget.Cast(w.FindAnyWidget(TEXT_WRONG_VERSION));

		super.HandlerAttached(w);
	}

	//------------------------------------------------------------------------------------------------
	bool SetWrongVersionMessage(string wrongVersion)
	{
		if (!m_wWrongVersion)
			return false;

		m_wWrongVersion.SetText(wrongVersion);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	static SCR_VersionMismatchTooltipComponent FindComponent(notnull Widget w)
	{
		return SCR_VersionMismatchTooltipComponent.Cast(w.FindHandler(SCR_VersionMismatchTooltipComponent));
	}
}
