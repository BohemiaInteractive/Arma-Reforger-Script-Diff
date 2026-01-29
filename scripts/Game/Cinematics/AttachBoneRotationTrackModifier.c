class AttachBoneRotationTrackModifier : CinematicTrackModifier
{
	[Attribute(defvalue:"", desc:"Entity we want be attached to")]
	string entityToAttach;
	
	[Attribute(defvalue:"", desc:"Bone we want be attached to")]
	string boneToAttach;
	
	vector transform[4];
	vector boneTransform[4];
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
		
		if (boneToAttach != "") //attach to bone
		{
			Animation entityAnimation = entity.GetAnimation();
			
			if (!entityAnimation)
				return originalValue;
			
			TNodeId boneId = entityAnimation.GetBoneIndex(boneToAttach);
			
			if (boneId == -1)
				return originalValue;
			
			entityAnimation.GetBoneMatrix(boneId, boneTransform);
			Math3D.MatrixMultiply4(transform, boneTransform, transform);
			
			Math3D.AnglesToMatrix(originalValue, originalVector);
			Math3D.MatrixMultiply4(transform, originalVector, transform);
			
			return  Math3D.MatrixToAngles(transform);
		}
		
		return originalValue;
	}
};
