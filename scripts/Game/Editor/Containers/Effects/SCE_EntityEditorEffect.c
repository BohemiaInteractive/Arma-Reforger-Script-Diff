[BaseContainerProps(), SCR_BaseContainerCustomTitleResourceName("m_Prefab", true, "ENT: \"%1\"")]
/** @ingroup Editor_Effects
*/
/*!
Spawned entity effect.
*/
class SCR_EntityEditorEffect: SCR_BaseEditorEffect
{
	//---- REFACTOR NOTE START: Should be protected ----
	[Attribute(params: "et", uiwidget: UIWidgets.ResourcePickerThumbnail, desc: "Spawned entity")]
	private ResourceName m_Prefab;
	//---- REFACTOR NOTE END ----
	
	[Attribute(desc: "Spawn the entity only when no entities are passed to the event.")]
	protected bool m_bNoEntities;
	
	[Attribute(desc: "True to delete previous entity created by this effect when a new one is being created.")]
	protected bool m_bReplacePrevious;
	
	protected IEntity m_Entity;
	
	//---- REFACTOR NOTE START: Not up to date with internal scripting style ----
	/*!
	Get entity created by the effect
	\return Entity
	*/
	IEntity GetEntity()
	{
		return m_Entity;
	}
	
	override bool EOnActivate(SCR_BaseEditorComponent editorComponent, vector position = vector.Zero, set<SCR_EditableEntityComponent> entities = null)
	{
		if (position == vector.Zero) return false;

		if (m_bNoEntities && entities && !entities.IsEmpty()) return false;
		
		Resource resource = Resource.Load(m_Prefab);
		if (!resource || !resource.IsValid())
		{
			Print(string.Format("Invalid prefab '%1'!", m_Prefab), LogLevel.WARNING);
			return false;
		}
		
		IEntitySource entitySource = SCR_BaseContainerTools.FindEntitySource(resource);
		if (!entitySource)
		{
			Print(string.Format("Prefab '%1' is not an entity!", m_Prefab), LogLevel.WARNING);
			return false;
		}
		
		EntitySpawnParams spawnParams = new EntitySpawnParams;
		spawnParams.Transform[3] = position;
		
		//--- Apply scale from prefab
		float scale;
		entitySource.Get("scale", scale);
		if (scale != 1)
		{
			spawnParams.Transform[0] = spawnParams.Transform[0] * scale;
			spawnParams.Transform[1] = spawnParams.Transform[1] * scale;
			spawnParams.Transform[2] = spawnParams.Transform[2] * scale;
		}
		
		if (m_bReplacePrevious)
			SCR_EntityHelper.DeleteEntityAndChildren(m_Entity);		
		m_Entity = GetGame().SpawnEntityPrefab(resource, GetGame().GetWorld(), spawnParams);
		return true;
	}
	//---- REFACTOR NOTE END ----
};