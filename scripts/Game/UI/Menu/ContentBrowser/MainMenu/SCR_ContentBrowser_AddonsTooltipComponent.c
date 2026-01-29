/*!
Component which must be attached to tooltip which lists dependencies or dependent mods.
*/
class SCR_ContentBrowser_AddonsTooltipComponent : SCR_ScriptedWidgetComponent
{
	[Attribute("{30907DAA2D89E065}UI/layouts/Menus/Tooltips/Tooltip_ListLine_NoBG.layout")]
	protected ResourceName m_sAddonLineLayout;

	[Attribute("30")]
	protected int m_iMaxDisplayed;
	
	protected VerticalLayoutWidget m_wAddonsList;

	//------------------------------------------------------------------------------------------------
	protected override void HandlerAttached(Widget w)
	{
		m_wAddonsList = VerticalLayoutWidget.Cast(w.FindAnyWidget("AddonsList"));

		super.HandlerAttached(w);
	}

	//------------------------------------------------------------------------------------------------
	void Init(array<ref SCR_WorkshopItem> addons)
	{
		foreach (int i, SCR_WorkshopItem addon : addons)
		{
			bool last = i >= m_iMaxDisplayed;
			
			string message = "...";
			if (!last)
				message = addon.GetName();
			
			Widget w = GetGame().GetWorkspace().CreateWidgets(m_sAddonLineLayout, m_wAddonsList);
			TextWidget wText = TextWidget.Cast(w.FindAnyWidget("Text"));
			if (wText)
				wText.SetText(message);
			
			if (last)
			{
				Widget dot = w.FindAnyWidget("Dot");
				if (dot)
					dot.SetVisible(false);
				break;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	static SCR_ContentBrowser_AddonsTooltipComponent FindComponent(notnull Widget w)
	{
		return SCR_ContentBrowser_AddonsTooltipComponent.Cast(w.FindHandler(SCR_ContentBrowser_AddonsTooltipComponent));
	}
}
