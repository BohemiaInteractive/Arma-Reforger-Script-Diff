// Component for simple loading circle widget with endless spining  

//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
// Non interactable component that inherits from the base of interactable components, and thus includes needless settings for clicking and sounds

//------------------------------------------------------------------------------------------------
class SCR_LoadingCircleComponent : SCR_WLibComponentBase 
{
	const string WIDGET_IMAGE = "Image";
	
	[Attribute("1", UIWidgets.CheckBox, "Speed of rotation")]
	protected float m_fSpeed;
	
	protected bool m_bIsPlayingAnimation;
	protected bool m_bIsAnimationDone;
	
	SizeLayoutWidget m_wRootSize;
	ImageWidget m_wImage;
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		
		m_wRoot = w;
		
		m_wRootSize = SizeLayoutWidget.Cast(m_wRoot);
		m_wImage = ImageWidget.Cast(m_wRoot.FindAnyWidget(WIDGET_IMAGE));

		m_bIsPlayingAnimation = true;
		m_bIsAnimationDone = true;
		
		Animate();
	}
	
	//------------------------------------------------------------------------------------------------	
	protected void Animate()
	{
		if (!m_wImage)
			return;
		
		m_wImage.SetRotation(0);
		WidgetAnimationBase anim = AnimateWidget.Rotation(m_wImage, 360, m_fSpeed);
		if (anim)
			anim.GetOnCompleted().Insert(Animate);
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_LoadingCircleComponent()
	{
			
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_LoadingCircleComponent()
	{
			
	}
	
	// API
	//------------------------------------------------------------------------------------------------
	void SetSpeed(float speed) { m_fSpeed = speed; }
	
	//------------------------------------------------------------------------------------------------
	float GetSpeed() { return m_fSpeed; }
	
	//------------------------------------------------------------------------------------------------
	void GetIsPlayingAnimation(bool playing) { m_bIsPlayingAnimation = playing; }
	
	//------------------------------------------------------------------------------------------------
	bool GetIsPlayingAnimation() { return m_bIsPlayingAnimation; }
};

//---- REFACTOR NOTE END ----