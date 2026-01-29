class SCR_AIDecoTargetWithinTurretAimingLimits : DecoratorScripted
{
	// Inputs
	protected static const string PORT_TURRET_ENTITY = "TurretEntity";
	protected static const string PORT_TARGET_POS = "TargetPos";
	
	//---------------------------------------------------------------------
	override bool TestFunction(AIAgent owner)
	{
		// Read inputs
		IEntity turretEntity;
		vector targetPos;
		GetVariableIn(PORT_TURRET_ENTITY, turretEntity);
		GetVariableIn(PORT_TARGET_POS, targetPos);
		
		if (!turretEntity || !targetPos)
			return false;
		
		TurretComponent turretComp = TurretComponent.Cast(turretEntity.FindComponent(TurretComponent));
		if (!turretComp)
			return false;
		
		vector aimingExcessDeg = turretComp.GetAimingAngleExcess(targetPos);
		bool withinAimingLimits = aimingExcessDeg[0] == 0 && aimingExcessDeg[1] == 0;
		
		return withinAimingLimits;
	}
	
	//---------------------------------------------------------------------
	protected static ref TStringArray s_aVarsIn = { PORT_TURRET_ENTITY, PORT_TARGET_POS };
	override TStringArray GetVariablesIn() { return s_aVarsIn; };
	
	override static string GetOnHoverDescription() { return "Returns true if given position is within aiming limits of the TurretComponent attached to provided entity."; }
}