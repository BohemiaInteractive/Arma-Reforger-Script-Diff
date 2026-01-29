[ComponentEditorProps(category: "GameScripted/Misc", description: "")]
class SCR_ResupplyMedicalGadgetSupportStationComponentClass : SCR_ResupplySupportStationComponentClass
{
}

class SCR_ResupplyMedicalGadgetSupportStationComponent : SCR_ResupplySupportStationComponent
{
	//------------------------------------------------------------------------------------------------
	override ESupportStationType GetSupportStationType()
	{
		return ESupportStationType.RESUPPLY_MEDICAL;
	}
}
