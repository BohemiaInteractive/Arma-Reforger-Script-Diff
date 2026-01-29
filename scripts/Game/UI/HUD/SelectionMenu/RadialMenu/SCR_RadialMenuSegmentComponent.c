/*!
Radial menu segment handling visualization
*/
class SCR_RadialMenuSegmentComponent : SCR_ScriptedWidgetComponent
{
	protected const string BACKGROUND = "Background";
	protected const string CATEGORY_WRAP = "CategoryWrap";
	protected const string CATEGORY = "Category";
	protected const string CATEGORY_ARROW = "CategoryArrow";
	
	protected ImageWidget m_wBackground;
	protected Widget m_wCategoryWrap;
	protected ImageWidget m_wCategory;
	protected ImageWidget m_wCategoryArrow;
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		
		m_wBackground = ImageWidget.Cast(w.FindAnyWidget(BACKGROUND));
		m_wCategoryWrap = w.FindAnyWidget(CATEGORY_WRAP);
		m_wCategory = ImageWidget.Cast(w.FindAnyWidget(CATEGORY));
		m_wCategoryArrow = ImageWidget.Cast(w.FindAnyWidget(CATEGORY_ARROW));
	}
	
	const float CATEGORY_OFFSET = 4;
	
	//------------------------------------------------------------------------------------------------
	void SetupSegment(float range, float angle, bool isCategory)
	{
		float angleDist = range * 360;
		float rot = angle - angleDist * 0.5;
		
		m_wBackground.SetMaskProgress(range);
		m_wBackground.SetRotation(rot);
		
		// Category
		m_wCategoryWrap.SetVisible(isCategory);
		
		if (isCategory)
		{
			m_wCategory.SetMaskProgress(range - CATEGORY_OFFSET / 360);
			m_wCategory.SetRotation(rot + CATEGORY_OFFSET * 0.5);
			
			m_wCategoryArrow.SetRotation(angle);
			
			float arrowRot = angle * Math.DEG2RAD;
			float radius = FrameSlot.GetPosY(m_wCategoryArrow);
			
			vector pos = Vector(
				radius * Math.Cos(arrowRot - 0.5 * Math.PI),
				radius * Math.Sin(arrowRot - 0.5 * Math.PI),
				0.0);
			
			FrameSlot.SetPos(m_wCategoryArrow, pos[0], pos[1]);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	ImageWidget GetBackgroundWidget()
	{
		return m_wBackground;
	}
	
	//------------------------------------------------------------------------------------------------
	Widget GetCategoryWrapWidget()
	{
		return m_wCategoryWrap;
	}
}