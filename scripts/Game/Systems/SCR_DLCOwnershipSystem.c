class SCR_DLCOwnershipSystem : GameSystem
{
	protected ref ScriptInvoker m_OnOwnershipNegative = new ScriptInvoker();

	//------------------------------------------------------------------------------------------------
	override static void InitInfo(WorldSystemInfo outInfo)
	{
		outInfo.SetAbstract(false);
		outInfo.SetLocation(WorldSystemLocation.Client);
		outInfo.AddPoint(WorldSystemPoint.Frame);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnInit()
	{
		
	}
	
	ScriptInvoker GetOnOwnershipNegative()
	{
		return m_OnOwnershipNegative;
	}
	
	void OnOwnershipNegative(SCR_EDLCInteractionType interaction)
	{
		m_OnOwnershipNegative.Invoke(interaction);
	}
}

enum SCR_EDLCInteractionType
{
	NONE,
	VEHICLE_DRIVER,
	ITEM_EQUIP,
	SCENARIO,
	OBJECT
};