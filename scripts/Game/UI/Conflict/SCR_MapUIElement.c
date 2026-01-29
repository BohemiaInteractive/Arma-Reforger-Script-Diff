enum SCR_EIconType
{
	NONE,
	BASE,
	ENEMY_BASE,
	RELAY,
	TASK,
	SERVICE,
	SPAWNPOINT
};

class SCR_MapUIElement : SCR_ScriptedWidgetComponent
{
	protected SCR_MapUIElementContainer m_Parent;
	protected static SCR_MapUIElement s_SelectedElement;

	protected MapItem m_MapItem; // todo@lk: delet

	protected ImageWidget m_wImage;
	protected ImageWidget m_wGradient;
	protected ImageWidget m_wSelectImg;
	protected ImageWidget m_wHighlightImg;

	protected bool m_bIsSelected;
	protected const float ANIM_SPEED = 20;
	protected SCR_EIconType m_eIconType;
	protected bool m_bVisible = false;

	[Attribute("{8479B3B5347DF5CF}UI/Imagesets/MilitarySymbol/ID_D.imageset")]
	protected ResourceName m_sImageSetARO;
	
	[Attribute("{27F2439D610D02B3}UI/Imagesets/MilitarySymbol/ICO_Land.imageset")]
	protected ResourceName m_sImageSetSpecial;

	[Attribute("1")]
	protected bool m_bUseBackgroundGradient;

	[Attribute(SCR_SoundEvent.SOUND_MAP_HOVER_BASE)]
	protected string m_sSoundBase;

	[Attribute(SCR_SoundEvent.SOUND_MAP_HOVER_ENEMY)]
	protected string m_sSoundEnemyBase;

	[Attribute(SCR_SoundEvent.SOUND_MAP_HOVER_TRANS_TOWER)]
	protected string m_sSoundRelay;

	[Attribute(SCR_SoundEvent.SOUND_FE_BUTTON_HOVER)]
	protected string m_sSoundTask;

	[Attribute(SCR_SoundEvent.SOUND_FE_BUTTON_HOVER)]
	protected string m_sSoundService;

	[Attribute(SCR_SoundEvent.SOUND_FE_BUTTON_HOVER)]
	protected string m_sSoundHover;

	[Attribute()]
	ref Color m_UnknownFactionColor;

	protected string m_sName;

	//------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		m_wRoot = w;

		m_wImage = ImageWidget.Cast(w.FindAnyWidget("Image"));
		m_wSelectImg = ImageWidget.Cast(w.FindAnyWidget("Corners"));
		m_wHighlightImg = ImageWidget.Cast(w.FindAnyWidget("Highlight"));

		if (m_bUseBackgroundGradient)
			m_wGradient = ImageWidget.Cast(w.FindAnyWidget("BackgroundGradient"));
	}

	//------------------------------------------------------------------------------
	void SetParent(SCR_MapUIElementContainer parent)
	{
		m_Parent = parent;
	}

	//------------------------------------------------------------------------------
	string GetName()
	{
		return m_sName;
	}
	
	//------------------------------------------------------------------------------
	bool GetIconVisible()
	{
		return m_bVisible;
	}


	//------------------------------------------------------------------------------
	void ShowName(bool visible)
	{
	}

	//------------------------------------------------------------------------------
	void SetVisible(bool visible)
	{
		m_wRoot.SetVisible(visible);
	}

	//------------------------------------------------------------------------------
	Widget GetRoot()
	{
		return m_wRoot;
	}


	// //------------------------------------------------------------------------------
	// override bool OnMouseEnter(Widget w, int x, int y)
	// {

	// 	return false;
	// }

	// //------------------------------------------------------------------------------
	// override bool OnMouseLeave(Widget w, Widget enterW, int x, int y)
	// {

	// 	return false;
	// }

	// ------------------------------------------------------------------------------
	override bool OnClick(Widget w, int x, int y, int button)
	{
		if (m_bVisible)
			SelectIcon();

		return false;
	}

	// ------------------------------------------------------------------------------
	void SelectIcon(bool invoke = true)
	{
	}

	// ------------------------------------------------------------------------------
	void Select(bool select = true)
	{
		s_SelectedElement = this;
		m_bIsSelected = select;
		if (m_wSelectImg)
			m_wSelectImg.SetVisible(select);
		if (select)
		{
			s_SelectedElement = this;
			AnimExpand();
		}
		else
		{
			AnimCollapse();
		}
		if (m_wGradient)
			m_wGradient.SetVisible(select);
	}

	//------------------------------------------------------------------------------
	protected void AnimExpand()
	{
	}

	//------------------------------------------------------------------------------
	protected void AnimCollapse()
	{
	}

	//------------------------------------------------------------------------------
	protected void PlayHoverSound(string sound)
	{
		if (sound != string.Empty)
			SCR_UISoundEntity.SoundEvent(sound);
	}

	//------------------------------------------------------------------------------
	vector GetPos()
	{
	}

	//------------------------------------------------------------------------------
	protected void SetImage(string image)
	{
	}

	//------------------------------------------------------------------------------
	Color GetColorForFaction(string factionKey)
	{
		FactionManager fm = GetGame().GetFactionManager();
		if (!fm)
			return null;

		Faction faction = fm.GetFactionByKey(factionKey);
		if (!faction)
			return Color.FromInt(m_UnknownFactionColor.PackToInt());

		return Color.FromInt(faction.GetFactionColor().PackToInt());
	}
	
	//------------------------------------------------------------------------------
	RplId GetSpawnPointId()
	{
		return RplId.Invalid();
	}
	
	//------------------------------------------------------------------------------------------------
	void SetName(string name);
};
