class SCR_RadialProgressBarUIComponent: ScriptedWidgetComponent
{	
	[Attribute(defvalue: "0", desc: "Progress of bar, value between min value and maxvalue")]
	protected float m_iProgress;
	
	[Attribute(defvalue: "0", desc: "Progress of bar, lowest value")]
	protected float m_fMinValue;
	
	[Attribute(defvalue: "1", desc: "Progress of bar, highest value")]
	protected float m_fMaxValue;
	
	[Attribute(defvalue: "0", desc: "Mirrors the Progress bar, by default true")]
	protected bool m_bIsMirrored;
	
	[Attribute("ProgressBar")]
	protected string m_sRadialProgressBarName;
	
	[Attribute("PreviewBar")]
	protected string m_sRadialProgressPreviewName;
	
	[Attribute("Background")]
	protected string m_sBackgroundName;
	
	protected ImageWidget m_wRadialProgressBar;
	protected ImageWidget m_wRadialProgressPreview;
	protected ImageWidget m_wBackground;
	
	protected ref ScriptInvoker Event_OnProgressFull = new ScriptInvoker;
	protected ref ScriptInvoker Event_OnProgressEmpty = new ScriptInvoker;
	
	protected bool m_bProgressFull;
	
	/*!
	Set the progress. Takes a value between min and max value
	\param progress value between min and max value
	*/
	void SetProgress(float progress)
	{ 		
		m_iProgress = Math.Clamp(progress, m_fMinValue, m_fMaxValue);

		float maskProgress = (progress - m_fMinValue) / (m_fMaxValue - m_fMinValue);
		
		if (m_bIsMirrored)
			m_wRadialProgressBar.SetMaskProgress(1 - maskProgress);
		else 
			m_wRadialProgressBar.SetMaskProgress(maskProgress);
		
		if (progress == m_fMaxValue && !m_bProgressFull)
		{
			m_bProgressFull = true;
			Event_OnProgressFull.Invoke(this);
		}
		else if (progress == m_fMinValue && m_bProgressFull)
		{
			m_bProgressFull = false;
			Event_OnProgressEmpty.Invoke(this);
		}
	}
	
	/*!
	Set the progress of the preview bar. Takes a value between 0 and 1
	\param progress value between 0 and 1
	*/
	void SetPreviewProgress(float progress)
	{
		progress = Math.Clamp(progress, m_fMinValue, m_fMaxValue);
		
		if (m_bIsMirrored)
			m_wRadialProgressPreview.SetMaskProgress(m_fMaxValue - progress);
		else 
			m_wRadialProgressPreview.SetMaskProgress(progress);
		
		m_wRadialProgressPreview.SetVisible(progress > m_fMinValue);
	}
	
	/*!
	Get the progress value
	\return float progress value between min and max value
	*/
	float GetProgress()
	{
		return m_iProgress;
	}
	
	/*!
	Get radial progress bar image widget.
	\return ImageWidget radial progress bar image widget
	*/
	ImageWidget GetRadialProgressBarWidget()
	{
		return m_wRadialProgressBar;
	}
	
	/*!
	Get radial progress bar background image widget.
	\return ImageWidget radial progress bar background image widget
	*/
	ImageWidget GetBackgroundWidget()
	{
		return m_wBackground;
	}
	
	/*!
	Set radial progress bar image.
	\param RadialProgressBarImage image to set
	*/
	void SetProgressBarImage(ResourceName RadialProgressBarImage)
	{
		if (m_wRadialProgressBar)
			m_wRadialProgressBar.LoadImageTexture(0, RadialProgressBarImage);
	}
	
	/*!
	Set radial progress bar background image.
	\param BackgroundImage image to set
	*/
	void SetBackgroundImage(ResourceName BackgroundImage)
	{
		if (m_wBackground)
			m_wBackground.LoadImageTexture(0, BackgroundImage);
	}
	
	ScriptInvoker GetOnProgressFull()
	{
		return Event_OnProgressFull;
	}
	
	ScriptInvoker GetOnProgressEmpty()
	{
		return Event_OnProgressEmpty;
	}
	

	override void HandlerAttached(Widget w)
	{	
		if (SCR_Global.IsEditMode()) 
			return;
		
		m_wRadialProgressBar = ImageWidget.Cast(w.FindAnyWidget(m_sRadialProgressBarName));
		m_wRadialProgressPreview = ImageWidget.Cast(w.FindAnyWidget(m_sRadialProgressPreviewName));
		m_wBackground = ImageWidget.Cast(w.FindAnyWidget(m_sBackgroundName));
		
		if (!m_wRadialProgressBar)
		{
			Print("SCR_RadialProgressBarUIComponent could not find m_wRadialProgressBar!", LogLevel.ERROR);
			return;
		}
		
		if (m_bIsMirrored)
		{
			m_wRadialProgressBar.SetMaskMode(ImageMaskMode.INVERTED);
			m_wRadialProgressPreview.SetMaskMode(ImageMaskMode.INVERTED);
		}
		else
		{
			m_wRadialProgressBar.SetMaskMode(ImageMaskMode.REGULAR);
			m_wRadialProgressPreview.SetMaskMode(ImageMaskMode.REGULAR);
		}
		
		m_bProgressFull = false;
		SetProgress(m_iProgress);
		SetPreviewProgress(0);
	}
	
};
