//! Base class for all widgets that can change their internal state as editbox or spinbox

//------------------------------------------------------------------------------------------------
class SCR_ChangeableComponentBase : SCR_WLibComponentBase
{
	[Attribute("true")]
	protected bool m_bUseLabel;
	
	[Attribute("", UIWidgets.EditBox, "")]
	protected string m_sLabel;
	
	[Attribute("true")]
	protected bool m_bForceSize;
	
	[Attribute("50", UIWidgets.EditBox, "")]
	protected float m_fSizeWithLabel;
	
	[Attribute("36", UIWidgets.EditBox, "")]
	protected float m_fSizeWithoutLabel;
	
	[Attribute("{829BC1189A899017}UI/layouts/WidgetLibrary/WLibRefined/WLib_Label.layout", UIWidgets.EditBox, "")]
	protected ResourceName m_sLabelLayout;
	
	[Attribute("SizeLayout", UIWidgets.EditBox, "Name of size layout if you want to find size that is not root child")]
	protected string m_sSizeLayout;
	
	protected Widget m_wBorder;
	protected Widget m_wBackground;
	protected Widget m_wLabelRoot;
	
//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
// Untyped invoker abuse! Not only passing different data to the same invoker is very prone to mistakes in listeners, child components' interpretation of what OnChanged means to them is very different, and led to mistakes in menus that resulted in huge amounts of pointless calls. We need different events for init, changes, and user interactions that are specific to each child
		
	ref ScriptInvoker m_OnChanged = new ScriptInvoker();
	// Arguments passed:
	// Toolbox (multiselect off): SCR_ToolboxComponent, int (selected item)
	// Toolbox (multiselect on): SCR_ToolboxComponent, int (current item), bool (is selected)
	// Checkbox: SCR_CheckboxComponent, bool (checked)
	// Spinbox: SCR_SpinBoxComponent, int (selected item)
	// ComboBox: SCR_ComboBoxComponent, int (selected item)
	// EditBox: SCR_EditBoxComponent, string (text)
	// Slider: SCR_SliderComponent, float (value)

//---- REFACTOR NOTE END ----
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		
		m_wBorder = m_wRoot.FindAnyWidget("Border");
		m_wBackground = m_wRoot.FindAnyWidget("Background");
		
		if (m_wBackground)
			m_wBackground.SetColor(Color.FromInt(UIColors.BACKGROUND_DEFAULT.PackToInt()));
		
		if (m_wBorder)
			m_wBorder.SetOpacity(0);
		
		if (m_bUseLabel)
			SetupLabel();
		else
			ClearLabel();
	}

//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
// Unmantained, and not generic enough! Too tied to Reforger's old look: this handling of borders and backgrounds is now pointless because they were mostly abandoned
	
	//------------------------------------------------------------------------------------------------
	override bool OnFocus(Widget w, int x, int y)
	{
		super.OnFocus(w, x, y);
		if (m_wBorder)
		AnimateWidget.Opacity(m_wBorder, 1, m_fAnimationRate);
		
		AnimateWidget.Color(m_wBackground, Color.FromInt(UIColors.BACKGROUND_HOVERED.PackToInt()), m_fAnimationRate);
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnFocusLost(Widget w, int x, int y)
	{
		super.OnFocusLost(w, x, y);

		if (m_wBorder)
			AnimateWidget.Opacity(m_wBorder, 0, m_fAnimationRate);
	
		Widget mouseOver = WidgetManager.GetWidgetUnderCursor();
		if (w != mouseOver && !IsChildWidget(w, mouseOver))
			AnimateWidget.Color(m_wBackground, Color.FromInt(UIColors.BACKGROUND_DEFAULT.PackToInt()), m_fAnimationRate);
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnMouseEnter(Widget w, int x, int y)
	{
		AnimateWidget.Color(m_wBackground, Color.FromInt(UIColors.BACKGROUND_HOVERED.PackToInt()), m_fAnimationRate);
		return super.OnMouseEnter(w, x, y);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnMouseLeave(Widget w, Widget enterW, int x, int y)
	{
		super.OnMouseLeave(w, enterW, x, y);
		if (GetGame().GetWorkspace().GetFocusedWidget() == w || IsChildWidget(w, enterW))
			return false;
		
		AnimateWidget.Color(m_wBackground, Color.FromInt(UIColors.BACKGROUND_DEFAULT.PackToInt()), m_fAnimationRate);
		return false;
	}
	
	
//---- REFACTOR NOTE END ----
	
	//------------------------------------------------------------------------------------------------
	protected void SetupLabel()
	{
		if (m_wLabelRoot)
			return;
		
		SizeLayoutWidget size = SizeLayoutWidget.Cast(m_wRoot.GetChildren());
		if (!size && !m_sSizeLayout.IsEmpty())
			size = SizeLayoutWidget.Cast(m_wRoot.FindAnyWidget(m_sSizeLayout));
		
		if (size)
		{
			size.EnableHeightOverride(m_bForceSize);
			size.SetHeightOverride(m_fSizeWithLabel);
		}
		
		Widget contentRoot = m_wRoot.FindAnyWidget("HorizontalLayout");
		if (!contentRoot)
			return;
		
		m_wLabelRoot = GetGame().GetWorkspace().CreateWidgets(m_sLabelLayout, contentRoot);
		if (!m_wLabelRoot)
			return;
		
		TextWidget text = TextWidget.Cast(m_wLabelRoot.FindAnyWidget("Text"));
		if (!text)
			return;
		
		text.SetText(m_sLabel);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ClearLabel()
	{
		if (m_wLabelRoot)
		{
			m_wLabelRoot.RemoveFromHierarchy();
			m_wLabelRoot = null;
		}
		
		SizeLayoutWidget size = SizeLayoutWidget.Cast(m_wRoot.GetChildren());
		if (size)
		{
			size.EnableHeightOverride(m_bForceSize);
			size.SetHeightOverride(m_fSizeWithoutLabel);
		}
		
		Widget layout = m_wRoot.FindAnyWidget("HorizontalLayout");
		if (layout)
			AlignableSlot.SetPadding(layout, 2, 2, 2, 2);
	}
	
	//------------------------------------------------------------------------------------------------
	TextWidget GetLabel()
	{
		return TextWidget.Cast(m_wLabelRoot.FindAnyWidget("Text"));
	}
	
	//------------------------------------------------------------------------------------------------
	void SetLabel(string label)
	{
		if (!m_wLabelRoot)
			return;
		
		TextWidget w = TextWidget.Cast(m_wLabelRoot.FindAnyWidget("Text"));
		if (!w)
			return;
		
		m_sLabel = label;
		
		w.SetText(m_sLabel);
	}

	//------------------------------------------------------------------------------------------------
	void UseLabel(bool use)
	{
		if (use == m_bUseLabel)
			return;
		
		m_bUseLabel = use;
		
		if (use)
			SetupLabel();
		else
			ClearLabel();
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsUsingLabel()
	{
		return m_bUseLabel;
	}
	
	//! If label is not used, label widget might not exist at all!
	//------------------------------------------------------------------------------------------------
	Widget GetLabelWidget()
	{
		return m_wLabelRoot;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetSizeWithLabel(float size)
	{
		m_fSizeWithLabel = size;
		if (m_bUseLabel)
			SetupLabel();
	}
	
	//------------------------------------------------------------------------------------------------
	void SetSizeWithoutLabel(float size)
	{
		m_fSizeWithoutLabel = size;
		if (!m_bUseLabel)
			ClearLabel();
	}
};