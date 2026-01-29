class SCR_LoadingSpinner : ScriptedWidgetComponent
{
	[Attribute("0.5", desc: "Rotations per seconds")]
	protected float m_fSpeed;

	protected Widget m_wRoot;
	protected ImageWidget m_wMap;
	protected ImageWidget m_wRadar;
	protected ImageWidget m_wTargets;

	//---------------------------------------------------------------------------------------------
	override protected void HandlerAttached(Widget w)
	{
		m_wRoot = w;
		
		m_wMap = ImageWidget.Cast(w.FindAnyWidget("Map"));
		m_wRadar = ImageWidget.Cast(w.FindAnyWidget("Radar"));
		m_wTargets = ImageWidget.Cast(w.FindAnyWidget("Targets"));
	}

	//---------------------------------------------------------------------------------------------
	void Update(float deltaTime)
	{
		ProgressMask(m_wTargets, deltaTime, m_fSpeed);
		RotateImage(m_wRadar, deltaTime, m_fSpeed);
	}
	
	//---------------------------------------------------------------------------------------------
	void ProgressMask(ImageWidget w, float timeSlice, float speed)
	{
		float maskProgress = w.GetMaskProgress() + speed * timeSlice;
		if (maskProgress > 1)
			maskProgress -= 1;
		
		w.SetMaskProgress(maskProgress);
	}
	
	//---------------------------------------------------------------------------------------------
	void RotateImage(ImageWidget w, float timeSlice, float speed)
	{
		float rotation = w.GetRotation() + (speed * timeSlice * 360);
		if (rotation > 360)
			rotation -= 360;
		
		w.SetRotation(rotation);
	}	
};