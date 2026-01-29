[EntityEditorProps(category: "GameScripted/Preview", color: "0 0 0 0", dynamicBox: true)]
class SCR_GenericPreviewEntityClass: SCR_BasePreviewEntityClass
{
};
/*!
Preview entity created from existing entities.
*/
class SCR_GenericPreviewEntity: SCR_BasePreviewEntity
{
	/*!
	Spawn preview entity from existing entity.
	\param entity Entity
	\param previewPrefab Prefab from which the preview entity will be spawned from. Apart from file path, it can be also class name
	\param world World in which the preview will be spawned
	\param spawnParams Spawn params of the preview
	\param material Material of the preview
	\return Entity preview
	*/
	static SCR_BasePreviewEntity SpawnPreviewFromEntity(IEntity entity, ResourceName previewPrefab, BaseWorld world = null, EntitySpawnParams spawnParams = null, ResourceName material = ResourceName.Empty, EPreviewEntityFlag flags = 0)
	{
		if (!entity)
		{
			Print("No entity defined", LogLevel.WARNING);
			return null;
		}
		
		array<ref SCR_BasePreviewEntry> entries = GetPreviewEntriesFromEntity(entity, spawnParams, flags);
		return SpawnPreview(entries, previewPrefab, world, spawnParams, material, flags);
	}
	
	/*!
	Get preview entries from existing entity.
	\param entity Entity
	\param spawnParams Spawn params of the preview
	\param[out] outEntries Array filled with preview entries
	\return Array of configuration entries.
	*/
	static array<ref SCR_BasePreviewEntry> GetPreviewEntriesFromEntity(IEntity entity, out EntitySpawnParams spawnParams = null, EPreviewEntityFlag flags = 0)
	{
		if (!spawnParams)
			spawnParams = new EntitySpawnParams();
		
		TraceParam trace;
		if (SCR_Enum.HasFlag(flags, EPreviewEntityFlag.GEOMETRY))
		{
			trace = new TraceParam();
			SCR_EntityHelper.GetHierarchyEntityList(entity, trace.ExcludeArray);
		}
		
		array<ref SCR_BasePreviewEntry> entries = {};
		GetPreviewEntries(entity, entries, spawnParams.Transform, -1, flags, trace);
		return entries;
	}
	
	/*!
	Get preview entries from existing entity.
	\param entity Entity
	\param[out] outEntries Array to be filled with entity entries
	\param[out] rootTransform Center pivot of the preview. When zero, transformation of the first entity will be used
	*/
	static void GetPreviewEntries(IEntity entity, out notnull array<ref SCR_BasePreviewEntry> outEntries, out vector rootTransform[4], int parentID = -1, EPreviewEntityFlag flags = 0, TraceParam trace = null)
	{
		vector transform[4];
		if (parentID == -1)
		{
			entity.GetWorldTransform(transform);
			SaveRootTransform(transform, rootTransform);
		}
		else
		{
			//--- Ignore vehicle crew, wouldn't have correct animations
			if (entity.IsInherited(ChimeraCharacter))
				return;
			
			//--- Skip the entity if...
			if (
				//--- ... it has an object (e.g., not a composition folder)...
				entity.GetVObject()
				&&
				(
					//--- ... but the object does not have a mesh (e.g., particle effect)...
					(!entity.GetVObject().ToMeshObject())
					||
					//--- ... or the object is not visible or is not intended for play mode
					(!(entity.GetFlags() & EntityFlags.VISIBLE) || (entity.GetFlags() & EntityFlags.EDITOR_ONLY))
				)
			)
				return;
			
			entity.GetLocalTransform(transform);
		}
		
		SCR_BasePreviewEntry entry = new SCR_BasePreviewEntry(true);
		entry.m_iParentID = parentID;
		parentID = outEntries.Insert(entry);
		
		entry.m_Entity = entity;
		entry.SaveTransform(transform);
		entry.SetScale(GetLocalScale(entity));
		entry.m_iPivotID = GetPivotName(entity);
		
		if (GetMesh(entity, flags, entry, outEntries))
			return;
		
		SCR_SlotCompositionComponent compositionComponent = SCR_SlotCompositionComponent.Cast(entity.FindComponent(SCR_SlotCompositionComponent));
		if (compositionComponent && compositionComponent.CanOrientChildrenToTerrain())
			entry.m_Flags |= EPreviewEntityFlag.ORIENT_CHILDREN;
		
		SCR_HorizontalAlignComponent horizontalComponent = SCR_HorizontalAlignComponent.Cast(entity.FindComponent(SCR_HorizontalAlignComponent));
		if (horizontalComponent)
			entry.m_Flags |= EPreviewEntityFlag.HORIZONTAL;
		
		if (!SCR_Enum.HasFlag(flags, EPreviewEntityFlag.IGNORE_TERRAIN) && (entry.m_iParentID == -1 || SCR_Enum.HasFlag(entry.m_Flags, EPreviewEntityFlag.ORIENT_CHILDREN)))
			SaveTerrainTransform(entity, entry, SCR_Enum.HasFlag(flags, EPreviewEntityFlag.UNDERWATER), trace);
		
		if (entry.m_Shape != EPreviewEntityShape.PREFAB)
		{
			IEntity child = entity.GetChildren();
			while (child)
			{
				GetPreviewEntries(child, outEntries, rootTransform, parentID, flags, trace);
				child = child.GetSibling();
			}
		}
	}
	
	protected static float GetLocalScale(IEntity entity)
	{
		float scale = entity.GetScale();
		if (entity.GetParent())
			scale /= entity.GetParent().GetScale();

		return scale;
	}
	protected static string GetPivotName(IEntity entity)
	{
		IEntity parent = entity.GetParent();
		if (!parent)
			return string.Empty;
		
		TNodeId pivotIndex = entity.GetPivot();
		if (pivotIndex == -1)
			return string.Empty;
				
		Animation anim = parent.GetAnimation();
		if (anim)
		{
			array<string> boneNames = {};
			anim.GetBoneNames(boneNames);
			for (int i = 0, count = boneNames.Count(); i < count; i++)
			{
				if (anim.GetBoneIndex(boneNames[i]) == pivotIndex)
					return boneNames[i];
			}
		}
		return string.Empty;
	}
	protected static bool GetMesh(IEntity entity, EPreviewEntityFlag flags, SCR_BasePreviewEntry entry, out notnull array<ref SCR_BasePreviewEntry> outEntries)
	{
		SCR_BaseAreaMeshComponent areaMeshComponent = SCR_BaseAreaMeshComponent.Cast(entity.FindComponent(SCR_BaseAreaMeshComponent));
		if (areaMeshComponent)
		{
			EAreaMeshShape areaMeshShape = areaMeshComponent.GetShape();
			if (areaMeshShape == EAreaMeshShape.ELLIPSE)
				entry.m_Shape = EPreviewEntityShape.ELLIPSE;
			else if (areaMeshShape == EAreaMeshShape.RECTANGLE)
				entry.m_Shape = EPreviewEntityShape.RECTANGLE;
			else 
				return false;
			
			areaMeshComponent.GetDimensions3D(entry.m_vScale);
			return false;
		}
		
		if (!SCR_Enum.HasFlag(flags, EPreviewEntityFlag.IGNORE_PREFAB))
		{
			SCR_PreviewEntityComponent previewComponent = SCR_PreviewEntityComponent.Cast(entity.FindComponent(SCR_PreviewEntityComponent));
			if (previewComponent && previewComponent.IsRuntime())
			{
				//--- Entries pre-defined in the component, use them instead of scanning entity children dynamically
				if (previewComponent.GetPreviewEntries(outEntries, entry) != 0)
					return true;
				
				//--- Preview defined as a prefab, use that one
				entry.m_Mesh = previewComponent.GetPreviewPrefab();
				entry.m_Shape = EPreviewEntityShape.PREFAB;
				return false;
			}
		}
		
		VObject meshObject = entity.GetVObject();
		if (meshObject)
		{
			entry.m_Mesh = meshObject.GetResourceName();
			entry.m_Shape = EPreviewEntityShape.MESH;
		}
		
		return false;
	}
	protected static void SaveTerrainTransform(IEntity entity, SCR_BasePreviewEntry entry, bool isUnderwater = false, TraceParam trace = null)
	{
		vector worldTransform[4], terrainTransform[4], surfaceBasis[4];
		
		//--- Get world transform
		entity.GetWorldTransform(worldTransform);
		
		//--- Get surface basis
		if (!SCR_TerrainHelper.GetTerrainBasis(worldTransform[3], surfaceBasis, entity.GetWorld(), !isUnderwater, trace))
			return;
		
		//--- Get identity matrix rotated according to the entity
		vector angles = Math3D.MatrixToAngles(worldTransform);
		Math3D.AnglesToMatrix(Vector(angles[0], 0, 0), terrainTransform);
		
		//--- Rotate surface basis
		Math3D.MatrixMultiply3(surfaceBasis, terrainTransform, terrainTransform);
		
		//--- Get local transformation relative to terrain
		Math3D.MatrixInvMultiply3(terrainTransform, worldTransform, terrainTransform);
		entry.m_vAnglesTerrain = Math3D.MatrixToAngles(terrainTransform);
		entry.m_vHeightTerrain = worldTransform[3][1] - surfaceBasis[3][1];
	}
	protected static void SaveRootTransform(out vector transform[4], out vector rootTransform[4])
	{
		if (SCR_Math3D.IsMatrixIdentity(rootTransform))
		{
			//--- When pivot is undefined, transformation of the first entity is used as pivot
			Math3D.MatrixCopy(transform, rootTransform);
			Math3D.MatrixIdentity4(transform);
			
			//--- Correct the matrix when pointing straight up or down (ToDo: More robust solution?)
			vector angles = Math3D.MatrixToAngles(rootTransform);
			if (float.AlmostEqual(Math.AbsFloat(angles[1]), 90))
			{
				if (angles[1] > 0) angles[2] = angles[2] - 90;
				Math3D.AnglesToMatrix(Vector(angles[2] + 180, 0, 0), rootTransform);
			}
		}
		else
		{
			//--- Relative transformation to pivot
			Math3D.MatrixInvMultiply4(rootTransform, transform, transform);
		}
	}
};