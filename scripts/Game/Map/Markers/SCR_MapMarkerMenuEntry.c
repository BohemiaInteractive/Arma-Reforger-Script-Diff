//------------------------------------------------------------------------------------------------
//! Base entry for marker selection menu
class SCR_MapMarkerMenuEntry : SCR_SelectionMenuEntry
{
	const string WIDGET_SYMBOL = "SymbolOverlay";
	
	protected int m_iMarkerConfigID;		// used to load proper config entry when placing from selection menu
	protected SCR_EMapMarkerType m_eMarkerType;
	protected SCR_MilitarySymbolUIComponent m_SymbolComp;
	
	protected ref Color m_Color = GUIColors.DEFAULT;
	protected ref SCR_MilitarySymbol m_MilSymbol = new SCR_MilitarySymbol();

		
	//------------------------------------------------------------------------------------------------
	SCR_EMapMarkerType GetMarkerType()
	{
		return m_eMarkerType;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetMarkerType(SCR_EMapMarkerType type)
	{
		m_eMarkerType = type;
	}	
	
	//------------------------------------------------------------------------------------------------
	int GetMarkerConfigID()
	{
		return m_iMarkerConfigID;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetMarkerConfigID(int id)
	{
		m_iMarkerConfigID = id;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set military symbol properties 
	void SetSymbolProps(EMilitarySymbolIdentity faction, Color color, EMilitarySymbolDimension dimension, EMilitarySymbolIcon iconFlag, EMilitarySymbolAmplifier amplifier)
	{
		m_Color = color;
		
		m_MilSymbol.SetIdentity(faction);
		m_MilSymbol.SetDimension(dimension);
		m_MilSymbol.SetIcons(iconFlag);
		m_MilSymbol.SetAmplifier(amplifier);
		
		// component will be created with entry layout, which is only done after category is entered from the menu
		SCR_SelectionMenuEntryIconComponent entryComp = SCR_SelectionMenuEntryIconComponent.Cast(m_EntryComponent);
		if (!entryComp)
			return;
		
		Widget symbolWidget = entryComp.GetRootWidget().FindAnyWidget(WIDGET_SYMBOL);
		if (!symbolWidget)
			return;
		
		symbolWidget.SetColor(m_Color);
			
		m_SymbolComp = SCR_MilitarySymbolUIComponent.Cast(symbolWidget.FindHandler(SCR_MilitarySymbolUIComponent));
		if (m_SymbolComp)
			m_SymbolComp.Update(m_MilSymbol);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set custom layout based on marker type
	void SetLayout()
	{
		if (m_eMarkerType == SCR_EMapMarkerType.PLACED_MILITARY)
			SetCustomLayout("{F328D6835DA3BCFC}UI/layouts/Common/RadialMenu/RadialMenuMarkerEntry.layout");
		else 
			SetCustomLayout("{B7B4E9F530904414}UI/layouts/Common/RadialMenu/SelectionMenuEntryIcon.layout");
	}
	
	//------------------------------------------------------------------------------------------------
	override void SetEntryComponent(SCR_SelectionMenuEntryComponent entryComponent)
	{
		super.SetEntryComponent(entryComponent);
		
		SCR_SelectionMenuEntryIconComponent entryComp = SCR_SelectionMenuEntryIconComponent.Cast(m_EntryComponent);
		if (!entryComp)
			return;
		
		Widget symbolWidget = entryComp.GetRootWidget().FindAnyWidget(WIDGET_SYMBOL);
		if (!symbolWidget)
			return;
		
		symbolWidget.SetColor(m_Color);
		
		m_SymbolComp = SCR_MilitarySymbolUIComponent.Cast(symbolWidget.FindHandler(SCR_MilitarySymbolUIComponent));
		if (m_SymbolComp)
			m_SymbolComp.Update(m_MilSymbol);
	}
		
	//------------------------------------------------------------------------------------------------
	void SCR_MapMarkerMenuEntry()
	{
		SetLayout();
	}
};
