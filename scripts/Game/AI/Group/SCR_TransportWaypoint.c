class SCR_TransportWaypointClass: SCR_AIWaypointClass
{
};

class SCR_TransportWaypoint : SCR_AIWaypoint
{
	[Attribute("", UIWidgets.EditBox, "How long AI should wait for vehicle to pick them up before moving out on foot")]
	float m_WaitForBoardingTime;
	
	[Attribute("", UIWidgets.EditBox, "Name of character which AI will wait for as driver")]
	string m_sDriverCharacterName;
	
	private IEntity m_pDriverEntity = null;	
	
	float GetWaitForBoardingTime()
	{
		return m_WaitForBoardingTime;
	}
	
	void SetWaitForBoardingTime(float newTime)
	{
		 m_WaitForBoardingTime = newTime;
	}
	
	void SetDriver(IEntity driver) 
	{
		m_pDriverEntity = driver;
	}
	
	IEntity GetDriver() 
	{
		if (m_pDriverEntity)
		{
			return m_pDriverEntity;
		};
		
		return GetGame().GetWorld().FindEntityByName(m_sDriverCharacterName);
	}
	
	IEntity GetVehicle()
	{
		ChimeraCharacter char = ChimeraCharacter.Cast(GetDriver());
		if (char)
		{
			CompartmentAccessComponent comp = char.GetCompartmentAccessComponent();
			if (comp)
			{
				if (comp.IsInCompartment())
				{
					BaseCompartmentSlot slot = comp.GetCompartment();
					if (slot)
					{
						auto testDriverSlot = PilotCompartmentSlot.Cast(slot);
						if (testDriverSlot)
							return slot.GetOwner();
					}
				}
			}
		}
		return null;
	}

};