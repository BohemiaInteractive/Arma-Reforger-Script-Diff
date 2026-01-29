class SCR_CareerProfileMenuUI : SCR_SuperMenuBase
{
	protected const float SMOOTH_BORDER = 0.8;
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen()
	{
		super.OnMenuOpen();

		SCR_InputButtonComponent comp = m_DynamicFooter.FindButton(UIConstants.BUTTON_BACK);
		if (comp)
			comp.m_OnActivated.Insert(Close);

		bool showBlur = !GetGame().m_bIsMainMenuOpen;

		Widget img = GetRootWidget().FindAnyWidget("MenuBackground");
		if (img)
			img.SetVisible(!showBlur);

		BlurWidget blur = BlurWidget.Cast(GetRootWidget().FindAnyWidget("Blur0"));
		if (blur)
		{
			blur.SetVisible(showBlur);
			if (showBlur)
			{
				float w, h;
				GetGame().GetWorkspace().GetScreenSize(w, h);
				blur.SetSmoothBorder(0, 0, w * SMOOTH_BORDER, 0);
			}
		}
	}
}
