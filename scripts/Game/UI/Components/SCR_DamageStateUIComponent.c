class SCR_DamageStateUIComponent : ScriptedWidgetComponent
{
	[Attribute(desc: "Config to get visuals from", params: "conf class=SCR_DamageStateConfig")]
	protected ref SCR_DamageStateConfig m_DamageStateConfig;
	
	[Attribute(desc: "If true will auto set icon, background and outline for the given type as well as colors")]
	protected bool m_bAutoSetVisuals;
	
	[Attribute(desc: "Damage type of visuals. Will be set automatically m_bAutoSetVisuals = true. Will be ignored if custom is set", uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(EDamageType))]
	protected EDamageType m_eDamageType;
	
	[Attribute("Icon", desc: "Icon to set")]
	protected string m_sIconName;
	
	[Attribute("Outline", desc: "Outline to set")]
	protected string m_sOutlineName;
	
	[Attribute("Background", desc: "Background to set")]
	protected string m_sBackgroundName;
	
	protected SCR_DamageStateUIInfo m_UiInfo;
	
	protected Widget m_wRoot;
	
	/*!
	Set visuals using EDamageType
	\param damageType Damage type of the visual that needs to be obtained
	*/
	void SetVisuals(EDamageType damageType)
	{
		if (!m_DamageStateConfig)
			return;
		
		m_eDamageType = damageType;
		
		SetVisuals(m_DamageStateConfig.GetUiInfo(damageType));
	}
	
	/*!
	Set visuals using SCR_DamageStateUIInfo
	\param uiInfo Visuals to set
	*/
	void SetVisuals(SCR_DamageStateUIInfo uiInfo, int iconIndex = -1)
	{
		m_UiInfo = uiInfo;
		
		m_wRoot.SetVisible(m_UiInfo != null);
		
		if (!m_UiInfo)
			return;
		
		ImageWidget icon = ImageWidget.Cast(m_wRoot.FindAnyWidget(m_sIconName));
		ImageWidget outline = ImageWidget.Cast(m_wRoot.FindAnyWidget(m_sOutlineName));
		ImageWidget background = ImageWidget.Cast(m_wRoot.FindAnyWidget(m_sBackgroundName));
		
		ResourceName image;
		string quadName;

		if (icon)
		{
			icon.SetVisible(m_UiInfo.SetIconTo(icon, iconIndex));
			icon.SetColor(m_UiInfo.GetColor());
		}
		
		if (outline)
		{
			image = m_UiInfo.GetOutlineImage();
			quadName = m_UiInfo.GetOutlineQuadName();
			
			outline.SetVisible(m_UiInfo.SetImageTo(outline, image, quadName));
			outline.SetColor(m_UiInfo.GetOutlineColor());
		}

		if (background)
		{
			image = m_UiInfo.GetBackgroundImage();
			quadName = m_UiInfo.GetBackgroundQuadName();

			background.SetVisible(m_UiInfo.SetImageTo(background, image, quadName));
			background.SetColor(m_UiInfo.GetBackgroundColor());
		}
	}

	/*!
	Set size of images
	\param size Size of images
	*/
	void SetSize(float size)
	{
		ImageWidget icon = ImageWidget.Cast(m_wRoot.FindAnyWidget(m_sIconName));
		ImageWidget outline = ImageWidget.Cast(m_wRoot.FindAnyWidget(m_sOutlineName));
		ImageWidget background = ImageWidget.Cast(m_wRoot.FindAnyWidget(m_sBackgroundName));
		
		if (icon)
			icon.SetSize(size, size);
		if (outline)
			outline.SetSize(size, size);
		if (background)
			background.SetSize(size, size);
	}
	
	/*!
	Get set UI info to obtain all releative visual information and strings
	\return UiInfo that is set.
	*/
	SCR_DamageStateUIInfo GetUiInfo()
	{
		return m_UiInfo;
	}
	
	override void HandlerAttached(Widget w)
	{		
		m_wRoot = w;
		
		if (!m_DamageStateConfig)
		{
			Print("'SCR_DamageStateUIComponent' is missing 'SCR_DamageStateConfig'", LogLevel.ERROR);
			return;
		}

		if (m_bAutoSetVisuals)
			SetVisuals(m_eDamageType);
	}
};
