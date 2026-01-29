[BaseContainerProps(), SCR_Spawnable_PrefabTitle()]
class SCR_PrefabSpawnable : SCR_BaseSpawnable
{
	[Attribute(ResourceName.Empty, UIWidgets.ResourcePickerThumbnail, "Prefabs to spawn (spawns ALL of them)", "et")]
	ref array<ResourceName> m_Prefabs;
	[Attribute("0.1", UIWidgets.Slider, "Damage received to physics impulse (speed / mass) multiplier", "0 10000 0.01")]
	float m_fDamageToImpulse;
	[Attribute("0.25", UIWidgets.Slider, "Random linear velocity multiplier (m/s)", "0 200 0.1")]
	float m_fRandomVelocityLinear;
	[Attribute("45", UIWidgets.Slider, "Random angular velocity multiplier (deg/s)", "0 3600 0.1")]
	float m_fRandomVelocityAngular;
	[Attribute("0", UIWidgets.CheckBox, "Whether the spawned prefabs should be set as children (sets auto-transform)")]
	bool m_bSpawnAsChildren;
	
#ifdef WORKBENCH
	//------------------------------------------------------------------------------------------------
	//! Returns true when attributes are the same
	//! Returns false otherwise
	override bool CompareAttributes(SCR_BaseSpawnable other)
	{
		SCR_PrefabSpawnable otherPrefab = SCR_PrefabSpawnable.Cast(other);
		
		if (!super.CompareAttributes(other))
			return false;
		
		int count = m_Prefabs.Count();
		if (otherPrefab.m_Prefabs.Count() != count)
			return false;
		
		for (int i = count - 1; i >= 0; i--)
		{
			if (otherPrefab.m_Prefabs[i] != m_Prefabs[i])
				return false;
		}
		
		if (otherPrefab.m_fDamageToImpulse != m_fDamageToImpulse)
			return false;
		
		if (otherPrefab.m_fRandomVelocityLinear != m_fRandomVelocityLinear)
			return false;
		
		if (otherPrefab.m_fRandomVelocityAngular != m_fRandomVelocityAngular)
			return false;
		
		if (otherPrefab.m_bSpawnAsChildren != m_bSpawnAsChildren)
			return false;
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override void SetVariables(WorldEditorAPI api, IEntitySource source, array<ref ContainerIdPathEntry> path, int index)
	{
		super.SetVariables(api, source, path, index);
		
		string prefabsArray = "";
		// Set all variables of the spawn object
		for (int i = 0, count = m_Prefabs.Count(); i < count; i++)
		{
			prefabsArray += m_Prefabs[i];
			
			if (i != count - 1) // Not last item
				prefabsArray += ",";
		}
		
		api.SetVariableValue(source, path, "m_Prefabs", prefabsArray);
		
		api.SetVariableValue(source, path, "m_fDamageToImpulse", m_fDamageToImpulse.ToString());
		api.SetVariableValue(source, path, "m_fRandomVelocityLinear", m_fRandomVelocityLinear.ToString());
		api.SetVariableValue(source, path, "m_fRandomVelocityAngular", m_fRandomVelocityAngular.ToString());
		api.SetVariableValue(source, path, "m_bSpawnAsChildren", m_bSpawnAsChildren.ToString(true));
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CreateObject(WorldEditorAPI api, IEntitySource source, array<ref ContainerIdPathEntry> path, int index)
	{
		if (!AlreadyExists(api, source, index))
		{
			api.CreateObjectArrayVariableMember(source, path, "m_aPhaseDestroySpawnObjects", "SCR_PrefabSpawnable", index);
			return true;
		}
		
		return false;
	}
#endif
	
	//------------------------------------------------------------------------------------------------
	//! Spawns the object
	override IEntity Spawn(IEntity owner, Physics parentPhysics, SCR_HitInfo hitInfo, bool snapToTerrain = false)
	{
		if (!hitInfo)
			return null;
		
		int numModelPrefabs = 0;
		if (m_Prefabs)
			numModelPrefabs = m_Prefabs.Count();
		
		for (int i = 0; i < numModelPrefabs; i++)
		{
			ResourceName prefabPath = m_Prefabs[i];
			
			bool isPrefab;
			if (SCR_Global.GetResourceContainsComponent(prefabPath, "RplComponent", isPrefab) && RplSession.Mode() == RplMode.Client)
				continue;
			
			if (!isPrefab)
				continue;
			
			vector spawnMat[4];
			GetSpawnTransform(owner, spawnMat, m_bSpawnAsChildren);
			
			// Divide the matrix by owner scale to remove scale from transformation
			float ownerScale = owner.GetScale();
			if (ownerScale > 0) // Avoid division by zero
			{
				spawnMat[0] = spawnMat[0] / ownerScale; // X axis
				spawnMat[1] = spawnMat[1] / ownerScale; // Y axis
				spawnMat[2] = spawnMat[2] / ownerScale; // Z axis
				// Position (spawnMat[3]) is NOT divided - it should remain in world space
			}
			
			EntitySpawnParams prefabSpawnParams = EntitySpawnParams();
			prefabSpawnParams.Transform = spawnMat;
			// Use Scale parameter to set scale instead of using transformation
			prefabSpawnParams.Scale = ownerScale;
			
			IEntity spawnedPrefab = GetGame().SpawnEntityPrefab(Resource.Load(prefabPath), null, prefabSpawnParams);
			if (!spawnedPrefab)
				continue;
			
			if (m_bSpawnAsChildren)
			{
				owner.AddChild(spawnedPrefab, -1, EAddChildFlags.AUTO_TRANSFORM);
				continue;
			}
			
			Physics prefabPhysics = spawnedPrefab.GetPhysics();
			if (!prefabPhysics || !prefabPhysics.IsDynamic())
				continue;
			
			//hotfix for spawned prefabs getting stuck in terrain causing low fps
			vector mins, maxs;
			spawnedPrefab.GetWorldBounds(mins, maxs);
						
			vector entityOrigin = spawnedPrefab.GetOrigin();
			float terrainYMins = GetGame().GetWorld().GetSurfaceY(mins[0], mins[2]);	
			float terrainYMaxs = GetGame().GetWorld().GetSurfaceY(maxs[0], maxs[2]);
				
			if ((mins[1] < terrainYMins || mins[1] < terrainYMaxs) && (maxs[1] > terrainYMins || maxs[1] > terrainYMaxs))
			{
				float highestTerrainY;
				if (terrainYMaxs > terrainYMins)
					highestTerrainY = terrainYMaxs;
				else
					highestTerrainY = terrainYMins;
				
				float newHeight = highestTerrainY - mins[1] + entityOrigin[1] + 0.1;
				spawnedPrefab.SetOrigin({entityOrigin[0], newHeight, entityOrigin[2]});
			}
			
			float dmgSpeed = hitInfo.m_HitDamage * m_fDamageToImpulse / prefabPhysics.GetMass();
			
			vector linearVelocity = hitInfo.m_HitDirection * Math.RandomFloat(0, 1);
			linearVelocity += Vector(Math.RandomFloat(-1, 1), Math.RandomFloat(-1, 1), Math.RandomFloat(-1, 1)) * m_fRandomVelocityLinear;
			linearVelocity *= dmgSpeed;
			vector angularVelocity = Vector(Math.RandomFloat(-1, 1), Math.RandomFloat(-1, 1), Math.RandomFloat(-1, 1)) * Math.RandomFloat(0.25, 4) * m_fRandomVelocityAngular;
			angularVelocity *= dmgSpeed;
			
			if (parentPhysics)
			{
				linearVelocity += parentPhysics.GetVelocity();
				angularVelocity += parentPhysics.GetAngularVelocity();
			}
			
			prefabPhysics.SetVelocity(linearVelocity);
			prefabPhysics.SetAngularVelocity(angularVelocity * Math.DEG2RAD);
		}
		
		return null; // We spawned multiple entities
	}
}