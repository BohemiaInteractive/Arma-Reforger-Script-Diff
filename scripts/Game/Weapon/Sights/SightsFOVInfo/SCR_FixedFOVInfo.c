class SCR_FixedFOVInfo: ScriptedSightsFOVInfo
{
	[Attribute("60.0", UIWidgets.Slider, desc: "Fixed field of view in degrees.", params: "0.1 179.9 0.1")]
	protected float m_fFieldOfView;
	
	/*!
		Returns current field of view provided by this info.
		\return Returns field of view in degrees.
	*/
	protected override float GetCurrentFOV() { return m_fFieldOfView; };
};
