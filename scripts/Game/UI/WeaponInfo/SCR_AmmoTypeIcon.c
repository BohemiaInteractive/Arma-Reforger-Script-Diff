class SCR_AmmoTypeIcon : SCR_ScriptedWidgetComponent
{
	[Attribute("Icon")]
	protected string m_sIcon;
	
	[Attribute("Glow")]
	protected string m_sGlow;
	
	[Attribute()]
	protected ref WeaponInfoColorSet m_IdleColorSet;
	
	[Attribute()]
	protected ref WeaponInfoColorSet m_HighlightColorSet;
	
	[Attribute()]
	protected ref WeaponInfoColorSet m_DisabledColorSet;
	
	protected ImageWidget m_wIconWidget;
	protected ImageWidget m_wGlowWidget;
	
	protected ref SCR_AmmoTypeInfoConfig m_AmmoTypeIcons;
	protected SCR_EWeaponInfoIconState m_currentState;
	
	//------------------------------------------------------------------------------------------------
	protected override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		
		m_wIconWidget = ImageWidget.Cast(w.FindAnyWidget(m_sIcon));
		m_wGlowWidget = ImageWidget.Cast(w.FindAnyWidget(m_sGlow));
	}
	
	//------------------------------------------------------------------------------------------------
	void Init(EAmmoType ammoType, SCR_AmmoTypeInfoConfig ammoTypeConfig)
	{
		m_AmmoTypeIcons = ammoTypeConfig;
		SetIcon(ammoType);
	}
	
	//------------------------------------------------------------------------------------------------
	void DeleteWidgets()
	{
		m_wRoot.RemoveFromHierarchy();
	}
	
	//------------------------------------------------------------------------------------------------
	void SetIcon(EAmmoType ammoType)
	{
		if (m_AmmoTypeIcons)
			m_AmmoTypeIcons.SetIconAndGlowTo(ammoType, m_wIconWidget, m_wGlowWidget);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetIconsConfig(SCR_AmmoTypeInfoConfig newConfig)
	{
		m_AmmoTypeIcons = newConfig;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetColor(WeaponInfoColorSet colorSet, bool animate)
	{		
		AnimateWidget.StopAllAnimations(m_wIconWidget);
		AnimateWidget.StopAllAnimations(m_wGlowWidget);
		
		if (animate)
		{
			AnimateWidget.Color(m_wIconWidget, colorSet.m_IconColor, UIConstants.FADE_RATE_SLOW);
			AnimateWidget.Color(m_wGlowWidget, colorSet.m_GlowColor, UIConstants.FADE_RATE_SLOW);
		}
		else
		{
			m_wIconWidget.SetColor(colorSet.m_IconColor);
			m_wGlowWidget.SetColor(colorSet.m_GlowColor);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void SetNewState(SCR_EWeaponInfoIconState newState)
	{
		if (m_currentState == newState)
			return;

		m_currentState = newState;
		switch (newState)
		{
			case SCR_EWeaponInfoIconState.IDLE:
				SetColor(m_IdleColorSet, false);
				break;
			
			case SCR_EWeaponInfoIconState.HIGHLIGHTED:
				SetColor(m_HighlightColorSet, false);
				break;
			
			case SCR_EWeaponInfoIconState.DISABLED:
				SetColor(m_DisabledColorSet, true);
				break;
		}	
	}
	
	//------------------------------------------------------------------------------------------------
	void SetVisibility(bool newVis)
	{
		m_wIconWidget.SetVisible(newVis);
		m_wGlowWidget.SetVisible(newVis);
		
		//AnimateWidget.Opacity(m_wIconWidget, newVis, UIConstants.FADE_RATE_DEFAULT, true);
		//AnimateWidget.Opacity(m_wGlowWidget, newVis, UIConstants.FADE_RATE_DEFAULT, true);
	}
}















