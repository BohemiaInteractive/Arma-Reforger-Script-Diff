class SCR_MenuTileComponent : ScriptedWidgetComponent
{
	[Attribute("")]
	protected string m_sTitle;
	
	[Attribute("")]
	protected string m_sDescription;
	
	[Attribute("0.2")]
	protected float m_fAnimationTime;
	
	[Attribute("{C58FCC06AF13075B}UI/Textures/MissionLoadingScreens/placeholder_1.edds", UIWidgets.ResourceNamePicker, "","edds")]
	protected ResourceName m_sImageDefault;
	
	[Attribute("{C58FCC06AF13075B}UI/Textures/MissionLoadingScreens/placeholder_1.edds", UIWidgets.ResourceNamePicker, "","edds")]
	protected ResourceName m_sImageSelected;
	
	[Attribute("", UIWidgets.ResourceNamePicker, "", "")]
	protected ResourceName m_sIconTexture;

	[Attribute()]
	protected string m_sIconImage;

	[Attribute("-1", UIWidgets.SearchComboBox, "", "", ParamEnumArray.FromEnum(ChimeraMenuPreset))]
	ChimeraMenuPreset m_eMenuPreset;

	[Attribute("Title")]
	protected LocalizedString m_sTitleName;

	[Attribute("Description")]
	protected LocalizedString m_sDescriptionName;

	[Attribute("ImageDefault")]
	protected string m_sDefaultWidgetName;

	[Attribute("ImageSelected")]
	protected string m_sSelectedWidgetName;

	[Attribute("Icon")]
	protected string m_sIconImageName;

	protected Widget m_wRoot;
	protected ImageWidget m_wImageDefault;
	protected ImageWidget m_wImageSelected;
	protected ImageWidget m_wIcon;
	protected TextWidget m_wTitle;
	protected TextWidget m_wDescription;
	protected float m_fAnimationRate = 10000;
	
	ref ScriptInvoker m_OnFocused = new ScriptInvoker();
	ref ScriptInvoker m_OnFocusLost = new ScriptInvoker();
	ref ScriptInvoker m_OnClicked = new ScriptInvoker();
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		m_wRoot = w;
		m_wImageDefault = ImageWidget.Cast(w.FindAnyWidget(m_sDefaultWidgetName));
		m_wImageSelected = ImageWidget.Cast(w.FindAnyWidget(m_sSelectedWidgetName));
		m_wIcon = ImageWidget.Cast(w.FindAnyWidget(m_sIconImageName));
		m_wTitle = TextWidget.Cast(w.FindAnyWidget(m_sTitleName));
		m_wDescription = TextWidget.Cast(w.FindAnyWidget(m_sDescriptionName));
		
		SetOrHideImage(m_wImageDefault, m_sImageDefault, string.Empty);
		SetOrHideImage(m_wImageSelected, m_sImageSelected, string.Empty);
		if (!m_sIconImage.IsEmpty())
			SetOrHideImage(m_wIcon, m_sIconTexture, m_sIconImage);

		if (m_wImageSelected)
			m_wImageSelected.SetOpacity(0);
		
		if (m_wDescription)
		{
			if (!m_sDescription.IsEmpty())
			{
				m_wDescription.SetText(m_sDescription);
				m_wDescription.SetOpacity(0);
			}
			else
				m_wDescription.SetVisible(false);
		}
		
		if (m_wTitle)
			m_wTitle.SetText(m_sTitle);
		
		GetGame().GetCallqueue().CallLater(SetAnimationRate, SCR_WLibComponentBase.START_ANIMATION_PERIOD);
	}

	//------------------------------------------------------------------------------------------------
	protected void SetOrHideImage(ImageWidget widget, string texture, string image)
	{
		if (!widget)
			return;
		
		bool success = SCR_WLibComponentBase.SetTexture(widget, texture, image);
		widget.GetParent().SetVisible(success); // Hide parent widget, because it's a scale widget
	}
	
	//------------------------------------------------------------------------------------------------
	private void SetAnimationRate()
	{
		if (m_fAnimationTime <= 0)
			m_fAnimationRate = 1000;
		else
			m_fAnimationRate = 1 / m_fAnimationTime;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnFocus(Widget w, int x, int y)
	{
		AnimateWidget.Opacity(m_wDescription, 1, m_fAnimationRate);
		AnimateWidget.Opacity(m_wImageSelected, 1, m_fAnimationRate);
		m_OnFocused.Invoke(this);
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnFocusLost(Widget w, int x, int y)
	{
		AnimateWidget.Opacity(m_wDescription, 0, m_fAnimationRate);
		AnimateWidget.Opacity(m_wImageSelected, 0, m_fAnimationRate);
		m_OnFocusLost.Invoke(this);
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnClick(Widget w, int x, int y, int button)
	{
		m_OnClicked.Invoke(this);
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	Widget GetRootWidget()
	{
		return m_wRoot;
	}
}
