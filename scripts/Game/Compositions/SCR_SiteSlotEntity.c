//#define SLOT_ENTITY_DEBUG

[EntityEditorProps(category: "GameScripted/Editor", description: "Slot in which entities can be placed in", color: "255 0 0 255")]
class SCR_SiteSlotEntityClass: GenericEntityClass
{
	[Attribute(defvalue: "-1", category: "Slot", desc: "Forced rotation step. 0 means unlimited rotation, -1 disables rotation.")]
	private float m_fRotationStep;
	
	/*!
	Get rotation step of the slot.
	\return Rotation step in degrees. 0 means unlimited rotation, -1 disables rotation.
	*/
	float GetRotationStep()
	{
		return m_fRotationStep;
	}
};

/** @ingroup Editor_Entities
*/

/*!
A slot is a suggested area for placing entities and compositions.
It has guarranteed traits like flatness or road going through which make editing easier for the user.
*/
class SCR_SiteSlotEntity : GenericEntity
{
	/*!
	Spawn entity in the prefab and set it as occupant.
	\param Prefab resource
	\param rotation Absolute angle in degrees (-1 to use slot angle)
	\param verticalMode Vertical editing mode (-1 to ignore it completely)
	\return Entity
	*/
	IEntity SpawnEntityInSlot(Resource resource, float rotation = -1, EEditorTransformVertical verticalMode = EEditorTransformVertical.TERRAIN)
	{
		//--- Already occupied
		if (GetOccupant()) return null;
		
		ArmaReforgerScripted game = GetGame();
		if (!game) return null;
		
		//--- Get slot transformation
		EntitySpawnParams spawnParams = new EntitySpawnParams;
		GetWorldTransform(spawnParams.Transform);
		spawnParams.TransformMode = ETransformMode.WORLD;
		
		//--- Apply rotation
		if (rotation != -1)
		{
			vector angles = Math3D.MatrixToAngles(spawnParams.Transform);
			angles[0] = rotation;
			Math3D.AnglesToMatrix(angles, spawnParams.Transform);
		}
		
		//--- Spawn the prefab
		string resourceName = resource.GetResource().GetResourceName();
		IEntity baseEntity = game.SpawnEntityPrefab(resource, GetWorld(), spawnParams);
		if (!baseEntity) return null;
		
		//--- Mark as occupied
		SetOccupant(baseEntity);
		
		//--- Orient according to vertical settings
		bool updateNavmesh = true;
		if (verticalMode != -1)
		{
			SCR_EditableEntityComponent entity = SCR_EditableEntityComponent.GetEditableEntity(baseEntity);
			if (entity)
			{
				SCR_EditorPreviewParams params = SCR_EditorPreviewParams.CreateParams(spawnParams.Transform, verticalMode: verticalMode);
				SCR_RefPreviewEntity.SpawnAndApplyReference(entity, params);
				updateNavmesh = false;
			}
			else
			{
				//--- Handled in SCR_SlotCompositionComponent.EOnInit() now
				//SCR_SlotCompositionComponent composition = SCR_SlotCompositionComponent.Cast(baseEntity.FindComponent(SCR_SlotCompositionComponent));
				//if (composition) composition.OrientToTerrain();
			}
		}
		
		//--- Update navmesh (if the reference entity didn't do it already)
		if (updateNavmesh)
		{
			SCR_AIWorld aiWorld = SCR_AIWorld.Cast(GetGame().GetAIWorld());
			if (aiWorld)
				aiWorld.RequestNavmeshRebuildEntity(baseEntity);
		}
		
		Print(string.Format("Entity @\"%1\" spawned in slot '%2'.", resourceName, GetName()), LogLevel.VERBOSE);
		
		return baseEntity;
	}
	/*!
	Get rotation step of the slot.
	\return Rotation step in degrees. 0 means unlimited rotation, -1 disables rotation.
	*/
	float GetRotationStep()
	{
		SCR_SiteSlotEntityClass prefabData = SCR_SiteSlotEntityClass.Cast(GetPrefabData());
		if (!prefabData) return 0;
		
		return prefabData.GetRotationStep();
	}
	/*!
	Get current editable entity occupying the slot.
	Will return null if the slot is occupied by non-editable entity
	\return Editable entity
	*/
	IEntity GetOccupant()
	{
		SCR_CompositionSlotManagerComponent manager = SCR_CompositionSlotManagerComponent.GetInstance();
		if (manager)
		{
			return manager.GetOccupant(this);
		}
		else
		{
			Debug.Error2(Type().ToString(), "Cannot get slot occupant, SCR_CompositionSlotManagerComponent is missing in the game mode entity!");
			return null;
		}
	}
	/*!
	Check if the slot is occupied.
	\return True when occupied
	*/
	bool IsOccupied()
	{
		SCR_CompositionSlotManagerComponent manager = SCR_CompositionSlotManagerComponent.GetInstance();
		if (manager)
		{
			return manager.IsOccupied(this);
		}
		else
		{
			Debug.Error2(Type().ToString(), "Cannot check if a slot is occupied, SCR_CompositionSlotManagerComponent is missing in the game mode entity!");
			return false;
		}
	}
	/*!
	Set the entity occupying the slot.
	When the entity is not editable (e.g., doesn't have SCR_EditableEntityComponent), the occupant will be stored only on server.
	Use null to remove the occupant.
	\param occupant Entity
	*/
	void SetOccupant(IEntity occupant)
	{
		SCR_CompositionSlotManagerComponent manager = SCR_CompositionSlotManagerComponent.GetInstance();
		if (manager)
		{
			manager.SetOccupant(this, occupant);
		}
		else
		{
			Debug.Error2(Type().ToString(), "Cannot set slot occupant, SCR_CompositionSlotManagerComponent is missing in the game mode entity!");
		}
	}
	
	void SCR_SiteSlotEntity(IEntitySource src, IEntity parent)
	{		
		if (SCR_Global.IsEditMode(this)) return;
		
		ClearFlags(EntityFlags.TRACEABLE, false);
		SetFlags(EntityFlags.STATIC, false);
	}
	
#ifdef WORKBENCH
	const string NAME_FORMAT = "%1_%2"; //--- Format of the name, where %1 is the prefab file name and %2 is an iterator
	const int ITERATOR_DIGITS = 3; //--- Number of digits in the iterator (e.g., 3 digits results in 007)
	const int ITERATORS_LIMIT = 1000; //--- How many iterators are changed for a free name
	
	void _WB_SnapToTerrain(IEntitySource entitySource)
	{
		vector pos = GetOrigin();
		pos[1] = GetWorld().GetSurfaceY(pos[0], pos[2]);
		if (entitySource.GetParent())
			pos = SCR_BaseContainerTools.GetLocalCoords(entitySource.GetParent(), pos);
		
		WorldEditorAPI api = _WB_GetEditorAPI();
		IEntitySource src = api.EntityToSource(this);
		api.SetVariableValue(src, null, "coords", pos.ToString(false));
	}
	void _WB_OrientToTerrain(IEntitySource entitySource)
	{
		WorldEditorAPI api = _WB_GetEditorAPI();
		IEntitySource src = api.EntityToSource(this);
		
		vector angles;
		src.Get("angles", angles);
		api.SetVariableValue(src, null, "angles", "0 " + angles[1] + " 0");
		
		vector transform[4];
		GetWorldTransform(transform);
		vector pos = transform[3];
		
		IEntity child;
		vector min, max, posChild;
		
#ifdef SLOT_ENTITY_DEBUG
		m_DebugShapes.Clear();
#endif
		
		for (int i, count = entitySource.GetNumChildren(); i < count; i++)
		{
			IEntitySource childSrc = entitySource.GetChild(i);
			child = api.SourceToEntity(childSrc);
			child.GetBounds(min, max);
			
			float posY = -float.MAX;
			childSrc.Get("angles", angles);
			angles[2] = _WB_GetAngle(api, pos + transform[0] * min[0], pos + transform[0] * max[0], posY);
			angles[0] = -_WB_GetAngle(api, pos + transform[2] * min[2], pos + transform[2] * max[2], posY);
			
			posChild = pos;
			posChild[1] = posY;
			posChild = CoordToLocal(posChild);
			
			api.SetVariableValue(childSrc, null, "coords", posChild.ToString(false));
			api.SetVariableValue(childSrc, null, "angles", string.Format("%1 %2 %3", angles[0], angles[1], angles[2]));
		}
	}
	float _WB_GetAngle(WorldEditorAPI api, vector posA, vector posB, out float posY)
	{
		float dis = vector.Distance(posA, posB);
		if (dis == 0)
		{
			Print(string.Format("Slot entity %1 at position %2 has zero size!", this, GetWorldTransformAxis(3)), LogLevel.WARNING);
			return 0;
		}
		
		posA[1] = api.GetTerrainSurfaceY(posA[0], posA[2]);
		posB[1] = api.GetTerrainSurfaceY(posB[0], posB[2]);
		
		vector posCenter = vector.Lerp(posA, posB, 0.5);
		posY = Math.Max(posY, posCenter[1]);
		
#ifdef SLOT_ENTITY_DEBUG
		m_DebugShapes.Insert(Shape.CreateSphere(Color.RED, ShapeFlags.VISIBLE, posA, 0.5));
		m_DebugShapes.Insert(Shape.CreateSphere(Color.GREEN, ShapeFlags.VISIBLE, posB, 0.5));
		m_DebugShapes.Insert(Shape.CreateSphere(Color.BLUE, ShapeFlags.VISIBLE, posCenter, 0.5));
#endif
		
		return Math.Tan((posB[1] - posA[1]) / dis) * Math.RAD2DEG;
	}

	override array<ref WB_UIMenuItem> _WB_GetContextMenuItems()
	{
		array<ref WB_UIMenuItem> items = { new WB_UIMenuItem("Snap and orient to terrain", 0) };
		
		if (GetName().IsEmpty())
			items.Insert(new WB_UIMenuItem("Auto-assign slot name", 1));
		
		return items;
	}
	override void _WB_OnContextMenu(int id)
	{
		switch (id)
		{
			case 0:
			{
				WorldEditorAPI api = _WB_GetEditorAPI();
				vector mat[4];
				api.BeginEntityAction();
				IEntitySource entitySource = _WB_GetEditorAPI().EntityToSource(this);
				_WB_SnapToTerrain(entitySource);
				_WB_OrientToTerrain(entitySource);
				api.EndEntityAction();
				break;
			}
			case 1:
			{
				WorldEditorAPI api = _WB_GetEditorAPI();
				api.BeginEntityAction();
				api.EndEntityAction();
				break;
			}
		}
	}
#ifdef SLOT_ENTITY_DEBUG
	protected ref array<ref Shape> m_DebugShapes = {};
#endif
	override bool _WB_OnKeyChanged(BaseContainer src, string key, BaseContainerList ownerContainers, IEntity parent)
	{
		if (key == "coords")
		{
			WorldEditorAPI api = _WB_GetEditorAPI();
			if (!api.UndoOrRedoIsRestoring())
			{
				IEntitySource entitySource = src.ToEntitySource();
				_WB_OrientToTerrain(entitySource);
			}
		}
		return false;
	}
#endif
};
