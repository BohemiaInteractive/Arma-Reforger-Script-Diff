class SCR_LocalPerceivedFactionWidgetComponent : SCR_ScriptedWidgetComponent
{
	[Attribute()]
	protected ref Color m_DefaultColor;
	
	[Attribute()]
	protected ref Color m_LightWarningColor;
	
	[Attribute()]
	protected ref Color m_WarningColor;
	
	[Attribute("1")]
	protected bool m_bShowIcon;
	
	[Attribute("1")]
	protected bool m_bShowIfDefaultFaction;
	
	[Attribute("0")]
	protected bool m_bColorOutline;
	
	[Attribute("{0B03F611D52043AE}UI/Textures/placeholder.edds", desc: "Icon if perceived faction is unknown")]
	protected ResourceName m_sUnknownFactionIcon;
	
	protected const string FACTION_ICON_HOLDER = "IconHolder";
	protected const string FACTION_ICON_NAME = "FactionIcon";
	protected const string EYE_ICON_NAME = "EyeIcon";
	protected const string OUTLINE_NAME = "Outline";
	protected const string FILTER_NAME = "Filter";
	
	protected Widget m_wFactionIconHolder;
	protected ImageWidget m_wFactionIcon;
	protected ImageWidget m_wEyeIcon;
	protected Widget m_wOutline;
	protected Widget m_wFilter;
	
	SCR_CharacterFactionAffiliationComponent m_CharFactionAffiliation;
	
	//------------------------------------------------------------------------------------------------
	protected void OnOnPerceivedFactionChanged(Faction faction)
	{
		//~ Character is not a player (Or is possessed AI) so never show perceived faction
		if (!m_CharFactionAffiliation || !m_CharFactionAffiliation.HasPerceivedFaction())
		{
			GetRootWidget().SetVisible(false);
			return;
		}
		
		//~ If player is punished for killing while disguised set color to warning otherwise set to light warning
		SCR_PerceivedFactionManagerComponent perceivedFactionManager = SCR_PerceivedFactionManagerComponent.GetInstance();
		if (perceivedFactionManager && perceivedFactionManager.GetPunishmentKillingWhileDisguisedFlags() == 0)
			SetWidgetColor(m_LightWarningColor);
		else 
			SetWidgetColor(m_WarningColor);
		
		//~ Get player faction
		Faction playerFaction = SCR_PlayerController.GetLocalMainEntityFaction();
		
		//~ No player faction so hide UI
		if (!playerFaction)
		{
			GetRootWidget().SetVisible(false);
			return;
		}
		
		//~ Unknown faction
		if (!faction)
		{
			GetRootWidget().SetVisible(true);
			m_wFactionIcon.LoadImageTexture(0, m_sUnknownFactionIcon);
			
			return;
		}
		
		//~ Player perceived faction is default faction
		if (playerFaction == faction)
		{
			//~ Do not show default faction
			if (!m_bShowIfDefaultFaction)
			{
				GetRootWidget().SetVisible(false);
				return;
			}
			//~ Show default faction
			else 
			{
				GetRootWidget().SetVisible(true);
				SetWidgetColor(m_DefaultColor);
			}
		}
		
		//~ Set perceived faction icon
		m_wFactionIcon.LoadImageTexture(0, faction.GetUIInfo().GetIconPath());
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetWidgetColor(Color color)
	{
		//if (m_wEyeIcon)
		//	m_wEyeIcon.SetColor(color);
		
		if (m_wFilter)
			m_wFilter.SetColor(color);
		
		if (m_bColorOutline && m_wOutline)
			m_wOutline.SetColor(color);
	}
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		
		if (SCR_Global.IsEditMode())
			return;
		
		m_wOutline = w.FindAnyWidget(OUTLINE_NAME);
		m_wFilter = w.FindAnyWidget(FILTER_NAME);
		
		//~ Faction icon
		m_wFactionIconHolder = w.FindAnyWidget(FACTION_ICON_HOLDER);
		if (!m_wFactionIconHolder)
		{
			Print("SCR_LocalPerceivedFactionWidgetComponent is missing Icon holder!", LogLevel.ERROR);
			return;
		}
		
		m_wFactionIcon = ImageWidget.Cast(m_wFactionIconHolder.FindAnyWidget(FACTION_ICON_NAME));
		if (!m_wFactionIcon)
		{
			Print("SCR_LocalPerceivedFactionWidgetComponent is missing faction icon widget!", LogLevel.ERROR);
			return;
		}
		
		m_wEyeIcon = ImageWidget.Cast(m_wFactionIconHolder.FindAnyWidget(EYE_ICON_NAME));
		if (!m_wEyeIcon)
			Print("SCR_LocalPerceivedFactionWidgetComponent is missing Eye Icon", LogLevel.WARNING);
		
		SCR_PerceivedFactionManagerComponent perceivedManager = SCR_PerceivedFactionManagerComponent.GetInstance();
		if (!perceivedManager || perceivedManager.GetCharacterPerceivedFactionOutfitType() == SCR_EPerceivedFactionOutfitType.DISABLED)
		{
			w.SetVisible(false);
			return;
		}
		
		IEntity player = SCR_PlayerController.GetLocalControlledEntity();
		if (!player)
		{
			w.SetVisible(false);
			m_CharFactionAffiliation = null;
			return;
		}
		
		m_CharFactionAffiliation = SCR_CharacterFactionAffiliationComponent.Cast(player.FindComponent(SCR_CharacterFactionAffiliationComponent));
		if (!m_CharFactionAffiliation || !m_CharFactionAffiliation.HasPerceivedFaction())
		{
			w.SetVisible(false);
			m_CharFactionAffiliation = null;
			return;
		}	
		
		m_CharFactionAffiliation.GetOnOnPerceivedFactionChanged().Insert(OnOnPerceivedFactionChanged);
		OnOnPerceivedFactionChanged(m_CharFactionAffiliation.GetPerceivedFaction());
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		super.HandlerDeattached(w);
		
		if (m_CharFactionAffiliation)
			m_CharFactionAffiliation.GetOnOnPerceivedFactionChanged().Remove(OnOnPerceivedFactionChanged);
	}
}
