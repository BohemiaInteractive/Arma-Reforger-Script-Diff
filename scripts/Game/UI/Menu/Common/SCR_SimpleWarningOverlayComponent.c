/*
	Component that takes care of displaying error states on tiles. Includes a darkening background, different text visualization modes, and the main icon is a button that will enlarge on mouse hover
	Since it is meant to either be non-interactable or to be part of tiles, it is not focusable: the menu should provide a gamepad alternative to clicking the icon button.
*/
//------------------------------------------------------------------------------------------------
class SCR_SimpleWarningOverlayComponent : SCR_SimpleWarningComponent
{
	[Attribute("1")]
	protected bool m_bShowTextBackground;
	
	[Attribute("-10")]
	protected int m_iNakedTextTopPadding;
	
	[Attribute(UIColors.GetColorAttribute(UIColors.WARNING_DISABLED))]
	protected ref Color m_NakedTextColor;
	
	[Attribute("1")]
	protected bool m_bShowDarkeningBackground;
	
	[Attribute("1")]
	protected bool m_bShowIconGlow;

	[Attribute("0")]
	protected bool m_bEnableIconButton;
	
	[Attribute("86")]
	protected float m_fIconSize;
	
	[Attribute("1.1")]
	protected float m_fIconSizeHoveredMultiplier;
	
	[Attribute("0 0 0")]
	protected vector m_vWarningPositionOffset;
	
	[Attribute("0")]
	protected bool m_bBlurBackground;
	
	protected SmartPanelWidget m_wTextBackground;
	protected ImageWidget m_wWarningImageGlow;
	protected ImageWidget m_wWarningOuterBackground;
	protected BlurWidget m_wWarningOuterBlur;
	protected SizeLayoutWidget m_wWarningTextSize;
	protected Widget m_wWarningImageOverlay;
	protected Widget m_wWarningVerticalLayout;
	
	protected SCR_ModularButtonComponent m_WarningIconButton;
	
	protected ref ScriptInvokerScriptedWidgetComponent m_OnWarningIconButtonClicked = new ScriptInvokerScriptedWidgetComponent();
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		// Needed before super call for SetWarning()
		m_wWarningImageGlow = ImageWidget.Cast(w.FindAnyWidget("WarningImageGlow"));
		if (m_wWarningImageGlow)
			m_wWarningImageGlow.SetVisible(m_bShowIconGlow);
		
		super.HandlerAttached(w);
		
		m_wTextBackground = SmartPanelWidget.Cast(w.FindAnyWidget("WarningTextBackground"));
		if (m_wTextBackground)
			m_wTextBackground.SetVisible(m_bShowTextBackground);
		
		m_wWarningOuterBackground = ImageWidget.Cast(w.FindAnyWidget("WarningOuterBackground"));
		if (m_wWarningOuterBackground)
			m_wWarningOuterBackground.SetVisible(m_bShowDarkeningBackground);
		
		m_wWarningOuterBlur = BlurWidget.Cast(w.FindAnyWidget("WarningOuterBlur"));
		if (m_wWarningOuterBlur)
			m_wWarningOuterBlur.SetVisible(m_bBlurBackground);
		
		// Adjust text padding
		m_wWarningTextSize = SizeLayoutWidget.Cast(w.FindAnyWidget("WarningTextSize"));
		if (m_wWarningTextSize)
		{
			if (m_wWarning && !m_bShowTextBackground)
			{
				AlignableSlot.SetPadding(m_wWarningTextSize, 0, m_iNakedTextTopPadding, 0, 0);
				AlignableSlot.SetPadding(m_wWarning, 0, 0, 0, 0);
			}
		}
		
		m_wWarningVerticalLayout = w.FindAnyWidget("WarningVerticalLayout");
		if (m_wWarningVerticalLayout)
			FrameSlot.SetPos(m_wWarningVerticalLayout, m_vWarningPositionOffset[0], m_vWarningPositionOffset[1]);
		
		m_wWarningImageOverlay = w.FindAnyWidget("WarningImageOverlay");
		if (m_wWarningImageOverlay)
			FrameSlot.SetSize(m_wWarningImageOverlay, m_fIconSize, m_fIconSize);
		
		ButtonWidget warningIconButton = ButtonWidget.Cast(w.FindAnyWidget("WarningIconButton"));
		if (warningIconButton)
			m_WarningIconButton = SCR_ModularButtonComponent.FindComponent(warningIconButton);
		
		if (m_WarningIconButton)
		{
			SCR_ButtonEffectSize effect = SCR_ButtonEffectSize.Cast(m_WarningIconButton.FindEffect("ButtonSize"));
			if (effect)
			{
				vector size = {m_fIconSize, m_fIconSize, 0};
				
				effect.m_vDefault = size;
				effect.m_vHovered = size * m_fIconSizeHoveredMultiplier;
				
				m_WarningIconButton.InvokeAllEnabledEffects(true);
				
				m_WarningIconButton.m_OnClicked.Insert(OnWarningIconButtonClicked);
			}
		}
		
		SetIconButtonEnabled(m_bEnableIconButton);
	}
	
	//------------------------------------------------------------------------------------------------
	override void ResetWarningColor()
	{
		Color textColor = m_TextColor;
		if (!m_bShowTextBackground)
			textColor = m_NakedTextColor;
		
		SetWarningColor(m_Color, textColor);
	}
	
	//------------------------------------------------------------------------------------------------
	override void SetWarning(string text, string iconName, ResourceName imageset = string.Empty)
	{
		super.SetWarning(text, iconName, imageset);
		
		// TODO: glow for different imagesets
		if (m_wWarningImageGlow && !iconName.IsEmpty() && imageset == UIConstants.ICONS_IMAGE_SET)
			m_wWarningImageGlow.LoadImageFromSet(0, UIConstants.ICONS_GLOW_IMAGE_SET, iconName);
	}
	
	//------------------------------------------------------------------------------------------------
	override static SCR_SimpleWarningOverlayComponent FindComponentInHierarchy(notnull Widget root)
	{
		ScriptedWidgetEventHandler handler = SCR_WidgetTools.FindHandlerInChildren(root, SCR_SimpleWarningOverlayComponent);
		if (handler)
			return SCR_SimpleWarningOverlayComponent.Cast(handler);

		return null;
	}

	//------------------------------------------------------------------------------------------------
	override static SCR_SimpleWarningOverlayComponent FindComponent(notnull Widget w)
	{
		return SCR_SimpleWarningOverlayComponent.Cast(w.FindHandler(SCR_SimpleWarningComponent));
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnWarningIconButtonClicked()
	{
		m_OnWarningIconButtonClicked.Invoke(this);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetIconButtonEnabled(bool enabled)
	{
		if (!m_wWarningImageSize || !m_WarningIconButton)
			return;
		
		float safeSize = m_fIconSize;
		float padding;
		
		if (enabled)
		{
			// When the icon button is enabled, we need to account for it's on hover enlargement animation
			safeSize *= m_fIconSizeHoveredMultiplier;
			padding = (m_fIconSize - safeSize) / 2;
		}
		
		LayoutSlot.SetPadding(m_wWarningImageSize, 0, padding, 0, padding);
		
		m_wWarningImageSize.SetWidthOverride(safeSize);
		m_wWarningImageSize.SetHeightOverride(safeSize);
		
		m_WarningIconButton.SetEnabled(enabled);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetBlurUnderneath(bool blur)
	{
		if (m_wWarningOuterBlur)
			m_wWarningOuterBlur.SetVisible(blur);
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvokerScriptedWidgetComponent GetOnWarningIconButtonClicked()
	{
		return m_OnWarningIconButtonClicked;
	}
}