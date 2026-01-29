class AttachBonePositionPlayerTrackModifier : CinematicTrackModifier
{
	
	[Attribute(defvalue:"", desc:"Bone we want be attached to")]
	string boneToAttach;
	
	[Attribute(defvalue:"false", desc:"If we want to rotate with parent")]
	bool applyRotation;
	
	IEntity player;
	vector transform[4];
	vector boneTransform[4];
	vector originalVector[4];
	ChimeraWorld chimeraWorld;
	
	override void OnInit(World world)
	{
		chimeraWorld = world;
		
		array<int> playerIDs = {};
		GetGame().GetPlayerManager().GetPlayers(playerIDs);
		foreach (int playerID : playerIDs)
		{
			player = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerID);
			if (!player)
				continue;
		}
	}
	
	override vector OnApplyModifierVector(float time, vector originalValue)
	{
				
		if (!player)
			return originalValue;
		
		player.GetTransform(transform);
		
		if (boneToAttach != "") //attach to bone
		{
			Animation entityAnimation = player.GetAnimation();
			
			if (!entityAnimation)
				return originalValue;
			
			TNodeId boneId = entityAnimation.GetBoneIndex(boneToAttach);
			
			if (boneId == -1)
				return originalValue;
			
			entityAnimation.GetBoneMatrix(boneId, boneTransform);
			Math3D.MatrixMultiply4(transform, boneTransform, transform);
			
			if (applyRotation)
			{
				originalVector = { "1 0 0", "0 1 0", "0 0 1", originalValue};
				Math3D.MatrixMultiply4(transform, originalVector, transform);
				
				return transform[3];
			}
			else
			{
				return transform[3] + originalValue;
			}
		}
		
		return originalValue;
	}
};
