//! Selectable button. Supported states: Disaled, hovered, focused, selected, deselected
//! Works with WLib_SelectableButton widget prefab

//------------------------------------------------------------------------------------------------
class SCR_SelectableButtonComponent : SCR_ButtonComponent 
{
	[Attribute(params: "edds imageset")]
	protected ResourceName m_sImageResource;
	
	[Attribute(defvalue: "", desc: "When the texture is an image set, define the quad name here")]
	private string m_sImageName;
	
	[Attribute(defvalue: "4", UIWidgets.EditBox, "How ,uch of spaceis between image and text - work if both are visible")]
	protected float m_fContentPadding;
	
	//protected string WIDGET_CHECKER = "Checker";
	protected string WIDGET_CONTET_HLAYOUT = "hLayoutContent";
	protected string WIDGET_IMAGE = "ImageContent";
	
	ref ScriptInvoker m_OnChanged = new ScriptInvoker();
	protected bool m_bIsSelected = false;
	protected bool m_bIsTriggered = true;
	
	//protected Widget m_wChecked;
	protected Widget m_wHLayoutContent;
	protected ImageWidget m_wImageContent;
	
	protected ref Color COLOR_CHECKED_TRUE = UIColors.CONTRAST_COLOR;
	protected ref Color COLOR_CHECKED_FALSE = UIColors.WHITE_DEFAULT;
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		
		//m_wChecked = m_wRoot.FindAnyWidget(WIDGET_CHECKER);
		m_wHLayoutContent = m_wRoot.FindAnyWidget(WIDGET_CONTET_HLAYOUT);
		m_wImageContent = ImageWidget.Cast(m_wRoot.FindAnyWidget(WIDGET_IMAGE));
		
		// Set image
		SetImage(m_sImageResource, m_sImageName);
		
		// Set checcker color 
		if (m_wOverlay)
		{
			if (m_bIsSelected)
				m_wOverlay.SetColor(COLOR_CHECKED_TRUE);
			else
				m_wOverlay.SetColor(COLOR_CHECKED_FALSE);
		}
		
		// Image - Text padding 
		if (!m_wImageContent || !m_wContent)
			return;
		
		if (m_wImageContent.IsVisible() && m_wContent.IsVisible())
		{
			HorizontalLayoutSlot.SetPadding(m_wContent, m_fContentPadding, 0, 0, 0);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void SetImage(ResourceName sResource, string sImageName)
	{
		if (!m_wImageContent)
			return;
		
		m_wImageContent.SetVisible(!sResource.IsEmpty());
			
		if(sResource.IsEmpty())
			return;
		
		if (sResource.EndsWith("imageset"))
				m_wImageContent.LoadImageFromSet(0, sResource, sImageName);
			else
				m_wImageContent.LoadImageTexture(0, sResource);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnClick(Widget w, int x, int y, int button)
	{
		super.OnClick(w, x, y, button);
		if (button != 0)
			return false;
		
		SetSelected(!m_bIsSelected);
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnMouseEnter(Widget w, int x, int y)
	{
		if (m_bMouseOverToFocus)
			GetGame().GetWorkspace().SetFocusedWidget(w);
		
		PlaySound(m_sSoundHovered);
		
		if (GetGame().GetWorkspace().GetFocusedWidget() != w)
			ColorizeWidgets(COLOR_BACKGROUND_HOVERED, COLOR_CONTENT_HOVERED);
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnMouseLeave(Widget w, Widget enterW, int x, int y)
	{
		if (GetGame().GetWorkspace().GetFocusedWidget() != w)
			ColorizeWidgets(COLOR_BACKGROUND_DEFAULT, COLOR_CONTENT_DEFAULT);
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnFocus(Widget w, int x, int y)
	{
		PlaySound(m_sSoundHovered);
		ColorizeWidgets(COLOR_BACKGROUND_FOCUSED, COLOR_CONTENT_FOCUSED);
		ColorizeCheckWidget();
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnFocusLost(Widget w, int x, int y)
	{
		
		if (WidgetManager.GetWidgetUnderCursor() == w)
			ColorizeWidgets(COLOR_BACKGROUND_HOVERED, COLOR_CONTENT_HOVERED);
		else
			ColorizeWidgets(COLOR_BACKGROUND_DEFAULT, COLOR_CONTENT_DEFAULT);
		
		ColorizeCheckWidget();
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void OnMenuSelect()
	{
		if (!m_wRoot.IsEnabled())
			return;

		super.OnMenuSelect();
		ColorizeCheckWidget();
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void ColorizeWidgets(Color colorBackground, Color colorContent, float speed = -1)
	{
		if (speed < 0)
			speed = m_fAnimationRate;

		AnimateWidget.Color(m_wBackground, colorBackground, speed);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set color of checked line 
	protected void ColorizeCheckWidget()
	{
		if (m_bIsSelected)
			AnimateWidget.Color(m_wOverlay, COLOR_CHECKED_TRUE, m_fAnimationRate);
		else
			AnimateWidget.Color(m_wOverlay, COLOR_CHECKED_FALSE, m_fAnimationRate);
	}
	
	// User API
	//------------------------------------------------------------------------------------------------
	bool IsSelected()
	{
		return m_bIsSelected;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetSelected(bool selected, bool notify = true)
	{
		if (m_bIsSelected == selected)
			return;
		
		m_bIsSelected = selected;
		
		ColorizeCheckWidget();
		
		if (notify)
			m_OnChanged.Invoke(this, m_wRoot, m_bIsSelected);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Static method to easily find component by providing name and parent.
	//! Searching all children will go through whole hierarchy, instead of immediate chidren
	static SCR_SelectableButtonComponent GetSelectableButtonComponent(string name, Widget parent, bool searchAllChildren = true)
	{
		auto comp = SCR_SelectableButtonComponent.Cast(
			SCR_WLibComponentBase.GetComponent(SCR_SelectableButtonComponent, name, parent, searchAllChildren)
		);
		return comp;
	}
};