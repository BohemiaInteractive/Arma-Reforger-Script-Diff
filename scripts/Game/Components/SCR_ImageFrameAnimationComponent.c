//! This component implements animation by switching textures, like in a movie.
//! Animation frame rate can be separate from game frame rate.
//! Images must be in image set.
//!
//! You do not have to care about starting/stopping the animation when widget is shown or hidden,
//! it will be performed automatically.
class SCR_ImageFrameAnimationComponent : ScriptedWidgetComponent
{
	// Attributes
	
	[Attribute("30", UIWidgets.EditBox, "Animation frames per second")]
	protected float m_fFramesPerSecond;
	
	[Attribute("1", UIWidgets.EditBox, "Number of frames")]
	protected int m_iNumFrames;
	
	[Attribute("0", UIWidgets.EditBox, "Start frame of an animation")]
	protected int m_iStartFrameId;
	
	[Attribute("Frame_%1", UIWidgets.EditBox, "Format for image name withim imageset")]
	protected string m_sImageNameFormat;
	
	[Attribute("-1", UIWidgets.EditBox, "Length of integer conversion of frame Id to string. See int.ToString().")]
	protected int m_iIntToStringLen;
	
	[Attribute("", UIWidgets.ResourceNamePicker, "Image Set with frames", params: "imageset")]
	protected ResourceName m_sImageSet;
	
	// Internal
	
	protected float m_fCounterMs;
	protected float m_fMsPerFrame;	// Milliseconds per frame
	protected int m_iFrameId;
	
	protected ImageWidget m_wRoot;
	
	//------------------------------------------------------------------------------------------------
	//! PUBLIC API
	
	// You can Start/Stop animation, but most likely it's not needed,
	// Because it will terminate itself when widget is hidden,
	// And it will get reactivated when widget is shown again.
	
	//------------------------------------------------------------------------------------------------
	//!
	void Start()
	{
		if (!m_wRoot)
			return;
		
		m_iFrameId = m_iStartFrameId;
		ShowFrame(m_iStartFrameId);
		
		ScriptCallQueue queue = GetGame().GetCallqueue();
		queue.Remove(OnEachFrame);
		queue.CallLater(OnEachFrame, 0, true);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	void Stop()
	{		
		GetGame().GetCallqueue().Remove(OnEachFrame);
	}

	//------------------------------------------------------------------------------------------------
	override protected void HandlerAttached(Widget w)
	{
		m_wRoot = ImageWidget.Cast(w);
		
		if (!m_wRoot)
		{
			Print("SCR_ImageFrameAnimationComponent must be attached to image widget!", LogLevel.ERROR);
			return;
		}
		
		// Ensure safe value for input data
		if (m_fFramesPerSecond <= 0.0)
		{
			Print("SCR_ImageFrameAnimationComponent: wrong m_fFramesPerSecond value", LogLevel.ERROR);
			m_fFramesPerSecond = 1;
		}
		if (m_iNumFrames <= 0)
		{
			Print("SCR_ImageFrameAnimationComponent: wrong m_iNumFrames value", LogLevel.ERROR);
			m_iNumFrames = 1;
		}
		if (m_iStartFrameId < 0)
		{
			Print("SCR_ImageFrameAnimationComponent: wrong m_iStartFrameId value", LogLevel.ERROR);
			m_iStartFrameId = 0;
		}
			
		// Calculate run time constants
		m_fMsPerFrame = Math.Round(1000.0 / m_fFramesPerSecond);
		m_fMsPerFrame = Math.Max(1.0, m_fMsPerFrame);
		
		if (w.IsVisible())
			Start();
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void HandlerDeattached(Widget w)
	{
		ArmaReforgerScripted game = GetGame();
		if (game && game.GetCallqueue())
			Stop();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnEachFrame()
	{
		if (!m_wRoot.IsVisibleInHierarchy())
		{
			Stop();
			return;
		}
			
		// Increase counter until we have accumulated m_fMsPerFrame time
		// Then skip some frames, at least one
		
		m_fCounterMs += Math.Min(ftime, 200.0); // Limit ftime within reasonable value
		
		float nFramesToSkip = Math.Ceil(m_fCounterMs / m_fMsPerFrame);
		
		if (nFramesToSkip >= 1.0)
		{
			m_fCounterMs -= nFramesToSkip * m_fMsPerFrame;
			m_iFrameId += nFramesToSkip;
			m_iFrameId = m_iFrameId % m_iNumFrames;
			
			ShowFrame(m_iFrameId);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ShowFrame(int id)
	{
		string imageName = string.Format(m_sImageNameFormat, id.ToString(m_iIntToStringLen));
		m_wRoot.LoadImageFromSet(0, m_sImageSet, imageName);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnUpdate(Widget w)
	{
		if (w != m_wRoot || !m_wRoot)
			return true;
		
		bool nowVisible = m_wRoot.IsVisibleInHierarchy();
		
		if (nowVisible)
			Start();
		else
			Stop();
		
		return true;
	}
}
