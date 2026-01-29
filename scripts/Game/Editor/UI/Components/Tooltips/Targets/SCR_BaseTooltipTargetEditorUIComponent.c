//! @ingroup Editor_UI Editor_UI_Components

class SCR_BaseTooltipTargetEditorUIComponent : SCR_BaseEditorUIComponent
{
	[Attribute("0", UIWidgets.ComboBox, "", enums: ParamEnumArray.FromEnum(EEditorTooltip))]
	protected EEditorTooltip m_TooltipType;

//	//------------------------------------------------------------------------------------------------
//	EEditorTooltip GetTooltipType()
//	{
//		return m_TooltipType;
//	}

	//------------------------------------------------------------------------------------------------
	void SetTooltipType(EEditorTooltip tooltipType)
	{
		m_TooltipType = tooltipType;
	}
	
	//------------------------------------------------------------------------------------------------
	protected SCR_UIInfo GetInfo();

	//------------------------------------------------------------------------------------------------
	protected Managed GetTarget();
	
	//------------------------------------------------------------------------------------------------
	protected void ShowTooltip()
	{
		SCR_TooltipManagerEditorUIComponent tooltipManager = SCR_TooltipManagerEditorUIComponent.Cast(GetRootComponent().FindComponent(SCR_TooltipManagerEditorUIComponent, true));
		if (tooltipManager)
			tooltipManager.SetInfo(GetInfo(), m_TooltipType, GetTarget());
	}

	//------------------------------------------------------------------------------------------------
	protected void HideTooltip()
	{
		if (!GetInfo())
			return;
		
		SCR_TooltipManagerEditorUIComponent tooltipManager = SCR_TooltipManagerEditorUIComponent.Cast(GetRootComponent().FindComponent(SCR_TooltipManagerEditorUIComponent));
		if (tooltipManager)
			tooltipManager.ResetInfo(GetInfo());
	}

	//------------------------------------------------------------------------------------------------
	protected void RefreshTooltip()
	{
		SCR_TooltipManagerEditorUIComponent tooltipManager = SCR_TooltipManagerEditorUIComponent.Cast(GetRootComponent().FindComponent(SCR_TooltipManagerEditorUIComponent, true));
		if (tooltipManager)
			tooltipManager.RefreshInfo(GetInfo(), m_TooltipType, GetTarget());
	}

	//------------------------------------------------------------------------------------------------
	override bool OnFocus(Widget w, int x, int y)
	{
		ShowTooltip();
		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnFocusLost(Widget w, int x, int y)
	{
		HideTooltip();
		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnMouseEnter(Widget w, int x, int y)
	{
		ShowTooltip();
		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnMouseLeave(Widget w, Widget enterW, int x, int y)
	{
		HideTooltip();
		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool IsUnique()
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		if (GetRootComponent())
			HideTooltip();
		
		super.HandlerDeattached(w);
	}
}
