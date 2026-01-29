[EntityEditorProps(category: "GameScripted/Gamemodes", description: "Combat Area 3")]
class SCR_SectorSpawnClass: GenericEntityClass
{

};

class SCR_SectorSpawn: GenericEntity
{
	[Attribute("10 5 10", UIWidgets.EditBox, "Size of box")]
	vector 	m_size; 	
	[Attribute("1", UIWidgets.CheckBox, "Show prefab preview?")]
	bool m_showPreview; 	
	
	void SCR_SectorSpawn(IEntitySource src, IEntity parent)
	{
		SetFlags(EntityFlags.ACTIVE);
		SetEventMask(EntityEvent.INIT | EntityEvent.TOUCH | EntityEvent.CONTACT);					
	}
	
	void ~SCR_SectorSpawn()
	{
		// cleanup
	}
	
	override private void EOnInit(IEntity owner)
	{
		//Print("SectorSpawn initialized.");
		autoptr PhysicsGeomDef geoms[] = {PhysicsGeomDef("", PhysicsGeom.CreateBox(m_size), "material/default", EPhysicsLayerDefs.Vehicle | EPhysicsLayerDefs.Character)};
		Physics.CreateGhostEx(this, geoms);
		//SpawnChildren("{8F21EDDBADC889FE}entities/Survival/Apple.et");
	}
	
	
	
#ifdef WORKBENCH	
	//-----------------------------------------------------------------------------------------------------------
	override int _WB_GetAfterWorldUpdateSpecs(IEntitySource src)
	{
		return EEntityFrameUpdateSpecs.CALL_WHEN_ENTITY_VISIBLE;
	}
	
	//-----------------------------------------------------------------------------------------------------------
	override void _WB_AfterWorldUpdate(float timeSlice)
	{
		Shape dbgShape = null;
		int color = ARGB(64, 0x33, 0x66, 0x99);
		vector m_start = {m_size[0] * -0.5, m_size[1] * -0.5, m_size[2] * -0.5};
		vector m_end = {m_size[0] * 0.5, m_size[1]  * 0.5, m_size[2] * 0.5}; 
		
		dbgShape = Shape.Create(ShapeType.BBOX, color, ShapeFlags.TRANSP | ShapeFlags.DOUBLESIDE | ShapeFlags.NOZWRITE | ShapeFlags.ONCE | ShapeFlags.NOOUTLINE, m_start, m_end);
		
		vector mat[4];
		GetWorldTransform(mat);
		dbgShape.SetMatrix(mat);
	}
	override bool _WB_OnKeyChanged(BaseContainer src, string key, BaseContainerList ownerContainers, IEntity parent)
	{
		// event when changing shape or position of the trigger in WB
		bool m_showPreviewLocal;
		src.Get("m_showPreview",m_showPreviewLocal);
		if (m_showPreviewLocal)
		{
			SpawnPreview();	
		}
		else
		{	
			DespawnChildren();
		}
		return false;	
	}	
	
#endif
	
	
	
	override private void EOnTouch(IEntity owner, IEntity other, int touchTypesMask)
	{
		PrintFormat("OnTouch triggered by entity %1 of type %2", other,touchTypesMask);
	
	}
	
	override private void EOnContact(IEntity owner, IEntity other, Contact contact)
	{
		PrintFormat("OnContact triggered by entity %1 of type %2", other, contact);
	}
	
	void SpawnPreview()
	{
		IEntity child = this.GetChildren();
		vector camMatrix[4];	
		
#ifdef WORKBENCH 
		WorldEditorAPI m_API = _WB_GetEditorAPI();
		if (!child)
		{
			Print("No children of sector spawn found!");
		}
		while (child) 
		{
			SCR_SectorPrefabSpawnPoint prefabSpawnPoint = SCR_SectorPrefabSpawnPoint.Cast(child);
			vector mat[4];
			prefabSpawnPoint.GetWorldTransform(mat);
			
			if (prefabSpawnPoint.ShouldSnapToGround())
			{
				vector position = mat[3];
				position[1] = prefabSpawnPoint.GetWorld().GetSurfaceY(position[0], position[2]);
				mat[3] = position;
			}
			EntitySpawnParams spawnParams = new EntitySpawnParams;
			spawnParams.TransformMode = ETransformMode.WORLD;
			spawnParams.Transform = mat;
			Resource resource = Resource.Load(prefabSpawnPoint.m_sLoadoutResource);
			
			//IEntity entity = m_API.CreateEntity(prefab, "moje " + index, m_API.GetCurrentEntityLayerId(), m_API.EntityToSource(prefabSpawnPoint),vector.Zero, vector.Zero);
			
			IEntity entity = GetGame().SpawnEntityPrefab(resource, m_API.GetWorld(), spawnParams);
			
				if (!entity)
				{
					Print("Something is awfully wrong somewhere");
				}
				else
				{
					prefabSpawnPoint.AddChild(entity,0, EAddChildFlags.AUTO_TRANSFORM);
					prefabSpawnPoint.GetWorldTransform(mat);
					entity.SetWorldTransform(mat);
				}				
			child = child.GetSibling();					
		}	
#else
		Print("Running game mode");		
			
				//IEntity entity = GetGame().SpawnEntityPrefab(resource, null, spawnParams);		
#endif			
	}	
	
	void DespawnChildren()
	{
		IEntity spawnPoint = this.GetChildren();
		while (spawnPoint) 
		{
			IEntity ent = spawnPoint.GetChildren();
			if (ent)
			{
				spawnPoint.RemoveChild(ent);
				delete ent;				
			}
			spawnPoint = spawnPoint.GetSibling();						
		}	
	}
};
