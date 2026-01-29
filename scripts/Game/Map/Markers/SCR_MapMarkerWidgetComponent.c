//! Map marker layout component
//! Attached to root of marker base layout
class SCR_MapMarkerWidgetComponent : SCR_ScriptedWidgetComponent
{
	protected const string PRIVATE_QUAD = "private";
	protected const string PUBLIC_QUAD = "public";
	
	protected const string AUTHOR_NAME_FORMAT = "[%1]";
	
	protected const int UPDATE_TIMESTAMP_INTERVAL = 10000; //! in milliseconds

	protected bool m_bIsEventListening;	// whether this marker reacts to events
	protected bool m_bIsSymbolMode;		// app-6 symbol visualization mode
	protected bool m_bIsOwnerMode;		// player is the markers owner
	protected int m_iLayerID;			// map layer ID
	
	protected WorldTimestamp m_Timestamp;
	protected bool m_bIsTimestampVisible;

	protected ImageWidget m_wMarkerIcon;
	protected ImageWidget m_wMarkerGlowIcon;
	protected ImageWidget m_wMarkerModeIcon;
	protected ImageWidget m_wAuthorPlatformIcon;
	protected TextWidget m_wMarkerText;
	protected TextWidget m_wMarkerAuthor;
	protected TextWidget m_wTypeIcon1;
	protected TextWidget m_wTypeIcon2;
	protected TextWidget m_wTypeIcon3;
	protected Widget m_wSymbolRoot;
	protected Widget m_wSymbolOverlay;
	protected Widget m_wTypeIconRoot;
	protected Widget m_wTypeOverlay1;
	protected Widget m_wTypeOverlay2;
	protected Widget m_wTypeOverlay3;
	protected TextWidget m_wMarkerTimestamp;
	
	protected ref Color m_GlowDefault  = Color.FromSRGBA(21, 29, 32, 155);
	protected ref Color m_GlowSelected = Color.FromSRGBA(226, 168, 79, 155);
	
	protected ref Color m_TextColor = new Color(0.0, 0.0, 0.0, 1.0);
	protected ref Color m_CurrentImageColor = new Color(0.0, 0.0, 0.0, 1.0);
	protected SCR_MapMarkerBase m_MarkerObject;
	
	protected SCR_MapMarkerManagerComponent m_MapMarkerManager;
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] marker
	void SetMarkerObject(notnull SCR_MapMarkerBase marker)
	{
		m_MarkerObject = marker;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] id
	void SetLayerID(int id)
	{
		m_iLayerID = id;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] angle
	void SetRotation(float angle)
	{
		m_wMarkerIcon.SetRotation(angle);
		m_wMarkerGlowIcon.SetRotation(angle);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Supports custom aspect ratio in case of non standard size imagesets
	//! \param[in] icon
	//! \param[in] quad
	//! \param[in] aspectRatio
	void SetImage(ResourceName icon, string quad, float aspectRatio = 1)
	{
		m_wMarkerIcon.LoadImageFromSet(0, icon, quad);
		if (aspectRatio != 1 && aspectRatio != 0)
		{
			vector size = m_wMarkerIcon.GetSize();
			m_wMarkerIcon.SetSize(size[0] * 0.9, (size[1] * (1/aspectRatio)) * 0.9); // todo, temp size adjust before symbols group side are fixed
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] icon
	//! \param[in] quad
	void SetGlowImage(ResourceName icon, string quad)
	{
		m_wMarkerGlowIcon.SetVisible(true);
		m_wMarkerGlowIcon.LoadImageFromSet(0, icon, quad);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set visual mode for military symbol which is constructed through additional component
	//! \param[in] state
	void SetMilitarySymbolMode(bool state)
	{
		m_bIsSymbolMode = state;
		
		m_wSymbolRoot.SetEnabled(state);
		m_wSymbolRoot.SetVisible(state);
		
		m_wMarkerIcon.SetVisible(!state);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] state
	void SetEventListening(bool state)
	{
		m_bIsEventListening = state;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] milSymbol
	void UpdateMilitarySymbol(SCR_MilitarySymbol milSymbol)
	{
		SCR_MilitarySymbolUIComponent symbolComp = SCR_MilitarySymbolUIComponent.Cast(m_wSymbolOverlay.FindHandler(SCR_MilitarySymbolUIComponent));
		if (symbolComp)
			symbolComp.Update(milSymbol);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] text
	//! \param[in] skipProfanityFilter determines whether the marker text should be checked for profanity
	void SetText(string text, bool skipProfanityFilter = false)
	{
		if (skipProfanityFilter || text.IsEmpty() || !m_MarkerObject.GetMarkerOwnerID() <= -1)
		{
			OnFilteredCallback({text});
			return;
		}
		
		if (!m_MapMarkerManager)
			return;
		
		SCR_ScriptProfanityFilterRequestCallback profanityCallback = m_MapMarkerManager.RequestProfanityFilter(text);
		
		if (!profanityCallback)
		{
			OnFilteredCallback({text});
			return;
		}
		
		profanityCallback.m_OnResult.Insert(OnFilteredCallback);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnFilteredCallback(array<string> text)
	{
		string resultText;
		
		if (GetGame().GetPlatformService().GetLocalPlatformKind() == PlatformKind.XBOX == PlatformKind.XBOX)
		{
			SCR_ProfaneFilter.ReplaceProfanities(text.Get(0), resultText);
		}
		else
		{
			resultText = text.Get(0);
		}
		
		if (!resultText.IsEmpty())
			m_wMarkerText.SetText(resultText);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] state
	void SetTextVisible(bool state)
	{
		m_wMarkerText.SetVisible(state);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Not synched secondary text
	void SetTypeIcon(int type, string text)
	{
		if (type == 1)
		{
			m_wTypeIcon1.SetText(text);
			m_wTypeOverlay1.SetVisible(true);
		}
		else if (type == 2)
		{
			m_wTypeIcon2.SetText(text);
			m_wTypeOverlay2.SetVisible(true);
		}
		else if (type == 3)
		{
			m_wTypeIcon3.SetText(text);
			m_wTypeOverlay3.SetVisible(true);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] state
	void SetTypeIconsVisible(bool state)
	{
		m_wTypeIconRoot.SetVisible(state);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] text
	void SetAuthor(string text)
	{
		m_bIsOwnerMode = false;
		
		m_wMarkerAuthor.SetTextFormat(AUTHOR_NAME_FORMAT, text);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] state
	void SetAuthorVisible(bool state)
	{
		m_wMarkerAuthor.SetVisible(state);
		m_wAuthorPlatformIcon.SetVisible(state);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] state
	//! \param[in] isPublic
	void SetModeIcon(bool state, bool isPublic)
	{		
		m_bIsOwnerMode = true;
		
		m_wMarkerModeIcon.SetVisible(m_MarkerObject.GetMarkerOwnerID() == SCR_PlayerController.GetLocalPlayerId());
		
		if (isPublic)
			m_wMarkerModeIcon.LoadImageFromSet(0, UIConstants.ICONS_IMAGE_SET, PUBLIC_QUAD);
		else 
			m_wMarkerModeIcon.LoadImageFromSet(0, UIConstants.ICONS_IMAGE_SET, PRIVATE_QUAD);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] color
	void SetColor(Color color)
	{
		m_CurrentImageColor = color;
		
		if (m_bIsSymbolMode)
			m_wSymbolOverlay.SetColor(color);
		else 
			m_wMarkerIcon.SetColor(color);
			
		m_wMarkerText.SetColor(m_TextColor);
		m_wMarkerAuthor.SetColor(m_TextColor);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	ImageWidget GetAuthorPlatformIcon()
	{
		return m_wAuthorPlatformIcon;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] timestamp
	void SetTimestamp(WorldTimestamp timestamp)
	{
		if (!timestamp)
			return;

		m_Timestamp = timestamp;
		GetGame().GetCallqueue().CallLater(UpdateCachedTimestamp, UPDATE_TIMESTAMP_INTERVAL, true);
		UpdateTimestamp(timestamp);
	}

	//------------------------------------------------------------------------------------------------
	void SetTimestampVisibility(bool isVisible)
	{
		m_bIsTimestampVisible = isVisible;
		m_wMarkerTimestamp.SetVisible(isVisible);
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateCachedTimestamp()
	{
		UpdateTimestamp(m_Timestamp);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] timestamp
	void UpdateTimestamp(WorldTimestamp timestamp)
	{
		if (!timestamp)
			return;

		ChimeraWorld world = GetGame().GetWorld();
		if (!world)
			return;

		string text = SCR_FormatHelper.GetTimeSinceEventImprecise(world.GetServerTimestamp().DiffSeconds(timestamp));
		m_wMarkerTimestamp.SetText(text);
	}

	//------------------------------------------------------------------------------------------------
	override bool OnMouseEnter(Widget w, int x, int y)
	{
		if (!m_bIsEventListening || !m_MarkerObject)
			return false;
				
		m_MarkerObject.LayerChangeLogic(0);
		
		SetTypeIconsVisible(true);
		
		if (!SCR_MapMarkersUI.IsOwnedMarker(m_MarkerObject))
			return false;
		
		if (m_bIsSymbolMode)
			m_wSymbolOverlay.SetColor(GUIColors.ORANGE);
		else
			m_wMarkerGlowIcon.SetColor(m_GlowSelected);
				
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnMouseLeave(Widget w, Widget enterW, int x, int y)
	{
		if (!m_bIsEventListening || !m_MarkerObject)
			return false;
			
		m_MarkerObject.LayerChangeLogic(m_iLayerID);

		SetTypeIconsVisible(false);
		
		if (!SCR_MapMarkersUI.IsOwnedMarker(m_MarkerObject))
			return false;
		
		if (m_bIsSymbolMode)
			m_wSymbolOverlay.SetColor(m_CurrentImageColor);
		else 
			m_wMarkerGlowIcon.SetColor(m_GlowDefault);
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnFocus(Widget w, int x, int y)
	{
		return OnMouseEnter(w, x, y);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnFocusLost(Widget w, int x, int y)
	{
		return OnMouseLeave(w, null, x, y);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnMapClose(MapConfiguration config)
	{
		SCR_MapEntity.GetOnMapClose().Remove(OnMapClose);
		GetGame().GetCallqueue().Remove(UpdateCachedTimestamp);
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		
		m_wMarkerIcon = ImageWidget.Cast(m_wRoot.FindAnyWidget("MarkerIcon"));
		m_wMarkerGlowIcon = ImageWidget.Cast(m_wRoot.FindAnyWidget("MarkerIconGlow"));
		m_wMarkerText = TextWidget.Cast(m_wRoot.FindAnyWidget("MarkerText"));
		m_wMarkerAuthor = TextWidget.Cast(m_wRoot.FindAnyWidget("MarkerAuthor"));
		m_wMarkerModeIcon = ImageWidget.Cast(m_wRoot.FindAnyWidget("MarkerModeIcon"));
		m_wAuthorPlatformIcon = ImageWidget.Cast(m_wRoot.FindAnyWidget("PlatformIcon"));
		m_wSymbolRoot = m_wRoot.FindAnyWidget("SymbolWidget");
		m_wSymbolOverlay = m_wSymbolRoot.FindAnyWidget("SymbolOverlay");
		m_wTypeIconRoot = m_wRoot.FindAnyWidget("TypeIconHLayout");
		m_wTypeOverlay1 = m_wTypeIconRoot.FindAnyWidget("TypeOverlay1");
		m_wTypeOverlay2 = m_wTypeIconRoot.FindAnyWidget("TypeOverlay2");
		m_wTypeOverlay3 = m_wTypeIconRoot.FindAnyWidget("TypeOverlay3");
		m_wTypeIcon1 = TextWidget.Cast(m_wTypeIconRoot.FindAnyWidget("TypeText1"));
		m_wTypeIcon2 = TextWidget.Cast(m_wTypeIconRoot.FindAnyWidget("TypeText2"));
		m_wTypeIcon3 = TextWidget.Cast(m_wTypeIconRoot.FindAnyWidget("TypeText3"));
		
		m_MapMarkerManager = SCR_MapMarkerManagerComponent.GetInstance();

		m_wMarkerTimestamp = TextWidget.Cast(m_wRoot.FindAnyWidget("MarkerTimestamp"));
		m_wMarkerTimestamp.SetText(string.Empty); // set empty

		if (m_Timestamp)
			GetGame().GetCallqueue().CallLater(UpdateCachedTimestamp, UPDATE_TIMESTAMP_INTERVAL, true);

		SCR_MapEntity.GetOnMapClose().Insert(OnMapClose);
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		super.HandlerDeattached(w);

		GetGame().GetCallqueue().Remove(UpdateCachedTimestamp);
	}
}
