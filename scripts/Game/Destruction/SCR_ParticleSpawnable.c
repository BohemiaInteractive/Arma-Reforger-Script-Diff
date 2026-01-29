[BaseContainerProps(), SCR_Spawnable_ParticleTitle()]
class SCR_ParticleSpawnable : SCR_BaseSpawnable
{
	[Attribute(ResourceName.Empty, UIWidgets.ResourceNamePicker, "Particle effect to spawn", "ptc")]
	ResourceName m_Particle;

	[Attribute("1", UIWidgets.CheckBox, "If true, the particle effect will play at the object's bounding box instead of at its origin")]
	bool m_bAtCenter;
	
	[Attribute("1", desc: "If true, the particle effect will play rotated in the hit direction.")]
	bool m_bDirectional;
	
#ifdef WORKBENCH
	//------------------------------------------------------------------------------------------------
	void GetPositionAndRotation(out notnull array<vector> positionAndRotation)
	{
		positionAndRotation.Clear();
		positionAndRotation.Insert(m_vOffsetPosition);
		positionAndRotation.Insert(m_vOffsetRotation);
	}

	//------------------------------------------------------------------------------------------------
	//! Returns true when attributes are the same
	//! Returns false otherwise
	override bool CompareAttributes(SCR_BaseSpawnable other)
	{
		SCR_ParticleSpawnable otherParticle = SCR_ParticleSpawnable.Cast(other);
		
		if (!super.CompareAttributes(other))
			return false;
		
		if (otherParticle.m_Particle != m_Particle)
			return false;
		
		if (otherParticle.m_bAtCenter != m_bAtCenter)
			return false;
		
		if (otherParticle.m_bDirectional != m_bDirectional)
			return false;
		
		return true;
	}
	//------------------------------------------------------------------------------------------------
	override void SetVariables(WorldEditorAPI api, IEntitySource source, array<ref ContainerIdPathEntry> path, int index)
	{
		super.SetVariables(api, source, path, index);
		
		// Set all variables of the spawn object
		api.SetVariableValue(source, path, "m_Particle", m_Particle);
		api.SetVariableValue(source, path, "m_bAtCenter", m_bAtCenter.ToString(true));
		api.SetVariableValue(source, path, "m_bDirectional", m_bDirectional.ToString(true));
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CreateObject(WorldEditorAPI api, IEntitySource source, array<ref ContainerIdPathEntry> path, int index)
	{
		if (!AlreadyExists(api, source, index))
		{
			api.CreateObjectArrayVariableMember(source, path, "m_aPhaseDestroySpawnObjects", "SCR_ParticleSpawnable", index);
			return true;
		}
		
		return false;
	}
#endif
	
	//------------------------------------------------------------------------------------------------
	ParticleEffectEntity SpawnAsChild(IEntity owner, SCR_HitInfo hitInfo, bool snapToTerrain = false)
	{
		if (m_Particle == ResourceName.Empty)
			return null;
		
		vector spawnMat[4];
		GetSpawnTransform(owner, spawnMat);
		
		if (m_bAtCenter)
		{
			vector mins, maxs;
			owner.GetBounds(mins, maxs);
			vector center = (maxs - mins) * 0.5 + mins;
			spawnMat[3] = center.Multiply4(spawnMat);
		}
		
		vector position = spawnMat[3];
		if (snapToTerrain)
		{
			position[1] = SCR_TerrainHelper.GetTerrainY(position, owner.GetWorld());
			spawnMat[3] = position;
		}
		
		if (m_bDirectional)
		{
			vector newRight, newUp, newForward;
			
			newUp = -hitInfo.m_HitDirection;
			newRight = newUp * spawnMat[2];
			newForward = newUp * newRight;
			
			spawnMat[0] = newRight;
			spawnMat[1] = newUp;
			spawnMat[2] = newForward;
		}
		
		return SCR_DestructionCommon.PlayParticleEffect_Child(m_Particle, hitInfo.m_DamageType, owner, spawnMat);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Spawns the object
	override ParticleEffectEntity Spawn(IEntity owner, Physics parentPhysics, SCR_HitInfo hitInfo, bool snapToTerrain = false)
	{
		if (!hitInfo || m_Particle == ResourceName.Empty)
			return null;
		
		vector spawnMat[4];
		GetSpawnTransform(owner, spawnMat);
		
		if (m_bAtCenter)
		{
			vector mins, maxs;
			owner.GetBounds(mins, maxs);
			vector center = (maxs - mins) * 0.5 + mins;
			spawnMat[3] = center.Multiply4(spawnMat);
		}
		
		vector position = spawnMat[3];
		if (snapToTerrain)
		{
			position[1] = SCR_TerrainHelper.GetTerrainY(position, owner.GetWorld());
			spawnMat[3] = position;
		}
		
		if (m_bDirectional)
		{
			vector newRight, newUp, newForward;
			
			newUp = -hitInfo.m_HitDirection;
			newRight = newUp * spawnMat[2];
			newForward = newUp * newRight;
			
			spawnMat[0] = newRight;
			spawnMat[1] = newUp;
			spawnMat[2] = newForward;
		}
		
		return SCR_DestructionCommon.PlayParticleEffect_Transform(m_Particle, hitInfo.m_DamageType, spawnMat);
	}
}