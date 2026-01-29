class SCR_AIDecoIsCompartmentTypeAllowed : DecoratorScripted
{
	static const string PORT_COMPARTMENT_TYPE	=	"CompartmentType";
	static const string PORT_BOARDING_PARAMS 	=	"BoardingParams";
	
	protected override bool TestFunction(AIAgent owner)
	{
		ECompartmentType compartmentType;
		SCR_AIBoardingParameters boardingParams;
		GetVariableIn(PORT_COMPARTMENT_TYPE, compartmentType);
		GetVariableIn(PORT_BOARDING_PARAMS, boardingParams);
		if (!boardingParams)
			return true;
		switch (compartmentType)
		{
			case ECompartmentType.PILOT:
			{
				return boardingParams.m_bIsDriverAllowed;
				break;
			}
			case ECompartmentType.TURRET:
			{
				return boardingParams.m_bIsGunnerAllowed;
				break;
			}
			case ECompartmentType.CARGO:
			{
				return boardingParams.m_bIsCargoAllowed;
				break;
			}		
		}
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsIn = {
		PORT_COMPARTMENT_TYPE,
		PORT_BOARDING_PARAMS
	};
	override TStringArray GetVariablesIn()
	{
		return s_aVarsIn;
	}
};