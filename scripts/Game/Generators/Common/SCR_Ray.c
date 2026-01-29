class SCR_Ray
{
	vector m_vPosition;
	vector m_vDirection;	//!< normalised vectorDir

	//------------------------------------------------------------------------------------------------
	//! Get the intermediate ray between two by linear interpolation.
	//! \param[in] valueA source
	//! \param[in] valueB destination
	//! \param[in] t the wanted interval - 0 is valueA, 1 is valueB, 0.5 is right between
	//! \return the lerped ray
	static SCR_Ray Lerp(notnull SCR_Ray valueA, notnull SCR_Ray valueB, float t = 0.5)
	{
		SCR_Ray result = new SCR_Ray();
		result.m_vPosition = vector.Lerp(valueA.m_vPosition, valueB.m_vPosition, t);
		result.m_vDirection = vector.Lerp(valueA.m_vDirection, valueB.m_vDirection, t).Normalized();
		return result;
	}
}
