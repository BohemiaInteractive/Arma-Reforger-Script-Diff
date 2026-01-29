//------------------------------------------------------------------------------------------------
class SCR_ServerFilterBarComponent : ScriptedWidgetComponent
{	
	[Attribute()]
	protected ResourceName m_sGroupLayout;
	
	[Attribute()]
	protected ResourceName m_sButtonLayout;
	
	[Attribute("", UIWidgets.ResourceNamePicker, "")]
	protected ResourceName m_sIconSet;
	
	[Attribute("", UIWidgets.ResourceNamePicker)]
	ResourceName m_sSplitLayout;
	
	[Attribute()]
	protected int m_iGroupPadding;
	
	[Attribute()]
	protected int m_iEntriesPadding;
	
	[Attribute("50")]
	protected float m_fButtonsSize;
	
	[Attribute()]
	protected ref array<ref ServerFilterGroup> m_aFilterGroups;
	
	protected Widget m_wRoot;
	protected Widget m_wHLayout;
	protected Widget m_wTooltip;
	
	protected ref array<bool> m_aCheckFilters = new array<bool>();
	protected Widget m_wFirstWidget;
	
	ref ScriptInvoker m_OnFilterChange = new ScriptInvoker();
	
	//------------------------------------------------------------------------------------------------
	Widget GetFirstWidget() { return m_wFirstWidget; }
	
	//------------------------------------------------------------------------------------------------
	Widget GetRootWidget()
	{
		return m_wRoot;
	}
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		m_wRoot = w;
		m_wHLayout = m_wRoot.FindAnyWidget("hLayout");
 
		CreateGroups(w.IsEnabled());
		
		// Create all buttons array 
		array<Widget> wButtons = new array<Widget>();
		foreach (ServerFilterGroup group : m_aFilterGroups)
		{
			array<Widget> inGroupButs = new array<Widget>();
			inGroupButs = group.GetEntryWidgetsArray();
			
			if(inGroupButs)
				wButtons.InsertAll(inGroupButs);
			
			// Add filtering booleans 
			for(int i = 0; i < inGroupButs.Count(); i++)
			{
				m_aCheckFilters.Insert(false);
			}
		}
		
		if(wButtons.Count() > 0)
		{
			if(wButtons[0])
				m_wFirstWidget = wButtons[0];
		}
		
		// Listen to selectable invokers
		ListenToSelectables(wButtons);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void CreateGroups(bool enabled)
	{
		if(!m_aFilterGroups || !m_sButtonLayout || !m_wHLayout)
			return;
	
		int count = 0;
		
		foreach (ServerFilterGroup group : m_aFilterGroups)
		{
			group.SetSize(m_fButtonsSize);
			group.CreateButtons(m_sButtonLayout, m_sIconSet, m_wHLayout, m_iGroupPadding, m_iEntriesPadding, count, enabled);
			
			// Create split 
			if (m_sSplitLayout && count < m_aFilterGroups.Count() - 1)
				Widget split = GetGame().GetWorkspace().CreateWidgets(m_sSplitLayout, m_wHLayout);
			
			count++;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Make query from activated data
	protected void CreateQuery()
	{
		/*foreach ()
		{
			
		}*/
	}
	
	//------------------------------------------------------------------------------------------------
	//! Find listeners to button clicks
	protected void ListenToSelectables(notnull array<Widget> aWidgets)
	{
		foreach (Widget w : aWidgets)
		{	
			SCR_ButtonBaseComponent selectable = SCR_ButtonBaseComponent.Cast(w.FindHandler(SCR_ButtonBaseComponent));		
			if(!selectable)
				return;
			
			selectable.m_OnToggled.Insert(OnFilter);
			//Print("attach listener");
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Updateing filter
	protected void OnFilter(SCR_ButtonBaseComponent button, bool isSelected)
	{
		int id = 0;
		Widget w = button.GetRootWidget();
		//int pos = 0;
		
		foreach (ServerFilterGroup group : m_aFilterGroups)
		{
			if(group.WasFilterCheck(id, w))
			{
				m_aCheckFilters[id] = isSelected;
				m_wFirstWidget = w;
				
				m_OnFilterChange.Invoke(m_aCheckFilters);
				return;
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void SetEnabled(bool enabled)
	{
		m_wRoot.SetEnabled(enabled);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetVisible(bool visible)
	{
		m_wRoot.SetVisible(visible);
	}
};

//------------------------------------------------------------------------------------------------
//! Class for grouping filter
[BaseContainerProps()] 
class ServerFilterGroup
{
	[Attribute()]
	protected string m_sName;
	
	protected float m_fSize = 0;
	
	//[Attribute("", UIWidgets.Object, "")]
	
	[Attribute("", UIWidgets.Object, "")]
	protected ref array<ref ServerFilterEntry> m_aEntries;
	
	[Attribute()]
	protected EFilterGroupType m_iGroupType;
	
	//protected ref array<ref SCR_SelectableButtonComponent> m_aButtons;
	protected ref array<Widget> m_aEntryWidgets = new array<Widget>();
	protected ref Widget m_wFirstEntryWidget;
	
	ref ScriptInvoker m_OnFilterCheck = new ScriptInvoker();
	
	//------------------------------------------------------------------------------------------------
	Widget GetFirstEntryWidget() { return m_wFirstEntryWidget; }
	
	//------------------------------------------------------------------------------------------------
	array<Widget> GetEntryWidgetsArray() { return m_aEntryWidgets; }
	
	//------------------------------------------------------------------------------------------------
	int GetEntriesCount() { return m_aEntries.Count(); }
	
	//------------------------------------------------------------------------------------------------
	void SetSize(float size) { m_fSize = size; }
	
	//------------------------------------------------------------------------------------------------
	void CreateButtons(ResourceName buttonRes, ResourceName imageSet, Widget targetLayout, int iGroupPadding, int iEntryPadding, int id, bool enabled)
	{
		// Split group
		if(id != 0)
			AddPaddings(iGroupPadding, 0, targetLayout);
		
		int count = 0;
		
		foreach (ServerFilterEntry entry : m_aEntries)
		{
			// Creating widget
			Widget wButton = GetGame().GetWorkspace().CreateWidgets(buttonRes, targetLayout);
			m_aEntryWidgets.Insert(wButton);
			
			Widget wContent = wButton.FindAnyWidget("Content");
			
			// Text setting 
			TextWidget wText = TextWidget.Cast(wContent);
			
			if (wText)
				wText.SetText(entry.GetText());
			
			// Image setting
			SCR_ButtonImageComponent comp = SCR_ButtonImageComponent.Cast(wButton.FindHandler(SCR_ButtonImageComponent));
			if (!comp)
				return;
			
			comp.SetImage(imageSet,  entry.GetImageName());
			
			// Size
			SetupButtonSize(wButton, m_fSize);
			
			// Create pagging for each button
			if (count < m_aEntries.Count() - 1)
				AddPaddings(iEntryPadding, 0, targetLayout);
			
			// Get First button 
			if (!m_wFirstEntryWidget)
				m_wFirstEntryWidget = wButton;
			
			// Enable button 
			comp.SetEnabled(enabled);
			
			count++;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void AddPaddings(float w, float h, Widget targetLayout)
	{
		Widget padding = GetGame().GetWorkspace().CreateWidget(WidgetType.ImageWidgetTypeID, WidgetFlags.VISIBLE, Color.FromInt(UIColors.TRANSPARENT.PackToInt()), 0, targetLayout);
		ImageWidget img = ImageWidget.Cast(padding);
		
		if (img)
		{
			img.SetColor(Color.FromInt(UIColors.TRANSPARENT.PackToInt()));
			img.SetSize(w,h);
			HorizontalLayoutSlot.SetVerticalAlign(img, LayoutVerticalAlign.Stretch);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetupButtonSize(Widget w, float size)
	{
		if (!w)
			return;
		
		SizeLayoutWidget wSize = SizeLayoutWidget.Cast(w.FindAnyWidget("SizeLayout"));
		if (!wSize)
			return;
		
		wSize.SetWidthOverride(size);
		wSize.SetHeightOverride(size);
	}
	
	//------------------------------------------------------------------------------------------------
	bool WasFilterCheck(out int id, Widget wFilter)
	{
		for(int i = 0; i < m_aEntryWidgets.Count(); i++)
		{
			if(m_aEntryWidgets[i] == wFilter)
				return true;
			id++;
		}
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	void GroupBehavior(out array<bool> filter, int pos)
	{
		if(m_aEntries.Count() <= 1)
			return;
		
		if (m_iGroupType == EFilterGroupType.OR)
		{
			int count = 0;
			
			for (int i = 0; i < m_aEntries.Count(); i++)
			{
				if(filter[i + pos])
				{
					count++;
					//Print("filter on: " + (i + pos));
				}
			}
			
			if(count <= 1)
				return;
			
			// clear 
			for (int i = 0; i < m_aEntries.Count(); i++)
			{
				filter[i + pos] = false;
			}
		}
	}
};

//------------------------------------------------------------------------------------------------
//! Child button data class
[BaseContainerProps()]  
class ServerFilterEntry
{
	//[Attribute()]
	protected string m_sText;
	
	[Attribute()]
	protected string m_sImageName;
	
	protected int m_iFilterId;
	
	//------------------------------------------------------------------------------------------------
	void SetText(string text) { m_sText = text; }
	
	//------------------------------------------------------------------------------------------------
	string GetText() { return m_sText; }

	//------------------------------------------------------------------------------------------------
	string GetImageName() { return m_sImageName; }
	
	//------------------------------------------------------------------------------------------------
	void SetId(int id) { m_iFilterId = id; }
	
	//------------------------------------------------------------------------------------------------
	int GetId() { return m_iFilterId; }

};
//------------------------------------------------------------------------------------------------
enum EFilterGroupType
{
	OR = 0,
	AND = 1
};