//! Attached to root of marker dynamic base layout
class SCR_MapMarkerDynamicWComponent : SCR_ScriptedWidgetComponent
{	
	protected int m_iLayerID;			// map layer ID
	protected ImageWidget m_wMarkerIcon;
	protected TextWidget m_wMarkerText;
	protected SCR_MapMarkerEntity m_MarkerEnt;
		
	//------------------------------------------------------------------------------------------------
	//! \param[in] marker
	void SetMarkerEntity(notnull SCR_MapMarkerEntity marker)
	{
		m_MarkerEnt = marker;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetLayerID(int id)
	{
		m_iLayerID = id;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Supports custom aspect ratio in case of non standard size imagesets
	//! \param[in] icon
	//! \param[in] quad
	//! \param[in] aspectRatio
	void SetImage(ResourceName icon, string quad, float aspectRatio = 1, int sizeFlag = 64)
	{
		m_wMarkerIcon.LoadImageFromSet(0, icon, quad);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] text
	void SetText(string text)
	{
		m_wMarkerText.SetText(text);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] state
	void SetTextVisible(bool state)
	{
		m_wMarkerText.SetVisible(state);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] color
	void SetColor(Color color)
	{
		m_wMarkerIcon.SetColor(color);
	}
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		
		m_wMarkerIcon = ImageWidget.Cast(m_wRoot.FindAnyWidget("MarkerIcon"));
		m_wMarkerText = TextWidget.Cast(m_wRoot.FindAnyWidget("MarkerText"));
	}
}
