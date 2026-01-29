class SCR_BoardingWaypointClass: SCR_AIWaypointClass
{
};

class SCR_AIBoardingParameters
{
	bool m_bIsDriverAllowed = true;
	bool m_bIsGunnerAllowed = true;
	bool m_bIsCargoAllowed = true;
}

[BaseContainerProps()]
class SCR_AIBoardingWaypointParameters
{
	[Attribute("", UIWidgets.CheckBox, "Occupy driver")]
	bool m_bIsDriverAllowed;
	[Attribute("", UIWidgets.CheckBox, "Occupy gunner")]
	bool m_bIsGunnerAllowed;
	[Attribute("", UIWidgets.CheckBox, "Occupy cargo")]
	bool m_bIsCargoAllowed;
	
	private ref SCR_AIBoardingParameters m_Parameters;
	
	void SCR_AIBoardingWaypointParameters()
	{
		m_Parameters = new SCR_AIBoardingParameters;
		m_Parameters.m_bIsDriverAllowed = m_bIsDriverAllowed;
		m_Parameters.m_bIsGunnerAllowed = m_bIsGunnerAllowed;
		m_Parameters.m_bIsCargoAllowed = m_bIsCargoAllowed;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_AIBoardingParameters GetParameters()
	{
		return m_Parameters;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetParameters(bool driverAllowed, bool gunnerAllowed, bool cargoAllowed)
	{
		m_Parameters.m_bIsDriverAllowed = driverAllowed;
		m_Parameters.m_bIsGunnerAllowed = gunnerAllowed;
		m_Parameters.m_bIsCargoAllowed = cargoAllowed;
	}	
};

class SCR_BoardingWaypoint : SCR_AIWaypoint
{

	[Attribute("", UIWidgets.Object, "Waypoint parameters")]
	ref SCR_AIBoardingWaypointParameters m_BoardingParameters;
		
	//------------------------------------------------------------------------------------------------
	SCR_AIBoardingParameters GetAllowance()
	{
		if (m_BoardingParameters)
			return m_BoardingParameters.GetParameters();
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetAllowance(bool driverAllowed, bool gunnerAllowed, bool cargoAllowed)
	{
		if (m_BoardingParameters)
			m_BoardingParameters.SetParameters(driverAllowed, gunnerAllowed, cargoAllowed);
	}
};