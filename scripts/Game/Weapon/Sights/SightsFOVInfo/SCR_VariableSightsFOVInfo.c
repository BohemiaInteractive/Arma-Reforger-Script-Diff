class SCR_VariableSightsFOVInfo: SCR_BaseVariableSightsFOVInfo
{
	[Attribute("", UIWidgets.Auto, desc: "Individual FOV settings.")]
	protected ref array<float> m_aFOVs;

	[Attribute("6.0", UIWidgets.Slider, desc: "Interpolation speed.", params: "0 100 0.1")]
	protected float m_fInterpolationSpeed;

	//! Immediate field of view value.
	protected float m_fCurrentFOV = 60.0;

	//! Currently selected FOV or -1 if none.
	protected int m_iCurrentIndex = -1;

	//------------------------------------------------------------------------------------------------
	/*!
		Returns the number of available elements.
	*/
	override int GetCount()
	{
		return m_aFOVs.Count();
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Returns currently selected element or -1 if none.
	*/
	override int GetCurrentIndex()
	{
		return m_iCurrentIndex;
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Set provided element as the current one.
		\param index Index of FOV element.
	*/
	protected override void SetIndex(int index)
	{
		m_iCurrentIndex = index;
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Called on initialization.
		\param owner Parent weapon entity.
		\param sights Parent sights component.
	*/
	protected override void OnInit(IEntity owner, BaseSightsComponent sights)
	{
		// Initialize to default (first available) value
		if (GetCount() > 0)
		{
			SetIndex(0);
			m_fCurrentFOV = m_aFOVs[0];
		}
		else
			m_fCurrentFOV = 60.0;
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
		m_fCurrentFOV = Math.Lerp(m_fCurrentFOV, target, timeSlice * m_fInterpolationSpeed);
	}

	//------------------------------------------------------------------------------------------------
	//! Returns true when current FOV of the optic does not match the target FOV.
	override bool IsAdjusting()
	{
		if (!m_aFOVs.IsIndexValid(m_iCurrentIndex))
			return false;

		return !float.AlmostEqual(m_fCurrentFOV, m_aFOVs[m_iCurrentIndex]);
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
};
