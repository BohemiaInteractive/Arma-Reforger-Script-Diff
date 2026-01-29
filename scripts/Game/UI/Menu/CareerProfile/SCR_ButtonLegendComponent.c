//------------------------------------------------------------------------------------------------
class SCR_ButtonLegendComponent : SCR_WLibComponentBase 
{
	[Attribute()]
	protected LocalizedString m_sSpecializationText;
	
	protected TextWidget m_wSpecializationText;
	protected ImageWidget m_wSpecializationCoreImage;
	protected ImageWidget m_wSpecializationBackgroundImage;
	protected ImageWidget m_wSpecializationBorderImage;
	
	protected ImageWidget m_wChangeSpLeft;
	protected ImageWidget m_wChangeSpRight;
	protected Widget m_wChangeSpLeftLayout;
	protected Widget m_wChangeSpRightLayout;
	
	protected int m_ibuttonId;

	[Attribute()]
	protected bool m_bIsActive;
	
	[Attribute("0 0 0 1", UIWidgets.ColorPicker)]
	protected ref Color m_ColorInactive;
	
	[Attribute("0 0 0 0.5", UIWidgets.ColorPicker)]
	protected ref Color m_ColorInactiveHovered;
	
	[Attribute("0.898 0.541 0.184 1", UIWidgets.ColorPicker)]
	protected ref Color m_ColorActive;
	
	[Attribute("1 1 1 1", UIWidgets.ColorPicker)]
	protected ref Color m_ColorBackgroundInactive;
	
	[Attribute("0 0 0 1", UIWidgets.ColorPicker)]
	protected ref Color m_ColorBackgroundActive;
	
	[Attribute("{FDD5423E69D007F8}UI/Textures/Icons/icons_wrapperUI-128.imageset", UIWidgets.ResourcePickerThumbnail)]
	protected ResourceName m_TextureLegendButton;
	
	[Attribute("career_traits")]
	protected string m_StringCoreImageLegendButton;
	
	[Attribute("careerCircle")]
	protected string m_StringBackgroundImageLegendButton;
	
	[Attribute("careerCircleOutline")]
	protected string m_StringNotSelectedImageLegendButton;
	
	[Attribute("careerCircleSelected")]
	protected string m_StringSelectedImageLegendButton;
	
	/*
	[Attribute("{F7FD1672FECA05E8}UI/Textures/Icons/icons_gamepad_64.imageset", UIWidgets.ResourcePickerThumbnail)]
	protected ResourceName m_TextureChangeSpButtonKeyboard;
	
	[Attribute("DPAD_left")]
	protected string m_StringChangeSpLeft;
	
	[Attribute("DPAD_right")]
	protected string m_StringChangeSpRight;
	*/
	
	[Attribute("{3093C92AE673BC5E}UI/Textures/Workshop/PagingArrow_UI.edds", UIWidgets.ResourcePickerThumbnail)]
	protected ResourceName m_PagingArrow;
	
	protected bool m_bLeftAndRightUI;
	
	// Arguments passed: this
	protected ref ScriptInvoker m_OnClicked = new ScriptInvoker();
	protected ref ScriptInvoker m_OnMouseEnter = new ScriptInvoker();
	protected ref ScriptInvoker m_OnMouseLeave = new ScriptInvoker();
	
	//------------------------------------------------------------------------------------------------
	protected override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		
		m_wSpecializationText = TextWidget.Cast(w.FindAnyWidget("SpecializationText"));
		if (m_wSpecializationText)
		{
			m_wSpecializationText.SetText(m_sSpecializationText);
			m_wSpecializationText.SetColor(m_ColorInactive);
		}
		
		m_wSpecializationCoreImage = ImageWidget.Cast(w.FindAnyWidget("SpecializationCoreImage"));
		m_wSpecializationBorderImage = ImageWidget.Cast(w.FindAnyWidget("SpecializationBorderImage"));
		m_wSpecializationBackgroundImage = ImageWidget.Cast(w.FindAnyWidget("SpecializationBackgroundImage"));
		if (m_wSpecializationCoreImage && m_wSpecializationBorderImage && m_wSpecializationBackgroundImage)
		{
			SetTexture(m_wSpecializationCoreImage, m_TextureLegendButton, m_StringCoreImageLegendButton+"1");
			SetTexture(m_wSpecializationBorderImage, m_TextureLegendButton, m_StringNotSelectedImageLegendButton);
			SetTexture(m_wSpecializationBackgroundImage, m_TextureLegendButton, m_StringBackgroundImageLegendButton);
			m_wSpecializationCoreImage.SetColor(m_ColorInactive);
			m_wSpecializationBorderImage.SetColor(m_ColorInactive);
			m_wSpecializationBackgroundImage.SetColor(m_ColorBackgroundInactive);
		}
		
		m_wChangeSpLeftLayout = w.FindAnyWidget("ChangeSpLeftSizeLayout");
		m_wChangeSpLeft = ImageWidget.Cast(w.FindAnyWidget("ChangeSpLeft"));
		m_wChangeSpRight = ImageWidget.Cast(w.FindAnyWidget("ChangeSpRight"));
		m_wChangeSpRightLayout = w.FindAnyWidget("ChangeSpRightSizeLayout");
		
		//Buttons on the sides of the legend
		m_bLeftAndRightUI = false;
		
		if (m_wChangeSpLeft && m_wChangeSpRight && m_wChangeSpLeftLayout && m_wChangeSpRightLayout)
		{
			/*
			SetTexture(m_wChangeSpLeft, m_TextureChangeSpButtonKeyboard, m_StringChangeSpLeft);
			SetTexture(m_wChangeSpRight, m_TextureChangeSpButtonKeyboard, m_StringChangeSpRight);
			*/
			SetTexture(m_wChangeSpLeft, m_PagingArrow);
			SetTexture(m_wChangeSpRight, m_PagingArrow);
			
			m_wChangeSpLeft.SetColor(m_ColorActive);
			m_wChangeSpRight.SetColor(m_ColorActive);
			m_wChangeSpLeftLayout.SetVisible(false);
			m_wChangeSpRightLayout.SetVisible(false);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnClicked()
	{
		return m_OnClicked;
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnMouseEnter()
	{
		return m_OnMouseEnter;
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnMouseLeave()
	{
		return m_OnMouseLeave;
	}
	
	//------------------------------------------------------------------------------------------------
	protected override bool OnClick(Widget w, int x, int y, int button)
	{
		super.OnClick(w, x, y, button);
		if (button != 0)
			return false;
		
		m_OnClicked.Invoke(this);
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnFocus(Widget w, int x, int y)
	{
		return false;
		return OnClick(w, x, y, 0);
	}
	
	//------------------------------------------------------------------------------------------------
	protected override bool OnMouseEnter(Widget w, int x, int y)
	{
		super.OnMouseEnter(w, x, y);
		
		m_OnMouseEnter.Invoke(this);
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	protected override bool OnMouseLeave(Widget w, Widget enterW, int x, int y)
	{
		super.OnMouseLeave(w, enterW, x, y);
		
		m_OnMouseLeave.Invoke(this);
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	void Activate(bool animate = true, bool playsound = true)
	{
		if (m_bIsActive)
			return;
		
		m_bIsActive = true;
		if (playsound)
			PlaySound(m_sSoundClicked);
		
		if (animate)
		{
			if (m_wSpecializationCoreImage)
				AnimateWidget.Color(m_wSpecializationCoreImage, m_ColorActive, m_fAnimationRate);
			
			if (m_wSpecializationBorderImage)
				AnimateWidget.Color(m_wSpecializationBorderImage, m_ColorActive, m_fAnimationRate);
			
			if (m_wSpecializationBackgroundImage)
				AnimateWidget.Color(m_wSpecializationBackgroundImage, m_ColorBackgroundActive, m_fAnimationRate);
			
			if (m_wSpecializationText)
				AnimateWidget.Color(m_wSpecializationText, m_ColorActive, m_fAnimationRate);
		}
		else
		{
			if (m_wSpecializationCoreImage)
			{
				AnimateWidget.StopAnimation(m_wSpecializationCoreImage, WidgetAnimationColor);
				m_wSpecializationCoreImage.SetColor(m_ColorActive);
			}
			
			if (m_wSpecializationBorderImage)
			{
				AnimateWidget.StopAnimation(m_wSpecializationBorderImage, WidgetAnimationColor);
				m_wSpecializationBorderImage.SetColor(m_ColorActive);
				SetTexture(m_wSpecializationBorderImage, m_TextureLegendButton, m_StringSelectedImageLegendButton);
			}
			
			if (m_wSpecializationBackgroundImage)
			{
				AnimateWidget.StopAnimation(m_wSpecializationBackgroundImage, WidgetAnimationColor);
				m_wSpecializationBackgroundImage.SetColor(m_ColorBackgroundActive);
			}
			
			if (m_wSpecializationText)
			{
				AnimateWidget.StopAnimation(m_wSpecializationText, WidgetAnimationColor);
				m_wSpecializationText.SetColor(m_ColorActive);
			}
		}
		
		if (m_bLeftAndRightUI && m_wChangeSpLeft && m_wChangeSpRight && m_wChangeSpLeftLayout && m_wChangeSpRightLayout)
		{
			m_wChangeSpLeftLayout.SetVisible(true);
			m_wChangeSpRightLayout.SetVisible(true);
		}	
	}
	
	//------------------------------------------------------------------------------------------------
	void Deactivate(bool animate = true, bool playsound = true)
	{
		if (!m_bIsActive)
			return;
		
		m_bIsActive = false;
		if (playsound)
			PlaySound(m_sSoundClicked);
		
		if (animate)
		{
			if (m_wSpecializationCoreImage)
				AnimateWidget.Color(m_wSpecializationCoreImage, m_ColorInactive, m_fAnimationRate);
			
			if (m_wSpecializationBorderImage)
				AnimateWidget.Color(m_wSpecializationBorderImage, m_ColorInactive, m_fAnimationRate);
			
			if (m_wSpecializationBackgroundImage)
				AnimateWidget.Color(m_wSpecializationBackgroundImage, m_ColorBackgroundInactive, m_fAnimationRate);
			
			if (m_wSpecializationText)
				AnimateWidget.Color(m_wSpecializationText, m_ColorInactive, m_fAnimationRate);
		}
		else
		{
			if (m_wSpecializationCoreImage)
			{
				AnimateWidget.StopAnimation(m_wSpecializationCoreImage, WidgetAnimationColor);
				m_wSpecializationCoreImage.SetColor(m_ColorInactive);
			}
			
			if (m_wSpecializationBorderImage)
			{
				AnimateWidget.StopAnimation(m_wSpecializationBorderImage, WidgetAnimationColor);
				m_wSpecializationBorderImage.SetColor(m_ColorInactive);
				SetTexture(m_wSpecializationBorderImage, m_TextureLegendButton, m_StringNotSelectedImageLegendButton);
			}
			
			if (m_wSpecializationBackgroundImage)
			{
				AnimateWidget.StopAnimation(m_wSpecializationBackgroundImage, WidgetAnimationColor);
				m_wSpecializationCoreImage.SetColor(m_ColorBackgroundInactive);
			}
			
			if (m_wSpecializationText)
			{
				AnimateWidget.StopAnimation(m_wSpecializationText, WidgetAnimationColor);
				m_wSpecializationText.SetColor(m_ColorInactive);
			}
		}
		
		if (m_wChangeSpLeft && m_wChangeSpRight && m_wChangeSpLeftLayout && m_wChangeSpRightLayout)
		{
			m_wChangeSpLeftLayout.SetVisible(false);
			m_wChangeSpRightLayout.SetVisible(false);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void SetOnHover(bool OnHover = true)
	{
		if (m_bIsActive)
			return;
		
		if (OnHover)
		{
			if (m_wSpecializationCoreImage)
				AnimateWidget.Color(m_wSpecializationCoreImage, m_ColorInactiveHovered, m_fAnimationRate);
			
			if (m_wSpecializationBorderImage)
				AnimateWidget.Color(m_wSpecializationBorderImage, m_ColorInactiveHovered, m_fAnimationRate);
			
			if (m_wSpecializationText)
				AnimateWidget.Color(m_wSpecializationText, m_ColorInactiveHovered, m_fAnimationRate);
			return;
		}
		
		if (m_wSpecializationCoreImage)
			AnimateWidget.Color(m_wSpecializationCoreImage, m_ColorInactive, m_fAnimationRate);
		
		if (m_wSpecializationBorderImage)
			AnimateWidget.Color(m_wSpecializationBorderImage, m_ColorInactive, m_fAnimationRate);
			
		if (m_wSpecializationText)
			AnimateWidget.Color(m_wSpecializationText, m_ColorInactive, m_fAnimationRate);
	}
	
	//------------------------------------------------------------------------------------------------
	void Hide()
	{
		m_wRoot.SetOpacity(0);
	}
	
	//------------------------------------------------------------------------------------------------
	void Unhide()
	{
		m_wRoot.SetOpacity(255);
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsActive()
	{
		return m_bIsActive;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetText(string text)
	{
		if (!m_wSpecializationText)
			return;
		
		if (m_sSpecializationText == text)
			return;
		
		m_sSpecializationText = text;
		m_wSpecializationText.SetText(text);
	}
	
	//------------------------------------------------------------------------------------------------
	string GetText()
	{
		return m_sSpecializationText;
	}
	
	//------------------------------------------------------------------------------------------------
	TextWidget GetTextWidget()
	{
		return m_wSpecializationText;
	}
	
	//------------------------------------------------------------------------------------------------
	ImageWidget GetCoreImageWidget()
	{
		return m_wSpecializationCoreImage;
	}
	
	//------------------------------------------------------------------------------------------------
	ImageWidget GetBorderImageWidget()
	{
		return m_wSpecializationBorderImage;
	}
	
	//------------------------------------------------------------------------------------------------
	ImageWidget GetChangeSpLeftImageWidget()
	{
		return m_wChangeSpLeft;
	}
	
	//------------------------------------------------------------------------------------------------
	ImageWidget GetChangeSpRightImageWidget()
	{
		return m_wChangeSpRight;
	}
	
	//------------------------------------------------------------------------------------------------
	bool GetLeftAndRightSelectedUI()
	{
		return m_bLeftAndRightUI;
	}
	
	void SetLeftAndRightUIActivate(bool flag)
	{
		m_bLeftAndRightUI = flag;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetButtonId(int n)
	{
		if (m_wSpecializationCoreImage)
			SetTexture(m_wSpecializationCoreImage, m_TextureLegendButton, m_StringCoreImageLegendButton+(n+1));
		m_ibuttonId = n;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetFocus()
	{
		WorkspaceWidget workspace = GetGame().GetWorkspace();
		if (!workspace)
			return;
		
		GetGame().GetCallqueue().CallLater(workspace.SetFocusedWidget, 1000, false, m_wRoot, true);
	}
	
	//------------------------------------------------------------------------------------------------
	int GetButtonId()
	{
		return m_ibuttonId;
	}
};