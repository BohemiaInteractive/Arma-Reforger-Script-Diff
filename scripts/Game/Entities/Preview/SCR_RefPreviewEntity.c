//#define PREVIEW_ENTITY_SHOW_REFERENCE //--- Don't delete reference entity after transformation ended, to show how it looked like.

[EntityEditorProps(category: "GameScripted/Preview", description: "")]
class SCR_RefPreviewEntityClass: SCR_EditablePreviewEntityClass
{
}

//! Reference entity used to position edited entities according to preview.
class SCR_RefPreviewEntity: SCR_EditablePreviewEntity
{
	protected ref set<SCR_EditableEntityComponent> m_aEditedEntities;
	protected bool m_bIsInstant;
	
	//------------------------------------------------------------------------------------------------
	//! Create and apply reference entity used for entity placement once transformation is confirmed.
	//! \param[in] entity Directly edited entity
	//! \param[in] param Transformation params
	static void SpawnAndApplyReference(SCR_EditableEntityComponent entity, SCR_EditorPreviewParams param)
	{
#ifdef PREVIEW_ENTITY_SHOW_REFERENCE
		const ResourceName material = "{D0126AF0E6A27141}Common/Materials/Colors/colorRed.emat";
#else
		const ResourceName material;
#endif
		EPreviewEntityFlag flags = EPreviewEntityFlag.IGNORE_TERRAIN | EPreviewEntityFlag.IGNORE_PREFAB | EPreviewEntityFlag.ONLY_EDITABLE;
		
		SCR_RefPreviewEntity refEntity = SCR_RefPreviewEntity.Cast(SCR_RefPreviewEntity.SpawnPreviewFromEditableEntity(entity, "SCR_RefPreviewEntity", GetGame().GetWorld(), null, material, flags));
		if (!refEntity)
		{
			Print("SCR_RefPreviewEntity:: refEntity was not spawned!", LogLevel.WARNING);
			return;
		}
		
		refEntity.m_EditableEntity = entity;
		refEntity.SetAsInstant();
		refEntity.ApplyReference(param);
		
#ifndef PREVIEW_ENTITY_SHOW_REFERENCE
		delete refEntity;
#endif
	}
	
	//------------------------------------------------------------------------------------------------
	//! Apply changes in reference entity to its original entities.
	//! \param[in] param Transformation params
	void ApplyReference(SCR_EditorPreviewParams param)
	{
		//--- Get navmesh rebuild areas *BEFORE* the change
		SCR_AIWorld aiWorld = SCR_AIWorld.Cast(GetGame().GetAIWorld());
		array<ref Tuple2<vector, vector>> areas = {}; //--- Min, max
		array<bool> redoAreas = {};
		if (aiWorld)
		{
			foreach (SCR_BasePreviewEntity child: m_aChildren)
			{			
				if (child.m_Entity)
					aiWorld.GetNavmeshRebuildAreas(child.m_Entity, areas, redoAreas);
			}
		}
		
		//--- Update reference (ToDo: Do it only when needed)
		vector transform[4];
		param.GetWorldTransform(transform);
		UpdateReference(transform, param.m_VerticalMode, param.m_bIsUnderwater);
		
		//--- Apply changes on all children, and get navmesh areas *AFTER* the change
		SCR_RefPreviewEntity childRef;
		if (aiWorld)
		{
			if (m_aChildren)
			{
				foreach (SCR_BasePreviewEntity child: m_aChildren)
				{
					childRef = SCR_RefPreviewEntity.Cast(child);
					if (childRef)
					{
						childRef.ApplyChild(param, true, m_aEditedEntities, m_bIsInstant);
						if (child.m_Entity)
							aiWorld.GetNavmeshRebuildAreas(child.m_Entity, areas, redoAreas);
					}
				}
			}
			aiWorld.RequestNavmeshRebuildAreas(areas, redoAreas);
		}
		else
		{
			if (m_aChildren)
			{
				foreach (SCR_BasePreviewEntity child: m_aChildren)
				{
					childRef = SCR_RefPreviewEntity.Cast(child);
					if (childRef)
						childRef.ApplyChild(param, true, m_aEditedEntities, m_bIsInstant);
				}
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Mark the reference entity as applied at the same time as it's created.
	void SetAsInstant()
	{
		m_bIsInstant = true;
	}

	//------------------------------------------------------------------------------------------------
	protected void ApplyChild(SCR_EditorPreviewParams param, bool isDirectChild, set<SCR_EditableEntityComponent> editedEntities, bool isInstant)
	{
		if (!m_EditableEntity)
			return;

		vector transform[4];
		GetWorldTransform(transform);
		
		bool changeTransformation = true;
		if (isDirectChild)
		{
			SCR_EditableEntityComponent currentParent = m_EditableEntity.GetParentEntity();
			SCR_EditableEntityComponent newParent;
			bool changeParent = false;
			
			if (param.m_Target && (param.m_TargetInteraction == EEditableEntityInteraction.LAYER || param.m_TargetInteraction == EEditableEntityInteraction.PASSENGER))
			{
				//--- Hovering over target layer
				newParent = param.m_Target;
				changeParent = true;
				changeTransformation = false;
			}
			else if (param.m_Parent != currentParent && param.m_Parent != m_EditableEntity && (param.m_bParentChanged || !currentParent || !currentParent.HasEntityFlag(EEditableEntityFlag.INDIVIDUAL_CHILDREN)))
			{
				//--- Entered target layer (and the target is not the entity itself)
				newParent = param.m_Parent;
				changeParent = true;
			}
			if (changeParent)
			{
				//--- Don't change parent of group members when their group is also being edited
				SCR_EditableEntityComponent editableGroup = m_EditableEntity.GetAIGroup();
				if (m_EditableEntity == editableGroup || !editableGroup || editedEntities.Find(editableGroup) == -1)
				{
					//--- Set the parent, and update the reference so all other entities use it as well (in case it changed, like when moving character to root)
					param.m_Parent = m_EditableEntity.SetParentEntity(newParent, true);
				}
			}
		}
		
		if (changeTransformation)
		{
			//--- Don't transform vehicle crew when the vehicle is also being edited
			SCR_EditableEntityComponent editableVehicle = m_EditableEntity.GetVehicle();
			if (!editableVehicle || editedEntities.Find(editableVehicle) == -1)
			{
				 if (isInstant)
				{
					vector localPos = m_EditableEntity.GetOwner().CoordToLocal(transform[3]);
					localPos -= m_EditableEntity.GetIconPos();
					transform[3] = m_EditableEntity.GetOwner().CoordToParent(localPos);
				}
				m_EditableEntity.SetTransform(transform, isDirectChild && !isInstant);
			}
			
			if (m_aChildren)
			{
				SCR_RefPreviewEntity childRef;
				foreach (SCR_BasePreviewEntity child: m_aChildren)
				{
					childRef = SCR_RefPreviewEntity.Cast(child);
					if (childRef)
						childRef.ApplyChild(param, false, editedEntities, isInstant);
				}
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateReference(vector transform[4], EEditorTransformVertical verticalMode, bool isUnderwater)
	{
		TraceParam trace;
		if (verticalMode == EEditorTransformVertical.GEOMETRY)
		{
			trace = new TraceParam();
			trace.ExcludeArray = m_aExcludeArray;
		}		
		
		float currentHeight;
		if (!SCR_Enum.HasFlag(m_Flags, EPreviewEntityFlag.IGNORE_TERRAIN))
		{
			vector currentPos = GetWorldTransformAxis(3);
			currentHeight = currentPos[1] - SCR_TerrainHelper.GetTerrainY(currentPos, GetWorld(), !SCR_Enum.HasFlag(m_Flags, EPreviewEntityFlag.UNDERWATER), trace);
		}
		float height = transform[3][1] - SCR_TerrainHelper.GetTerrainY(transform[3], GetWorld(), !isUnderwater, trace);
		
		m_fHeightTerrain = 0;
		SetPreviewTransform(transform, verticalMode, height - currentHeight, isUnderwater, trace);
	}

	//------------------------------------------------------------------------------------------------
	override protected void EOnPreviewInit(SCR_BasePreviewEntry entry, SCR_BasePreviewEntity root)
	{
		super.EOnPreviewInit(entry, root);
		
		SCR_RefPreviewEntity refRoot = SCR_RefPreviewEntity.Cast(root);
		if (refRoot)
		{
			if (!refRoot.m_aEditedEntities)
				refRoot.m_aEditedEntities = new set<SCR_EditableEntityComponent>();
			
			refRoot.m_aEditedEntities.Insert(m_EditableEntity);
		}
	}
}
