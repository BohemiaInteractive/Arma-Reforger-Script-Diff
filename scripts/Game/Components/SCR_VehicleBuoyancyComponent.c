class SCR_VehicleBuoyancyComponentClass : VehicleBuoyancyComponentClass
{	
}

//----------------------------------------------------------------------------
class SCR_VehicleBuoyancyComponent : VehicleBuoyancyComponent
{
	protected ref ScriptInvokerVoid m_OnWaterEnter;
	protected ref ScriptInvokerVoid m_OnWaterExit;
	
	//----------------------------------------------------------------------------
	ScriptInvokerVoid GetOnWaterEnter()
	{
		if (!m_OnWaterEnter)
			m_OnWaterEnter = new ScriptInvokerVoid();
		
		return m_OnWaterEnter;
	}
	
	//----------------------------------------------------------------------------
	ScriptInvokerVoid GetOnWaterExit()
	{
		if (!m_OnWaterExit)
			m_OnWaterExit = new ScriptInvokerVoid();
		
		return m_OnWaterExit;
	}
	
	//----------------------------------------------------------------------------
	override void OnEnterWater()
	{
		if (m_OnWaterEnter)
			m_OnWaterEnter.Invoke();
	}
	
	//----------------------------------------------------------------------------
	override void OnExitWater()
	{
		if (m_OnWaterExit)
			m_OnWaterExit.Invoke();
	}
}