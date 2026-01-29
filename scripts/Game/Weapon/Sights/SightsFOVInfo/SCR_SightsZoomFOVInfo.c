/*
Scripted fov info class basing fovs on given zoom info.
Variable zoom.
*/

class SCR_SightsZoomFOVInfo : SCR_BaseVariableSightsFOVInfo
{	
	[Attribute("1", UIWidgets.Auto, desc: "Initial zoom that can be static or scaled up", params: "0.1 100 0.1")]
	protected float m_fBaseZoom;
	
	[Attribute("0", UIWidgets.Auto, desc: "Max zoom, smaller zoomMax than baseZoom will result in static zoom", params: "0.1 100 0.1")]
	protected float m_fZoomMax;
	
	[Attribute("0.5", UIWidgets.Slider, desc: "Zoom step size, will automatically set step count", params: "0.1 5 0.1")]
	protected float m_fStepZoomSize;
	
	[Attribute("6.0", UIWidgets.Slider, desc: "Interpolation speed.", params: "0 100 0.1")]
	protected float m_fInterpolationSpeed;

	protected ref array<float> m_aFOVs = new array<float>;
	protected int m_iStepsCount = 1;

	//! Immediate field of view value.
	protected float m_fCurrentFOV;
	
	//! Currently selected FOV or 0 (as base fov) if none.
	protected int m_iCurrentIndex = 0;
	
	protected ref ScriptInvoker<float, float> event_OnZoomChanged;

	//------------------------------------------------------------------------------------------------
	//! Returns the number of available elements.
	override int GetCount() { return m_aFOVs.Count(); }

	//------------------------------------------------------------------------------------------------
	//! Returns currently selected element or -1 if none.
	override int GetCurrentIndex() { return m_iCurrentIndex; }

	//------------------------------------------------------------------------------------------------
	/*!
		Set provided element as the current one.
		\param index Index of FOV element.
	*/
	override void SetIndex(int index) 
	{
		m_iCurrentIndex = index;
		InvokeOnZoomChanged(GetCurrentZoom(), m_aFOVs[m_iCurrentIndex]);
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void OnInit(IEntity owner, BaseSightsComponent sights)
	{
		// Initialize to default (first available) value
		if (GetCount() > 0)
		{
			SetIndex(0);
			m_fCurrentFOV = m_aFOVs[0];
		}
		else
		{
			m_fCurrentFOV = SCR_2DOpticsComponent.CalculateZoomFOV(m_fBaseZoom);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
		Called every frame when active to update current state.
		\param owner Parent weapon entity.
		\param sights Parent sights component.
		\param timeSlice Delta of time since last update.
	*/
	protected override void OnUpdate(IEntity owner, BaseSightsComponent sights, float timeSlice)
	{
		if (!m_aFOVs.IsIndexValid(m_iCurrentIndex))
			return;

		float target = m_aFOVs[m_iCurrentIndex];
		
		float t = timeSlice * m_fInterpolationSpeed;
		if (t > 1.0) 
			t = 1.0;
		if (t < 0.0)
			t = 0.0;
		
		m_fCurrentFOV = Math.Lerp(m_fCurrentFOV, target, t);
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Returns current field of view provided by this info.
		\return Returns field of view in degrees.
	*/
	protected override float GetCurrentFOV()
	{
		return m_fCurrentFOV;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get array of each zoom based on min-max and steps in between
	array<float> ZoomsToArray()
	{
		array<float> zooms = new array<float>;
		
		// Single zoom 
		if (m_fBaseZoom == m_fZoomMax || m_fBaseZoom > m_fZoomMax)
		{
			zooms.Insert(m_fBaseZoom);
			return zooms;
		}
		
		// Count steps 
		int count = (m_fZoomMax - m_fBaseZoom) / m_fStepZoomSize;
		
		// Create variable zooms
		for (int i = 0; i <= count; i++)
		{
			float zoom = GetZoom(i, count);
			zooms.Insert(zoom);
		}
		
		m_iStepsCount = zooms.Count(); 
		return zooms;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Add new fov based on reticle range and zoom 
	void InsertFov(float fov)
	{
		m_aFOVs.Insert(fov);
	}

	//------------------------------------------------------------------------------------------------
	int GetStepsCount()
	{
		return m_iStepsCount;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetCurrentFov(float fov)
	{ 
		m_fCurrentFOV = fov;
	}
	
	//------------------------------------------------------------------------------------------------
	protected float GetZoom(int i, int count)
	{
		if (i < 0)
			i = 0;
		
		return m_fBaseZoom + i * m_fStepZoomSize;
	}
	
	//------------------------------------------------------------------------------------------------
	float GetCurrentZoom()
	{
		// TODO just check this with Jakub Werner as whether its good or nicht,
		// ich have to say that das ist nicht gut, das ist nicht gut Hans!
		
		int count = m_aFOVs.Count();

		if (count == 0)
			return m_fBaseZoom;
		
		return GetZoom(m_iCurrentIndex, count);
	}
	
	//------------------------------------------------------------------------------------------------
	float GetBaseZoom()
	{
		int count = m_aFOVs.Count();
		if (count > 0)
			return GetZoom(0, count);
		
		return 1.0;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
		Returns max field of view provided by this info.
		Assumes that first FOV is the largest one.
		Returns zero if FOVs array is empty.
		\return Returns field of view in degrees.
	*/
	override float GetBaseFOV()
	{
		if (!m_aFOVs.IsEmpty())
			return m_aFOVs[0];

		return m_fCurrentFOV;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns true when current FOV of the optic does not match the target FOV.
	override bool IsAdjusting()
	{
		if (!m_aFOVs.IsIndexValid(m_iCurrentIndex))
			return false;

		return !float.AlmostEqual(m_fCurrentFOV, m_aFOVs[m_iCurrentIndex]);
	}

	void ForceUpdate(IEntity owner, BaseSightsComponent sights, float timeSlice)
	{
		OnUpdate(owner, sights, timeSlice);
	}
	
	//------------------------------------------------------------------------------------------------
	// Invoker API
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------\
	ScriptInvoker GetEventOnZoomChanged()
	{
		if (!event_OnZoomChanged)
			event_OnZoomChanged = new ScriptInvoker();
		
		return event_OnZoomChanged;
	}
	
	//------------------------------------------------------------------------------------------------\
	protected void InvokeOnZoomChanged(float zoom, float fov)
	{
		if (event_OnZoomChanged)
			event_OnZoomChanged.Invoke(zoom, fov);
	}
};
