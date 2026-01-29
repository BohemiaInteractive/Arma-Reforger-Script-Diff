//------------------------------------------------------------------------------------------------
class SCR_SelectionHintComponent : ScriptedWidgetComponent
{
	[Attribute("{73B3D8BBB785B5B9}UI/Textures/Common/circleFull.edds", UIWidgets.ResourceNamePicker, "")]
	protected ResourceName m_sHintElementTexture;

	[Attribute("", UIWidgets.ResourceNamePicker, "")]
	protected ResourceName m_sHintElementImage;
	
	[Attribute("SelectionHintElement", UIWidgets.EditBox, "Name for generated selection hints widgets")]
	protected string m_sSelectionHintElementName;

	[Attribute("0.760 0.392 0.08 1", UIWidgets.ColorPicker)]
	protected ref Color m_ColorSelected;

	[Attribute("0.25 0.25 0.25 1", UIWidgets.ColorPicker)]
	protected ref Color m_ColorDeselected;

	[Attribute("true")]
	protected bool m_bSetCustomSize;

	[Attribute("16")]
	protected float m_fItemWidth;

	[Attribute("16")]
	protected float m_fItemHeight;

	[Attribute("0.2")]
	protected float m_fAnimationTime;

	[Attribute("8")]
	protected float m_fItemSpacing;

	[Attribute("0")]
	protected int m_iCurrent;
	
	[Attribute("0")]
	protected int m_iItemCount;

	protected float m_fAnimationRate = SCR_WLibComponentBase.START_ANIMATION_RATE;
	protected Widget m_wRoot;
	protected Widget m_wCurrent;
	protected ref array<Widget> m_aWidgets = {};

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		m_wRoot = w;
		
		// Convert time to animation speed
		GetGame().GetCallqueue().CallLater(SetAnimationRate, SCR_WLibComponentBase.START_ANIMATION_PERIOD);
		
		CreateWidgets(m_iItemCount);

		// Highlight correct item
		if (m_iItemCount > m_iCurrent)
			SetCurrentItem(m_iCurrent, false);
	}

	//------------------------------------------------------------------------------------------------
	protected void CreateWidgets(int count)
	{
		int currentCount = m_aWidgets.Count();
		bool addWidgets;
		
		if (currentCount == count)
			return;
		else if (currentCount < count)
			addWidgets = true;
		
		// Add
		if (addWidgets)
		{
			int iterations = count - currentCount;
			for (int i = 0 ; i < iterations; i++)
			{
				CreateWidget();
			}
		}
		else
		{
			int iterations = currentCount - count;
			for (int i = 0 ; i < iterations; i++)
			{
				// Go from the last
				m_aWidgets[currentCount - i - 1].RemoveFromHierarchy();
			}
			m_aWidgets.Resize(count);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void CreateWidget()
	{
		Widget w = GetGame().GetWorkspace().CreateWidget(WidgetType.ImageWidgetTypeID, WidgetFlags.VISIBLE |
		WidgetFlags.STRETCH | WidgetFlags.BLEND | WidgetFlags.INHERIT_CLIPPING, m_ColorDeselected, 0, m_wRoot);
		
		ImageWidget img = ImageWidget.Cast(w);
		if (!img)
			return;
		
		m_aWidgets.Insert(img);

		if (m_sHintElementTexture != string.Empty)
			SCR_WLibComponentBase.SetTexture(img, m_sHintElementTexture, m_sHintElementImage);

		if (m_bSetCustomSize)
			img.SetSize(m_fItemWidth, m_fItemHeight);
		
		img.SetName(m_sSelectionHintElementName);
		
		AlignableSlot.SetPadding(img, m_fItemSpacing * 0.5, 0, m_fItemSpacing * 0.5, 0);
		AlignableSlot.SetVerticalAlign(img, LayoutVerticalAlign.Center);
		AlignableSlot.SetHorizontalAlign(img, LayoutHorizontalAlign.Center);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetAnimationRate()
	{
		if (m_fAnimationTime <= 0)
			m_fAnimationRate = 1000;
		else
			m_fAnimationRate = 1 / m_fAnimationTime;
	}

	// Public API
	//------------------------------------------------------------------------------------------------
	void SetItemCount(int count, bool animate = true)
	{
		if (count == m_iItemCount || count == 1) // Do not show for only one item
			return;

		m_iItemCount = count;
		CreateWidgets(count);
		SetCurrentItem(m_iCurrent, animate);
	}

	//------------------------------------------------------------------------------------------------
	int GetItemCount()
	{
		return m_iItemCount;
	}

	//------------------------------------------------------------------------------------------------
	int GetCurrentItem()
	{
		return m_iCurrent;
	}

	//------------------------------------------------------------------------------------------------
	void SetCurrentItem(int index, bool animate = true)
	{
		Widget newWidget;

		if (index >= 0 && index < m_iItemCount)
			newWidget = m_aWidgets[index];
		
		//if (m_wCurrent == newWidget)
			//return;
		
		Color color;
		foreach (int i, Widget w : m_aWidgets)
		{
			if (i == index)
				color = m_ColorSelected;
			else
				color = m_ColorDeselected;
			
			ColorizeItem(w, color, animate);
		}
		
		m_iCurrent = index;
		m_wCurrent = newWidget;
	}
	
	//------------------------------------------------------------------------------------------------
	void ColorizeItem(Widget w, Color color, bool animate)
	{
		if (!color || !w || w.GetColor() == color)
			return;
		
		if (animate && m_fAnimationRate < SCR_WLibComponentBase.START_ANIMATION_RATE)
			AnimateWidget.Color(w, color, m_fAnimationRate);
		else
			w.SetColor(color);
	}
};
