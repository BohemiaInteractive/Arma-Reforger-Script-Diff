class SCR_InventoryProgressBar : ScriptedWidgetComponent
{
	[Attribute("0 0 0 1")]
	protected ref Color m_ColorLow;
	
	[Attribute("0.5 0.5 0.5 1")]
	protected ref Color m_ColorMiddle;
	
	[Attribute("1 1 1 1")]
	protected ref Color m_ColorHigh;
	
	[Attribute("0 1 0 1")]
	protected ref Color m_ColorPreviewPositive;
	
	[Attribute("1 0 0 1")]
	protected ref Color m_ColorPreviewNegative;
	
	[Attribute("1")]
	protected bool m_bFlipColors;
	
	[Attribute("ProgressBar")]
	protected string m_sProgressBarName;

	[Attribute("ProgressBarTemp")]
	protected string m_sProgressBarNamePreview;	
		
	protected ProgressBarWidget m_wProgressBar;
	protected ProgressBarWidget m_wProgressBarPreview;
	protected bool m_bColorsFlipped;	
	
	protected const float LOW_TRESHOLD = 0.3;
	protected const float MID_DIFF = 0.05;
	protected const float HIGH_TRESHOLD = 0.8;	
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		m_wProgressBar = ProgressBarWidget.Cast(w.FindAnyWidget(m_sProgressBarName));
		if (!m_wProgressBar)
		{
			m_wProgressBar = ProgressBarWidget.Cast(w);
			if (!m_wProgressBar)
			{
				Print("Cannot find ProgressBarWidget in the hierarchy.", LogLevel.ERROR);
				return;
			}
		}
		
		m_wProgressBarPreview = ProgressBarWidget.Cast(w.FindAnyWidget(m_sProgressBarNamePreview));
		if (!m_wProgressBarPreview)
		{
			m_wProgressBarPreview = ProgressBarWidget.Cast(w);
			if (!m_wProgressBarPreview)
			{
				Print("Cannot find ProgressBarWidget in the hierarchy.", LogLevel.ERROR);
				return;
			}
		}

		if (m_bFlipColors)
		{
			Color temp = m_ColorHigh;
			m_ColorHigh = m_ColorLow;
			m_ColorLow = temp;
		}
		
		m_wProgressBarPreview.SetColor(m_ColorPreviewPositive);

		UpdateProgressBar();
	}
	
	void SetProgressRange(float min, float max)
	{
		if (!m_wProgressBar || !m_wProgressBarPreview || max <= 0)
			return;

		m_wProgressBar.SetMin(min);
		m_wProgressBar.SetMax(max);

		m_wProgressBarPreview.SetMin(min);
		m_wProgressBarPreview.SetMax(max);

		UpdateProgressBar();	
	}
	
	void SetCurrentProgress(float progress)
	{
		if (!m_wProgressBar)
			return;

		m_wProgressBar.SetCurrent(progress);
		UpdateProgressBar();
	}
	
	void SetCurrentProgressPreview(float progress)
	{
		if (!m_wProgressBarPreview)
			return;
		
		m_wProgressBarPreview.SetCurrent(progress);
	}
	
	void SetPreviewColor(bool enabled)
	{
		if (enabled)
			m_wProgressBarPreview.SetColor(m_ColorPreviewPositive);
		else
			m_wProgressBarPreview.SetColor(m_ColorPreviewNegative);
	}
	
	void FlipColors()
	{
		if (m_bColorsFlipped)
			return;

		Color temp = m_ColorHigh;
		m_ColorHigh = m_ColorLow;
		m_ColorLow = temp;
		m_bColorsFlipped = true;

		UpdateProgressBar();
	}
	
	protected void UpdateProgressBar()
	{
		float progress01 = Math.InverseLerp(m_wProgressBar.GetMin(), m_wProgressBar.GetMax(), m_wProgressBar.GetCurrent());
		if (progress01 < 0.5)
		{
			m_wProgressBar.SetColor(m_ColorLow.LerpNew(m_ColorMiddle, Math.InverseLerp(LOW_TRESHOLD, 0.5 - MID_DIFF, Math.Clamp(progress01, LOW_TRESHOLD, 0.5 - MID_DIFF))));
		}
		else
		{
			m_wProgressBar.SetColor(m_ColorMiddle.LerpNew(m_ColorHigh, Math.InverseLerp(0.5 + MID_DIFF, HIGH_TRESHOLD, Math.Clamp(progress01, 0.5 + MID_DIFF, HIGH_TRESHOLD))));
		}
	}
}