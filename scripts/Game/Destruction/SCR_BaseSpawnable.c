[BaseContainerProps()]
class SCR_BaseSpawnable
{
	[Attribute("0 0 0", UIWidgets.Coords, desc: "Positional offset (in local space to the destructible)", params: "inf inf 0 purpose=coords space=entity coordsVar=m_vOffsetPosition")]
	protected vector m_vOffsetPosition;
	[Attribute("0 0 0", UIWidgets.Coords, desc: "Yaw, pitch & roll offset (in local space to the destructible)", params: "inf inf 0 purpose=angles space=entity anglesVar=m_vOffsetRotation")]
	protected vector m_vOffsetRotation;
	
#ifdef WORKBENCH
	//------------------------------------------------------------------------------------------------
	void SetVariables(WorldEditorAPI api, IEntitySource source, array<ref ContainerIdPathEntry> path, int index)
	{
		if (source.GetResourceName().Contains("BrickWall_01/BrickWall_01_white_2m.et"))
			Print("BROKEN");
		
		// Set all variables of the spawn object
		api.SetVariableValue(source, path, "m_vOffsetPosition", string.Format("%1 %2 %3", m_vOffsetPosition[0], m_vOffsetPosition[1], m_vOffsetPosition[2]));
		api.SetVariableValue(source, path, "m_vOffsetRotation", string.Format("%1 %2 %3", m_vOffsetRotation[0], m_vOffsetRotation[1], m_vOffsetRotation[2]));
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns true when attributes are the same
	//! Returns false otherwise
	bool CompareAttributes(SCR_BaseSpawnable other)
	{
		if (other.m_vOffsetPosition != m_vOffsetPosition)
			return false;
		
		if (other.m_vOffsetRotation != m_vOffsetRotation)
			return false;
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	bool AlreadyExists(WorldEditorAPI api, IEntitySource source, int index)
	{
		array<ref BaseDestructionPhase> phases = {};
		source.Get("DamagePhases", phases);
		
		if (phases && phases.IsIndexValid(index))
		{
			SCR_BaseDestructionPhase phase = SCR_BaseDestructionPhase.Cast(phases[index]);
			for (int i = phase.m_aPhaseDestroySpawnObjects.Count() - 1; i >= 0; i--)
			{
				if (phase.m_aPhaseDestroySpawnObjects[i].Type() == Type())
				{
					if (CompareAttributes(phase.m_aPhaseDestroySpawnObjects[i]))
						return true;
				}
			}
		}
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	bool CreateObject(WorldEditorAPI api, IEntitySource source, array<ref ContainerIdPathEntry> path, int index)
	{
		if (!AlreadyExists(api, source, index))
		{
			api.CreateObjectArrayVariableMember(source, path, "m_aPhaseDestroySpawnObjects", "SCR_BaseSpawnable", index);
			return true;
		}
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	void CopyToSource(WorldEditorAPI api, IEntitySource source, array<ref ContainerIdPathEntry> path, int index, string currentObjectName)
	{
		if (!CreateObject(api, source, path, index))
			return;
		
		// Change the path to the current spawn object
		int last = path.Insert(new ContainerIdPathEntry(currentObjectName, index));
		
		SetVariables(api, source, path, index);
		
		path.Remove(last);
	}
#endif
	
	//------------------------------------------------------------------------------------------------
	//! Calculates the spawn tranformation matrix for the object
	void GetSpawnTransform(IEntity owner, out vector outMat[4], bool localCoords = false)
	{
		if (localCoords)
		{
			Math3D.AnglesToMatrix(m_vOffsetRotation, outMat);
			// TODO: Remove hotfix for sleeping/static object
			if (m_vOffsetPosition == vector.Zero)
				outMat[3] = vector.Up * 0.001;
			else
				outMat[3] = m_vOffsetPosition;
		}
		else
		{
			vector localMat[4], parentMat[4];
			owner.GetWorldTransform(parentMat);
			Math3D.AnglesToMatrix(m_vOffsetRotation, localMat);
			localMat[3] = m_vOffsetPosition;
			
			Math3D.MatrixMultiply4(parentMat, localMat, outMat);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Spawns the object
	IEntity Spawn(IEntity owner, Physics parentPhysics, SCR_HitInfo hitInfo, bool snapToTerrain = false)
	{
		return null;
	}
}

class SCR_Spawnable_SmallDebrisTitle : BaseContainerCustomTitle
{
	override bool _WB_GetCustomTitle(BaseContainer source, out string title)
	{
		title = "Small Debris";
		return true;
	}
}

class SCR_Spawnable_PrefabTitle : BaseContainerCustomTitle
{
	override bool _WB_GetCustomTitle(BaseContainer source, out string title)
	{
		title = "Prefab";
		return true;
	}
}

class SCR_Spawnable_ParticleTitle : BaseContainerCustomTitle
{
	override bool _WB_GetCustomTitle(BaseContainer source, out string title)
	{
		title = "Particle Effect";
		return true;
	}
}