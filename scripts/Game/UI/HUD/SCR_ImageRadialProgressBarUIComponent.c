class SCR_ImageRadialProgressBarUIComponent: SCR_RadialProgressBarUIComponent
{		
	[Attribute("FadeImage")]
	protected string m_sFadeImageName;
	
	[Attribute(defvalue: "1", desc: "Fade in speed of image")]
	protected float m_bFadeImageInSpeed;	
	
	[Attribute(defvalue: "1", desc: "Fade out speed of image")]
	protected float m_bFadeImageOutSpeed;
	
	[Attribute(defvalue: "0", desc: "Sets the start opacity of the image, it starts with opacity 0 if false else it starts with 1")]
	protected bool m_bIsFadeImageVisible;
	
	//Ref
	protected ImageWidget m_wFadeImage;
	
	//State
	protected bool m_bisFading;
	protected bool m_bIsFadingIn;
	
	//Script invoker
	protected ref ScriptInvoker Event_OnFadeDone = new ScriptInvoker; //SCR_ImageRadialProgressBarUIComponent, bool wasFadingIn
	
	override void SetProgress(float progress)
	{
		super.SetProgress(progress);
		
		if (m_wFadeImage)
		{
			if (m_iProgress >= m_fMaxValue && !m_bIsFadeImageVisible)
			{
				FadeImage(true);
			}
			else if (m_iProgress < m_fMaxValue && m_bIsFadeImageVisible)
			{
				FadeImage(false);
			}
		}
	}
	
	void InstantProgressMaxMin(bool SetToMax)
	{
		if (SetToMax)
		{
			SetProgress(m_fMaxValue);
			SetFadeImageVisible(true);
		}
		else 
		{
			SetProgress(m_fMinValue);
			SetFadeImageVisible(false);
		}
	}
	
	float GetMinProgress()
	{
		return m_fMinValue;
	}
	
	float GetMaxProgress()
	{
		return m_fMaxValue;
	}
	
	ScriptInvoker GetOnFadeDone()
	{
		return Event_OnFadeDone;
	}
	
	void FadeImage(bool fadeIn)
	{
		m_bIsFadeImageVisible = fadeIn;
		m_bIsFadingIn = fadeIn;
		
		if (!m_bisFading)
		{
			m_bisFading = true;
			GetGame().GetCallqueue().CallLater(FadeUpdate, 500, true);
		}
		
		AnimateWidget.Opacity(m_wFadeImage, fadeIn, m_bFadeImageInSpeed);
	}
	
	protected void FadeUpdate()
	{
		if (m_bisFading)
		{
			if ((m_bIsFadingIn && m_wFadeImage.GetOpacity() >= 1) || (!m_bIsFadingIn && m_wFadeImage.GetOpacity() <= 0))
			{
				m_bisFading = false;
				GetGame().GetCallqueue().Remove(FadeUpdate);
				Event_OnFadeDone.Invoke(this, m_bIsFadingIn);
			}
		}
	}
	
	/*!
	Get Fade Image
	\return m_wFadeImage widget to be faded
	*/
	ImageWidget GetFadeImage()
	{
		return m_wFadeImage;
	}
	
	void SetFadeImageVisible(bool setVisible)
	{
		if (m_bisFading)
		{
			m_bisFading = false;
			GetGame().GetCallqueue().Remove(FadeUpdate);
		}
		
		AnimateWidget.StopAnimation(m_wFadeImage, WidgetAnimationOpacity);
		
		m_bIsFadeImageVisible = setVisible;
		
		if (setVisible)
			m_wFadeImage.SetOpacity(1);
		else
			m_wFadeImage.SetOpacity(0);
	}
	
	override void HandlerAttached(Widget w)
	{	
		if (SCR_Global.IsEditMode()) 
			return;
		
		super.HandlerAttached(w);
		
		m_wFadeImage = ImageWidget.Cast(w.FindAnyWidget(m_sFadeImageName));
		
		SetFadeImageVisible(m_bIsFadeImageVisible);
	}
	
	override void HandlerDeattached(Widget w)
	{
		if (m_bisFading)
			GetGame().GetCallqueue().Remove(FadeUpdate); //>>>>> ADD CHECK FADE DONE TO HIDE UI IN TOGGLE EDITOR (JUST ALWAYS INVOKE AND CHECK IF IMAGE IS VISIBLE)
	}
	
};