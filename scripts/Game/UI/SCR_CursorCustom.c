//------------------------------------------------------------------------------------------------
//! Scripted map cursor visualization for cases where the gamecode provided one is not sufficient
class SCR_CursorCustom
{
	const int REAL_CURSOR_HIDE = 12;	// "empty" visual ID of real cursor used to "hide" it
	const string layoutPath = "{80F7ED5410342195}UI/layouts/Map/CursorCustom.layout";
	
	protected bool m_bIsVisible;
	protected Widget m_wRoot;
	protected Widget m_wCursor;
	protected ImageWidget m_wCursorImage; 
	
	//------------------------------------------------------------------------------------------------
	//! Set visual
	//! \param stateCfg is cursor state config, null to disable
	void SetCursorVisual(SCR_CursorVisualState stateCfg)
	{				
		if (!stateCfg)
		{
			WidgetManager.SetCursor(0);
			m_bIsVisible = false;
			m_wRoot.SetVisible(false);
			return;
		}
		else if (!m_bIsVisible)
		{
			WidgetManager.SetCursor(REAL_CURSOR_HIDE);
			m_bIsVisible = true;
			m_wRoot.SetVisible(true);
		}
								
		if (SCR_MapCursorInfo.isGamepad && !stateCfg.m_sImageQuadController.IsEmpty())
			m_wCursorImage.LoadImageFromSet(0, stateCfg.m_sCursorIconsImageset, stateCfg.m_sImageQuadController);
		else
			m_wCursorImage.LoadImageFromSet(0, stateCfg.m_sCursorIconsImageset, stateCfg.m_sImageQuad);
		
		AlignableSlot.SetPadding(m_wCursorImage, stateCfg.m_fPaddingLeft, stateCfg.m_fPaddingTop, 0, 0);
				
		m_wCursorImage.SetColor(stateCfg.m_Color);
	} 
		
	//------------------------------------------------------------------------------------------------
	//! Set opacity
	void SetOpacity(float opacity)
	{
		m_wCursorImage.SetOpacity(opacity);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Update position, has to be called manually
	void Update(float x, float y)
	{
		FrameSlot.SetPos(m_wCursor, x, y);
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_CursorCustom()
	{
		m_wRoot = GetGame().GetWorkspace().CreateWidgets(layoutPath);
		m_wCursor = m_wRoot.FindAnyWidget("CursorWidget");
		m_wCursorImage = ImageWidget.Cast(m_wRoot.FindAnyWidget("CursorImage"));
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_CursorCustom()
	{
		m_wCursorImage.RemoveFromHierarchy();
		m_wRoot.RemoveFromHierarchy();
	}

};
