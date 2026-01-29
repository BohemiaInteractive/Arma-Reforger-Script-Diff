//! Listbox component which supports multi selection, custom element layouts.
//!
//! !!! This is not entirely finished, use at own risk.
class SCR_ListBoxComponent : ScriptedWidgetComponent
{
	// ---- Public member variables ----
	
	ref ScriptInvoker m_OnChanged = new ScriptInvoker(); // (SCR_ListBoxComponent comp, int item, bool newSelected)
	
	// ---- Protected member variables ----
	
	[Attribute("{2A736E823F7F72BB}UI/layouts/WidgetLibrary/ListBox/WLib_ListBoxIconElement.layout", UIWidgets.ResourceNamePicker, "List box element", "layout")]
	protected ResourceName m_sElementLayout;
	
	[Attribute("{6F2238B8D9FDB169}UI/layouts/WidgetLibrary/ListBox/WLib_ListBoxSeparator.layout", UIWidgets.ResourceNamePicker, "List box separator", "layout")]
	protected ResourceName m_sSeparatorLayout;
	
	[Attribute("false", UIWidgets.CheckBox, "Multiple Selection")]
	protected bool m_bMultiSelection;
	
	protected VerticalLayoutWidget m_wList;
	
	// Parallel arrays to manage element components, selected state, custom data
	protected ref array<SCR_ListBoxElementComponent> m_aElementComponents = {};
	
	// Currently selected item -  if multi selection is disabled
	protected int m_iCurrentItem = -1;
	
	// Used for generation of unique names for widgets
	protected int m_iWidgetNameNextId;
	
	// ---- Public ----
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] item
	//! \param[in] data
	//! \param[in] itemLayout
	//! \return
	int AddItem(string item, Managed data = null, ResourceName itemLayout = string.Empty)
	{	
		SCR_ListBoxElementComponent comp;
		
		int id = _AddItem(item, data, comp, itemLayout);
		
		return id;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] item
	//! \param[in] imageOrImageset
	//! \param[in] iconName
	//! \param[in] data
	//! \param[in] itemLayout
	//! \return
	// -----------------------------------------------------------------------------------------
	int AddItemAndIcon(string item, ResourceName imageOrImageset, string iconName, out SCR_ListBoxElementComponent outComp, Managed data = null, ResourceName itemLayout = string.Empty)
	{
		SCR_ListBoxElementComponent comp;
		
		int id = _AddItem(item, data, comp, itemLayout);
		
		comp.SetImage(imageOrImageset, iconName);
		
		outComp = comp;
		
		return id;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] item
	// -----------------------------------------------------------------------------------------
	void RemoveItem(int item)
	{
		if (item < 0 || item > m_aElementComponents.Count())
			return;

		Widget elementWidget = m_aElementComponents[item].GetRootWidget();
		
		m_aElementComponents.Remove(item);
		m_wList.RemoveChild(elementWidget);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] item
	//! \return
	Managed GetItemData(int item)
	{
		if (item < 0 || item > m_aElementComponents.Count())
			return null;
		
		return m_aElementComponents[item].GetData();
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	int GetItemCount()
	{
		return m_aElementComponents.Count();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns ID of item with same user data as propvided. Returns -1 if not found or null was passed
	int FindItemWithData(Managed data)
	{
		if (!data)
			return -1;
		
		int ret = -1;
		int count = m_aElementComponents.Count();
		for(int i = 0; i < count; i++)
		{
			if (m_aElementComponents[i].GetData() == data)
			{
				ret = i;
				break;
			}
		}
		
		return ret;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns IDs of selected items.
	array<int> GetSelectedItems(bool selected = true)
	{
		array<int> a = {};
		
		int c = m_aElementComponents.Count();
		
		for (int i = 0; i < c; i++)
		{
			if (m_aElementComponents[i].GetToggled() == selected)
				a.Insert(i);
		}
		
		return a;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns ID of currently selected item, if multiselection is disabled. Otherwise returns -1.
	int GetSelectedItem()
	{
		if (m_bMultiSelection)
			return -1;
		
		return m_iCurrentItem;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns list box element with given ID
	//! Probably you want to use it in very rare cases...
	SCR_ListBoxElementComponent GetElementComponent(int item)
	{
		if (item < 0 || item > m_aElementComponents.Count())
			return null;
		
		return m_aElementComponents[item];
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns true if item with given ID is selected
	bool IsItemSelected(int item)
	{
		if (item < 0 || item > m_aElementComponents.Count())
			return false;
		
		return m_aElementComponents[item].GetToggled();
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] text
	Widget AddSeparator(string text)
	{
		// Create widget for this item
		Widget w = GetGame().GetWorkspace().CreateWidgets(m_sSeparatorLayout, m_wList);
		
		TextWidget tw = TextWidget.Cast(w.FindAnyWidget("Text"));
		if (tw)
			tw.SetText(text);
		
		return w;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetFocusOnFirstItem()
	{
		if (m_aElementComponents.IsEmpty())
			return;
		
		GetGame().GetWorkspace().SetFocusedWidget(m_aElementComponents[0].GetRootWidget());
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns true when any list entry is focused.
	bool GetFocused()
	{
		Widget focused = GetGame().GetWorkspace().GetFocusedWidget();
		
		if (!focused)
			return false;
		
		foreach (SCR_ListBoxElementComponent comp : m_aElementComponents)
		{
			if (comp.GetRootWidget() == focused)
				return true;
		}
		
		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Removes all items and separators from the listbox
	void Clear()
	{
		while (m_wList.GetChildren())
		{
			m_wList.GetChildren().RemoveFromHierarchy();
		}
		
		m_aElementComponents.Clear();
		m_iCurrentItem = -1;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] item
	//! \param[in] selected
	//! \param[in] invokeOnChanged
	void SetItemSelected(int item, bool selected, bool invokeOnChanged = true, bool instant = false)
	{
		if (item < 0 || item > m_aElementComponents.Count())
			return;
		
		// If multiselection is disabled, unselect current item
		if (!m_bMultiSelection && selected && m_aElementComponents.IsIndexValid(m_iCurrentItem))
			VisualizeSelection(m_iCurrentItem, false, instant);

		_SetItemSelected(item, selected, invokeOnChanged, instant);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] selected
	//! \param[in] invokeOnChanged
	void SetAllItemsSelected(bool selected, bool invokeOnChanged = true)
	{
		int c = m_aElementComponents.Count();
		
		for (int i = 0; i < c; i++)
		{
			_SetItemSelected(i, selected, invokeOnChanged);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] item
	//! \param[in] text
	void SetItemText(int item, string text)
	{
		if (item < 0 || item > m_aElementComponents.Count())
			return;
		
		m_aElementComponents[item].SetText(text);
	}
	
	//------------------------------------------------------------------------------------------------
	// ---- Protected ----
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	protected void VisualizeSelection(int item, bool selected, bool instant = false)
	{
		m_aElementComponents[item].SetToggled(selected, true, instant);
	}
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		
		m_wList = VerticalLayoutWidget.Cast(w.FindAnyWidget("List"));
	}
	
	//------------------------------------------------------------------------------------------------
	protected int _AddItem(string item, Managed data, out SCR_ListBoxElementComponent compOut, ResourceName itemLayout = string.Empty)
	{	
		// Create widget for this item
		// The layout can be provided either as argument or through attribute
		ResourceName selectedLayout = m_sElementLayout;
		if (!itemLayout.IsEmpty())
			selectedLayout = itemLayout;
		Widget newWidget = GetGame().GetWorkspace().CreateWidgets(selectedLayout, m_wList);
		
		SCR_ListBoxElementComponent comp = SCR_ListBoxElementComponent.Cast(newWidget.FindHandler(SCR_ListBoxElementComponent));
		
		comp.SetText(item);
		comp.SetToggleable(true);
		comp.SetData(data);
		
		// Pushback to internal arrays
		int id = m_aElementComponents.Insert(comp);
		
		// Setup event handlers
		comp.m_OnClicked.Insert(OnItemClick);
		
		// Set up explicit navigation rules for elements. Otherwise we can't navigate
		// Through separators when we are at the edge of scrolling if there is an element
		// directly above/below the list box which intercepts focus
		string widgetName = this.GetUniqueWidgetName();
		newWidget.SetName(widgetName);
		if (m_aElementComponents.Count() > 1)
		{
			Widget prevWidget = m_aElementComponents[m_aElementComponents.Count() - 2].GetRootWidget();
			prevWidget.SetNavigation(WidgetNavigationDirection.DOWN, WidgetNavigationRuleType.EXPLICIT, newWidget.GetName());
			newWidget.SetNavigation(WidgetNavigationDirection.UP, WidgetNavigationRuleType.EXPLICIT, prevWidget.GetName());
		}
		
		compOut = comp;
		
		return id;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void _SetItemSelected(int item, bool selected, bool invokeOnChanged, bool instant = false)
	{
		// Set m_iCurrentItem, if multi selection is not used
		if (!m_bMultiSelection)
		{
			if (selected)
			{
				m_iCurrentItem = item;
			}
			else
			{
				// Nothing will be selected
				if (item == m_iCurrentItem)
					m_iCurrentItem = -1;
			}
		}
				
		bool oldSelected = m_aElementComponents[item].GetToggled();
		VisualizeSelection(item, selected, instant);
		
		if (invokeOnChanged && oldSelected != selected) // Only invoke if value actually changed
			m_OnChanged.Invoke(this, item, selected);
	}
	
	//------------------------------------------------------------------------------------------------
	protected string GetUniqueWidgetName()
	{
		string ret = string.Format("%1_Element_%2", this, m_iWidgetNameNextId);
		m_iWidgetNameNextId++;
		return ret;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnItemClick(SCR_ListBoxElementComponent comp)
	{
		int id = m_aElementComponents.Find(comp);
		if (id == -1)
			return;
		
		// Behaviour depends on multi selection
		if (m_bMultiSelection)
		{
			// If multi selection is enabled, inverse the selection state for this item
			bool selected = m_aElementComponents[id].GetToggled();
			_SetItemSelected(id, !selected, true);
		}
		else
		{
			// Unselect previous item
			if (id != m_iCurrentItem && m_iCurrentItem >= 0 && m_iCurrentItem < m_aElementComponents.Count())
				VisualizeSelection(m_iCurrentItem, false);
			
			// Select new item
			_SetItemSelected(id, true, true);
		}
	}
}
