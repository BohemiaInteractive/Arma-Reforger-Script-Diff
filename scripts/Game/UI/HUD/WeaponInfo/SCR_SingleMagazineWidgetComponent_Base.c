class SCR_SingleMagazineWidgetComponent_Base : SCR_ScriptedWidgetComponent
{
	//Magazine
	[Attribute("m_MagazineGlow")]
	protected string m_sMagazineGlow;
	
	[Attribute("m_MagazineBackground")]
	protected string m_sMagazineBackground;
	
	[Attribute("m_MagazineOutline")]
	protected string m_sMagazineOutline;
	
	[Attribute("m_MagazineProgress")]
	protected string m_sMagazineProgress;
	
	[Attribute("m_ReloadBackground")]
	protected string m_sReloadBackground;
	
	[Attribute("m_ReloadOutline")]
	protected string m_sReloadOutline;
	
	[Attribute("m_MagazineEmpty")]
	protected string m_sMagazineEmpty;
	
	//color sets
	[Attribute()]
	protected ref WeaponInfoColorSet m_IdleColorSet;
	
	[Attribute()]
	protected ref WeaponInfoColorSet m_HighlightColorSet;
	
	[Attribute()]
	protected ref WeaponInfoColorSet m_DisabledColorSet;
	
	//Magazine indicator
	protected ImageWidget m_wMagazineGlow;
	protected ImageWidget m_wMagazineBackground;
	protected ImageWidget m_wMagazineOutline;
	protected ImageWidget m_wMagazineProgress;
	protected ImageWidget m_wMagazineEmpty;
	protected ImageWidget m_wReloadBackground;
	protected ImageWidget m_wReloadOutline;
	
	protected SCR_EWeaponInfoIconState m_eCurrentState = SCR_EWeaponInfoIconState.IDLE;
	
	protected bool m_bIsActive = true, m_bIsSelected;
	
	
	//------------------------------------------------------------------------------------------------
	SCR_EWeaponInfoIconState GetCurrentState()
	{
		return m_eCurrentState;
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		
		//magazine
		m_wMagazineGlow = ImageWidget.Cast(w.FindAnyWidget(m_sMagazineGlow));
		m_wMagazineBackground = ImageWidget.Cast(w.FindAnyWidget(m_sMagazineBackground));
		m_wMagazineOutline = ImageWidget.Cast(w.FindAnyWidget(m_sMagazineOutline));
		m_wMagazineProgress = ImageWidget.Cast(w.FindAnyWidget(m_sMagazineProgress));
		m_wMagazineEmpty = ImageWidget.Cast(w.FindAnyWidget(m_sMagazineEmpty));
		m_wReloadBackground = ImageWidget.Cast(w.FindAnyWidget(m_sReloadBackground));
		m_wReloadOutline = ImageWidget.Cast(w.FindAnyWidget(m_sReloadOutline));
		
	}
	
	//------------------------------------------------------------------------------------------------
	void Init(BaseWeaponComponent weapon, int height, SCR_MagazineIndicatorConfiguration magConfig, bool isSelected)
	{
		SetMagazineTextures(magConfig, height);
		UpdateAmmoCount(weapon);
		SetSelected(isSelected);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetAllSize(int height)
	{
		//magazine icons size
		
		m_wMagazineOutline.SetSize(GetImageDesiredWidth(m_wMagazineOutline, height), height);
		m_wMagazineGlow.SetSize(GetImageDesiredWidth(m_wMagazineGlow, height), height);
		m_wMagazineBackground.SetSize(GetImageDesiredWidth(m_wMagazineBackground, height), height);
		m_wMagazineProgress.SetSize(GetImageDesiredWidth(m_wMagazineProgress, height), height);
		m_wMagazineEmpty.SetSize(GetImageDesiredWidth(m_wMagazineEmpty, height), height);
		m_wReloadBackground.SetSize(GetImageDesiredWidth(m_wReloadBackground, height), height);
		m_wReloadOutline.SetSize(GetImageDesiredWidth(m_wReloadOutline, height), height);
	}
	
	//------------------------------------------------------------------------------------------------
	protected float GetImageDesiredWidth(ImageWidget w, int height)
	{
		int sx, sy;
		float ratio;
		
		w.GetImageSize(0, sx, sy);
		
		if (sy == 0)
			ratio = 0;
		else
			ratio = sx / sy;
		
		return height * ratio;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetMagazineTextures(SCR_MagazineIndicatorConfiguration config, int height)
	{
		if (!config)
		{
			SetAllSize(height);
			return;
		}
		
		if (config.m_bProgressBar)
		{
			SetWidgetImage(m_wMagazineOutline, config.m_sImagesetIcons, config.m_sOutline);
			SetWidgetImage(m_wMagazineBackground, config.m_sImagesetIcons, config.m_sBackground);
			SetWidgetImage(m_wMagazineGlow, config.m_sImagesetGlows, config.m_sBackground);			
		}
		else
		{
			SetWidgetImage(m_wMagazineOutline, config.m_sImagesetIcons, config.m_sOutline);
			SetWidgetImage(m_wMagazineGlow, config.m_sImagesetGlows, config.m_sOutline);			
		}
		
		SetWidgetImage(m_wMagazineEmpty, config.m_sImagesetIcons, config.m_sEmptyMagazine);
		
		// Setup textures for reloading indicator
		SetWidgetImage(m_wReloadBackground, config.m_sImagesetIcons, config.m_sProgress);
		m_wReloadBackground.SetVisible(false);			
		SetWidgetImage(m_wReloadOutline, config.m_sImagesetIcons, config.m_sOutline);
		m_wReloadOutline.SetVisible(false);

		
		// Setup visibility, keep only Outline texture, if there is no progress indication
		m_wMagazineProgress.SetVisible(config.m_bProgressBar);
		m_wMagazineBackground.SetVisible(config.m_bProgressBar);			
		m_wMagazineOutline.SetOpacity(1);
					
		
		if (config.m_bProgressBar)
		{
			SetWidgetImage(m_wMagazineProgress, config.m_sImagesetIcons, config.m_sProgress);
			m_wMagazineProgress.LoadMaskFromSet(config.m_sImagesetIcons, config.m_sProgressAlphaMask);
			m_wMagazineProgress.SetMaskMode(ImageMaskMode.REGULAR);
		}
		
		SetAllSize(height);
		//AnimateWidget_ColorFlash(m_Widgets.m_wMagazineIndicator);
	}	
	
	//---------------------------------------------------------------------------------------------------------
	//! Sets widget's image to an image or imageset
	protected void SetWidgetImage(ImageWidget w, string imageOrImageset, string imageName = "", int size = -1)
	{
		if (!imageName.IsEmpty())
		{
			// Assume it's an image set
			w.LoadImageFromSet(0, imageOrImageset, imageName);
		}
		else if (!imageOrImageset.IsEmpty())
		{
			// Assume it's an image
			w.LoadImageTexture(0, imageOrImageset);
		}

		if (size == -1)
			return;
				
		// Perform resizing
		int sx, sy;
		w.GetImageSize(0, sx, sy);
		
		float ratio = sx / sy;
		
		w.SetSize(size * ratio, size);
	}
	
	//DELEGATED TO CHILDREN
	//------------------------------------------------------------------------------------------------
	void UpdateAmmoCount(BaseWeaponComponent weapon);
	
	//------------------------------------------------------------------------------------------------
	protected void SetColors(WeaponInfoColorSet colorSet)
	{
		m_wMagazineOutline.SetColor(colorSet.m_IconColor);
		m_wMagazineProgress.SetColor(colorSet.m_IconColor);
		m_wMagazineEmpty.SetColor(colorSet.m_IconColor);
		m_wMagazineBackground.SetColor(colorSet.m_BackgroundColor);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetState(SCR_EWeaponInfoIconState newState)
	{
		m_eCurrentState = newState;
		
		switch (m_eCurrentState){
			case SCR_EWeaponInfoIconState.IDLE:
				SetColors(m_IdleColorSet);
				break;
			
			case SCR_EWeaponInfoIconState.HIGHLIGHTED:
				SetColors(m_HighlightColorSet);
				break;
			
			case SCR_EWeaponInfoIconState.DISABLED:
				SetColors(m_DisabledColorSet);
				break;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void SetSelected(bool selected)
	{
		if (m_eCurrentState != SCR_EWeaponInfoIconState.DISABLED)
		{
			if (selected)
				SetState(SCR_EWeaponInfoIconState.HIGHLIGHTED);
			else
				SetState(SCR_EWeaponInfoIconState.IDLE);	
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void SetActive(bool active)
	{
		if (active)
			SetState(SCR_EWeaponInfoIconState.IDLE);
		else
			SetState(SCR_EWeaponInfoIconState.DISABLED);
	}
	
	//------------------------------------------------------------------------------------------------
	void DestroyMagazine()
	{
		m_wRoot.RemoveFromHierarchy();
	}
}