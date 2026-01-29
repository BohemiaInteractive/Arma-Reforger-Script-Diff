class SCR_DestructionHeatmapEntry: DestructionHeatmapEntry
{
	//static SCR_DebugShapeManager = new SCR_DebugShapeManager;
	event override void OnDiag()
	{
		#ifndef ENABLE_DIAG
		return;
		#endif
		
		// might want to change colors based on category type.
		
		vector mins = GetMins();
		vector maxs = GetMaxs();
		
		Shape box = Shape.Create(ShapeType.BBOX, 0x30ff0000, ShapeFlags.TRANSP | ShapeFlags.NOZWRITE | ShapeFlags.ADDITIVE | ShapeFlags.ONCE, mins, maxs);
	}
}
	
