// Simple class to change the view profile button label automatically
class SCR_ViewProfileButtonComponent : SCR_ScriptedWidgetComponent
{
	protected static const string PROFILE_BUTTON_TEXT = "#AR-PlayerLists_ShowProfile";
	protected static const string PROFILE_BUTTON_TEXT_CONSOLE = "#AR-PlayerLists_ShowGamercard";

	protected SCR_InputButtonComponent m_ViewProfile;

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

		Init();
	}

	//------------------------------------------------------------------------------------------------
	//!
	void Init()
	{
		m_ViewProfile = SCR_InputButtonComponent.FindComponent(GetRootWidget());
		if (!m_ViewProfile)
			return;

		string label = PROFILE_BUTTON_TEXT;

		if (GetGame().IsPlatformGameConsole() && GetGame().GetPlatformService().GetLocalPlatformKind() != PlatformKind.PSN)
			label = PROFILE_BUTTON_TEXT_CONSOLE;

		m_ViewProfile.SetLabel(label);
	}
}
