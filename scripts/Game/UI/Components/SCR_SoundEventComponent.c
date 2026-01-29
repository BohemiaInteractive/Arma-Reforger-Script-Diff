class SCR_SoundEventComponent : ScriptedWidgetComponent
{
	[Attribute()]
	string m_sSoundMouseEnter;

	[Attribute()]
	string m_sSoundMouseLeave;

	[Attribute()]
	string m_sSoundFocus;

	[Attribute()]
	string m_sSoundFocusLost;

	[Attribute()]
	string m_sSoundClick;

	[Attribute()]
	string m_sSoundMouseButtonDown;

	[Attribute()]
	string m_sSoundMouseButtonUp;

	//------------------------------------------------------------------------------------------------
	override bool OnMouseEnter(Widget w, int x, int y)
	{
		if (m_sSoundMouseEnter != string.Empty)
			SCR_UISoundEntity.SoundEvent(m_sSoundMouseEnter);
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnMouseLeave(Widget w, Widget enterW, int x, int y)
	{
		if (m_sSoundMouseLeave != string.Empty)
			SCR_UISoundEntity.SoundEvent(m_sSoundMouseLeave);
		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnFocus(Widget w, int x, int y)
	{
		if (m_sSoundFocus != string.Empty)
			SCR_UISoundEntity.SoundEvent(m_sSoundFocus);
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnFocusLost(Widget w, int x, int y)
	{
		if (m_sSoundFocusLost != string.Empty)
			SCR_UISoundEntity.SoundEvent(m_sSoundFocusLost);
		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnClick(Widget w, int x, int y, int button)
	{
		if (m_sSoundClick != string.Empty)
			SCR_UISoundEntity.SoundEvent(m_sSoundClick);
		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnMouseButtonDown(Widget w, int x, int y, int button)
	{
		if (m_sSoundMouseButtonDown != string.Empty)
			SCR_UISoundEntity.SoundEvent(m_sSoundMouseButtonDown);
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnMouseButtonUp(Widget w, int x, int y, int button)
	{
		if (m_sSoundMouseButtonUp != string.Empty)
			SCR_UISoundEntity.SoundEvent(m_sSoundMouseButtonUp);
		return false;
	}
};