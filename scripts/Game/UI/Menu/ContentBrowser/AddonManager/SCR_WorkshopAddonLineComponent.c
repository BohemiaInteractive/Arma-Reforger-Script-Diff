/*
Component to be attached to addon lines.
*/

class SCR_WorkshopAddonLineComponent : SCR_AddonLineBaseComponent
{
	//------------------------------------------------------------------------------------------------
	override bool OnClick(Widget w, int x, int y, int button)
	{
		bool result = super.OnClick(w, x, y, button);
		OnOpenDetailsButton();

		return result;
	}

	//------------------------------------------------------------------------------------------------
	override void UpdateAllWidgets()
	{
		if (m_Item)
			HandleEnableButtons(IsEnabled());

		super.UpdateAllWidgets();
	}

	//------------------------------------------------------------------------------------------------
	override bool IsEnabled()
	{
		if (!m_Item)
			return false;

		return m_Item.GetEnabled();
	}
}
