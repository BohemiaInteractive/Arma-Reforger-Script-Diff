class SCR_InventorySpinBoxComponent : SCR_SpinBoxComponent
{
	protected bool m_bIsFocused;
	protected SCR_InventoryHitZoneUI m_FocusedPoint;

	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		SetNavigationActive(false);
		m_OnChanged.Insert(OnSpinBoxSelectionChanged);
	}

	override bool OnFocus(Widget w, int x, int y)
	{
		super.OnFocus(w, x, y);
		SetNavigationActive(true);
		m_bIsFocused = true;

		OnSpinBoxSelectionChanged(this, m_iSelectedItem);
		if (m_FocusedPoint)
			m_FocusedPoint.GetMenuHandler().OnAttachmentSpinboxFocused();

		return false;
	}

	override bool OnFocusLost(Widget w, int x, int y)
	{
		super.OnFocusLost(w, x, y);
		SetNavigationActive(false);
		m_bIsFocused = false;

		return false;
	}

	void OnSpinBoxSelectionChanged(SCR_SpinBoxComponent comp, int itemIndex)
	{
		if (!m_bIsFocused || itemIndex < 0)
			return;

		m_FocusedPoint = null;
		SCR_InventoryHitZonePointContainerUI hzContainer = SCR_InventoryHitZonePointContainerUI.Cast(comp.GetCurrentItemData());
		if (hzContainer)
		{
			m_FocusedPoint = hzContainer.GetStorage();
			hzContainer.ShowApplicableItems();
		}

		SCR_InventorySlotGearInspectionUI point = SCR_InventorySlotGearInspectionUI.Cast(comp.GetCurrentItemData());
		if (point)
		{
			point.ShowCompatibleAttachments();
		}
	}
	
	int FindItem(string item)
	{
		return m_aElementNames.Find(item);
	}

	bool IsFocused()
	{
		return m_bIsFocused;
	}

	SCR_InventoryHitZoneUI GetFocusedHZPoint()
	{
		return m_FocusedPoint;
	}

	void SetNavigationActive(bool active)
	{
		if (m_ButtonLeft)
		{
			if (!active)
				m_ButtonLeft.m_OnClicked.Remove(OnLeftArrowClick);
			else
				m_ButtonLeft.m_OnClicked.Insert(OnLeftArrowClick);
		}
		if (m_ButtonRight)
		{
			if (!active)
				m_ButtonRight.m_OnClicked.Remove(OnRightArrowClick);
			else
				m_ButtonRight.m_OnClicked.Insert(OnRightArrowClick);
		}
	}
	
	override protected void OnMenuLeft() 
	{
		
	}

	override protected void OnMenuRight()
	{
	
	}

};