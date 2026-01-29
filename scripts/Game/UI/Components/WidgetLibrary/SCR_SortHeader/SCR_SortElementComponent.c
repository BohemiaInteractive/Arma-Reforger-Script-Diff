enum ESortOrder
{
	ASCENDING = 0,
	DESCENDING = 1,
	NONE = 2		// This element does not perform sorting
};

/*!
Sort element must be placed on a widget inside a sort header.
The sort element visualizes sort order by using effects with different tags:
- Effects with "all" tag are always active;
- Effects with "sort_desc" and "sort_asc" are active only when sorting in specific direction;
- Effects with "sort_none" are active when not sorting in any direciton.

!!! When any sorting is selected, button is in "toggled on" state. When no sorting, button is in "toggled off" state.
Therefore effects must be configured accordingly, and proper states must be used in effect configuration.
*/

class SCR_SortElementComponent : SCR_ModularButtonComponent
{
	protected bool m_bSortOrderAscending;
	
	[Attribute("", UIWidgets.Auto, "Internal name, returned by GetName()")]
	protected string m_sName;
	
	[Attribute("false", UIWidgets.Auto, "Locks sorting direction only in the order set by m_bDefaultSortOrder.")]
	protected bool m_bSortSingleOrder;
	
	[Attribute("0", UIWidgets.ComboBox, "Default sort order when we first click on this element.", enums: ParamEnumArray.FromEnum(ESortOrder))]
	protected ESortOrder m_eDefaultSortOrder;
	
	// Tags for effects
	const string TAG_ALL = "all";
	const string TAG_SORT_ASC = "sort_asc";
	const string TAG_SORT_DESC = "sort_desc";
	const string TAG_SORT_NONE = "sort_none";
	
	
	//---------------------------------------------------------------------------------------------------
	// P U B L I C
	//---------------------------------------------------------------------------------------------------
	
	
	//---------------------------------------------------------------------------------------------------
	void SetSortOrder(ESortOrder order)
	{	
		if (order == ESortOrder.NONE)
		{
			SetToggled(false);
		}
		else
		{
			SetToggled(true);
			
			// If this must be sorted only in one order
			if (m_bSortSingleOrder)
			{
				SelectNewSortOrder(reset: true); // To be sure...
			}
			else
			{
				if (order == ESortOrder.ASCENDING)
					m_bSortOrderAscending = true;
				else
					m_bSortOrderAscending = false;
			}
		}
		
		UpdateSortEffects();
	}
	
	//---------------------------------------------------------------------------------------------------
	ESortOrder GetSortOrder()
	{
		if (!GetToggled())
			return ESortOrder.NONE;
		else
		{
			if (m_bSortOrderAscending)
				return ESortOrder.ASCENDING;
			else
				return ESortOrder.DESCENDING;
		}
	}
	
	//---------------------------------------------------------------------------------------------------
	ESortOrder GetDefaultSortOrder()
	{
		return m_eDefaultSortOrder;
	}
	
	//---------------------------------------------------------------------------------------------------
	string GetName()
	{
		return m_sName;
	}
	
	
	//---------------------------------------------------------------------------------------------------
	// P R O T E C T E D
	//---------------------------------------------------------------------------------------------------
	
	//---------------------------------------------------------------------------------------------------
	protected override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		
		// Setting default sort order to NONE is illegal
		if (m_eDefaultSortOrder == ESortOrder.NONE)
			m_eDefaultSortOrder = ESortOrder.ASCENDING;
		
		// Override toggle-ability:
		m_bCanBeToggled = true;				// This can be toggled, when toggled it means that we are sorting by this column
		m_bToggledOnlyThroughApi = true;	// Toggled only through API, because when it's toggled on, we don't want to untoggle it.
		SelectNewSortOrder(reset: true);
		UpdateSortEffects();
	}
	
	//---------------------------------------------------------------------------------------------------
	protected override void Internal_SetToggled(bool newToggled, bool invokeOnToggled = true, bool instant = false)
	{
		super.Internal_SetToggled(newToggled, invokeOnToggled, instant);
		UpdateSortEffects();
	}
	
	//---------------------------------------------------------------------------------------------------
	protected override bool OnClick(Widget w, int x, int y, int button)
	{
		bool oldToggled = m_bToggled;
		
		if (!oldToggled)
			Internal_SetToggled(true, true);
		
		SelectNewSortOrder(reset: !oldToggled); // Reset sort order when switching from toggled off to toggled on state
		UpdateSortEffects();
		
		bool returnValue = super.OnClick(w, x, y, button);
		
		return returnValue;
	}
	
	
	//---------------------------------------------------------------------------------------------------
	//! Selects new sort order, either default or flips prev. sort order
	protected void SelectNewSortOrder(bool reset)
	{
		if (m_bSortSingleOrder || reset)
		{
			if (m_eDefaultSortOrder == ESortOrder.ASCENDING)
				m_bSortOrderAscending = true;
			else
				m_bSortOrderAscending = false;
		}
		else
		{
			m_bSortOrderAscending = !m_bSortOrderAscending;
		}
	}
	
	//---------------------------------------------------------------------------------------------------
	//! Enables effects according to new sort order and toggled state:
	//! When not toggled, only effects with TAG_ALL are enabled.
	//! When toggled, also effects with TAG_SORT_DESC or TAG_SORT_ASC are enabled.
	protected void UpdateSortEffects()
	{
		array<string> tags = {TAG_ALL};
		if (GetToggled())
		{
			if (m_bSortOrderAscending)
				tags.Insert(TAG_SORT_ASC);
			else
				tags.Insert(TAG_SORT_DESC);
		}
		else
			tags.Insert(TAG_SORT_NONE);
		
		SetEffectsWithAnyTagEnabled(tags);
	}
	
	
};