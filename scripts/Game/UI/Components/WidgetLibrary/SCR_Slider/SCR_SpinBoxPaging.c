//------------------------------------------------------------------------------------------------
class SCR_SpinBoxPagingComponent : SCR_WLibComponentBase
{
	[Attribute("MenuTabRight")]
	protected string m_sLeftAction;
	
	[Attribute("MenuTabLeft")]
	protected string m_sRightAction;
	
//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
// Why is this even an attribute? Why is this string not localized?
	
	[Attribute("%1 / %2", UIWidgets.Auto, "Text which will be visualized. %1 contains the actual page and %2 total pages")]
	protected string m_sText;
	
//---- REFACTOR NOTE END ----
	
	[Attribute()]
	ref array<string> m_aElementNames;

	[Attribute()]
	protected int m_iPageCount;

	[Attribute()]
	protected int m_iPageCurrent;
	
	[Attribute()]
	bool m_bCycleMode;
	
//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
// Unused
	
	protected bool m_bEnabled = true;
	protected bool m_bCanNavigate = true;

//---- REFACTOR NOTE END ----
	
	protected TextWidget m_wText;
	
	protected SCR_PagingButtonComponent m_ButtonLeft;
	protected SCR_PagingButtonComponent m_ButtonRight;
	
	ref ScriptInvoker m_OnChanged = new ScriptInvoker();
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		
		m_wText = TextWidget.Cast(m_wRoot.FindAnyWidget("Text"));
		
		m_ButtonLeft = SCR_PagingButtonComponent.GetPagingButtonComponent("ButtonLeft", w);
		if (m_ButtonLeft)
		{
			m_ButtonLeft.m_OnActivated.Insert(OnButtonLeft);
			m_ButtonLeft.SetAction(m_sLeftAction);
		}
		
		m_ButtonRight = SCR_PagingButtonComponent.GetPagingButtonComponent("ButtonRight", w);
		if (m_ButtonRight)
		{
			m_ButtonRight.m_OnActivated.Insert(OnButtonRight);
			m_ButtonRight.SetAction(m_sRightAction);
		}
		
		m_iPageCurrent = Math.Clamp(m_iPageCurrent, 0, m_iPageCount - 1);
		if (m_iPageCurrent < -1)
			m_iPageCurrent = 0;
		
		UpdateTextAndButtons(false);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnButtonLeft()
	{
		if (m_ButtonLeft.IsEnabled())
			SetCurrentItem(m_iPageCurrent - 1);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnButtonRight()
	{
		if (m_ButtonRight.IsEnabled())
			SetCurrentItem(m_iPageCurrent + 1);
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateTextAndButtons(bool animate = true)
	{
		bool disableLeft, disableRight;
		
		if (m_iPageCount < 2)
		{
			disableLeft = true;
			disableRight = true;
		}
		else if (!m_bCycleMode)
		{
			if (m_iPageCurrent <= 0)
				disableLeft = true;
			else if (m_iPageCurrent >= m_iPageCount -1)
				disableRight = true;
		}
		
		if (m_ButtonLeft)
			m_ButtonLeft.SetEnabled(!disableLeft, animate);
		
		if (m_ButtonRight)
			m_ButtonRight.SetEnabled(!disableRight, animate);
		
		// Change text
		if (!m_wText)
			return;
		
		int currentPage = Math.Min(m_iPageCurrent + 1, m_iPageCount);
		m_wText.SetTextFormat(m_sText, currentPage, m_iPageCount);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetCurrentItem(int page, bool invokeChange = true)
	{
		if (m_iPageCurrent == page)
			return;
		
		if (page < 0 || page >= m_iPageCount)
		{
			if (m_bCycleMode)
			{
				if (page == -1)
					page = m_iPageCount - 1;
				else
					page = 0;
			}
			else
			{
				return;
			}
		}
		
		m_iPageCurrent = page;
		UpdateTextAndButtons();

		// Invoke change
		if (invokeChange)
			m_OnChanged.Invoke(this, page);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetPageCount(int count, bool animate = true) 
	{
		if (count == 0 || count == 1)
			AnimateWidget.Opacity(m_wRoot, 0, UIConstants.FADE_RATE_DEFAULT);
		else
			AnimateWidget.Opacity(m_wRoot, 1, UIConstants.FADE_RATE_DEFAULT);
		
		if (count == m_iPageCount)
			return;
		
		m_iPageCount = Math.Max(count, 0);
		if (m_iPageCurrent >= m_iPageCount)
			SetCurrentItem(m_iPageCount - 1);
		
		UpdateTextAndButtons(animate);
	}
	
	//------------------------------------------------------------------------------------------------
	int GetPageCount()
	{
		return m_iPageCount;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetCurrentIndex() 
	{
		return m_iPageCurrent;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetCanNavigate(bool enable) 
	{
		if (m_ButtonLeft)
			m_ButtonLeft.SetEnabled(enable);
		
		if (m_ButtonRight)
			m_ButtonRight.SetEnabled(enable);
	}

	//------------------------------------------------------------------------------------------------
	void SetButtonsVisible(bool visible)
	{
		if (m_ButtonLeft)
			m_ButtonLeft.SetVisible(visible);

		if (m_ButtonRight)
			m_ButtonRight.SetVisible(visible);
	}

	//------------------------------------------------------------------------------------------------
	void SetButtonsActive(bool active)
	{
		if (m_ButtonLeft)
		{
			m_ButtonLeft.m_OnActivated.Remove(OnButtonLeft);
			if (active)
				m_ButtonLeft.m_OnActivated.Insert(OnButtonLeft);
		}

		if (m_ButtonRight)
		{
			m_ButtonRight.m_OnActivated.Remove(OnButtonRight);
			if (active)
				m_ButtonRight.m_OnActivated.Insert(OnButtonRight);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Static method to easily find component by providing name and parent.
	//! Searching all children will go through whole hierarchy, instead of immediate chidren
	static SCR_SpinBoxPagingComponent GetSpinBoxPagingComponent(string name, Widget parent, bool searchAllChildren = true)
	{
		auto comp = SCR_SpinBoxPagingComponent.Cast(
				SCR_WLibComponentBase.GetComponent(SCR_SpinBoxPagingComponent, name, parent, searchAllChildren)
			);
		return comp;
	}
};