// Simple class to make current button link to the DLC store
class SCR_ButtonDLCStoreLinkComponent : SCR_ScriptedWidgetComponent
{
	[Attribute(defvalue: "0", desc: "DLC store to be opened after pressing the button", uiwidget: UIWidgets.ComboBox, enumType: EDlc)]
	protected EDlc m_eLinkedDLC;

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
		SCR_InputButtonComponent inputButton = SCR_InputButtonComponent.FindComponent(GetRootWidget());
		if (!inputButton)
			return;

		inputButton.m_OnActivated.Insert(OpenDLCStore);

		PlatformKind platform = GetGame().GetPlatformService().GetLocalPlatformKind();

		switch (platform)
		{
			case (PlatformKind.XBOX):
				inputButton.SetLabel("#AR-DLC_Store_MS");
				break;

			case (PlatformKind.PSN):
				inputButton.SetLabel("#AR-DLC_Store_PS");
				break;

			default:
				inputButton.SetLabel("#AR-DLC_Store");
				break;
		}
	}

	//------------------------------------------------------------------------------------------------
	//!
	void OpenDLCStore()
	{
		DlcManager manager = DlcManager.Get();
		if (!manager)
			return;

		manager.OpenStore(m_eLinkedDLC);
	}
}