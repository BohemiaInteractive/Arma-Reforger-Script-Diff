class AttachRotationTrackModifier : CinematicTrackModifier
{
	[Attribute(defvalue:"", desc:"Entity we want be attached to")]
	string entityToAttach;
	
	vector transform[4];
	vector originalVector[4];
	ChimeraWorld chimeraWorld;
	
	override void OnInit(World world)
	{
		chimeraWorld = world;
	}
	
	override vector OnApplyModifierVector(float time, vector originalValue)
	{
		
		IEntity entity =  chimeraWorld.FindEntityByName(entityToAttach);
		
		if (!entity)
			return originalValue;
		
		entity.GetTransform(transform);
		
		Math3D.AnglesToMatrix(originalValue, originalVector);
		Math3D.MatrixMultiply4(transform, originalVector, transform);
		
		return Math3D.MatrixToAngles(transform);
	}
};
