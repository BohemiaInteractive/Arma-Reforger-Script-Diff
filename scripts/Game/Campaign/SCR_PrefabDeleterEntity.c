[EntityEditorProps(category: "GameScripted/Utility", description: "Delete entities in a certain radius.")]
class SCR_PrefabDeleterEntityClass : GenericEntityClass
{
}

//! - Deletes entities in the defined area
//! - Option to only delete visible entities (avoids deleting logics)
class SCR_PrefabDeleterEntity : GenericEntity
{
	/*
		Deletion
	*/

	[Attribute(defvalue: "10", uiwidget: UIWidgets.Slider, desc: "Radius (included) in which entities are deleted", params: "0.001 1000 1", category: "Deletion")]
	protected float m_fRadius;

	[Attribute(defvalue: "1", desc: "Delete root parent entities of detected entities, e.g delete the whole composition if an item is spotted; otherwise only delete entities in range", category: "Deletion")]
	protected bool m_bDeleteRootEntities;

	[Attribute(defvalue: "0", desc: "Only delete entities with a VObjectComponent, e.g a model/particle/etc; otherwise delete everything", category: "Deletion")]
	protected bool m_bOnlyDeleteVisibleEntities;

	[Attribute(defvalue: "1", desc: "Listed Prefabs in Prefabs List are protected, otherwise they are the only ones to be deleted", category: "Deletion")]
	protected bool m_bUsePrefabsListAsProtectedList;

	[Attribute(uiwidget: UIWidgets.ResourcePickerThumbnail, desc: "Prefabs list to avoid/delete (depending on the Use Prefabs List As Protected List checkbox)\nConsiders inherited Prefabs too", params: "et", category: "Deletion")]
	protected ref array<ResourceName> m_aPrefabsList;

	protected ref array<IEntity> m_aFoundEntities = {};

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		// loadtime only
		if (!Replication.Loadtime())
		{
			Print("A " + Type() + " instance was created outside of loadtime and will be ignored/deleted", LogLevel.WARNING);
			RplComponent.DeleteRplEntity(owner, false); // destroy self
			return;
		}

		// root only
		if (GetParent())
		{
			Print("A " + Type() + " instance was created in the hierarchy of another entity and will be ignored/deleted", LogLevel.WARNING);
			RplComponent.DeleteRplEntity(owner, false); // destroy self
			return;
		}
	}

	//------------------------------------------------------------------------------------------------
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		if (Replication.Runtime())
		{
			PerformDeletion(owner); // delete first runtime frame
			ClearEventMask(EntityEvent.FRAME);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void PerformDeletion(IEntity owner)
	{
		EQueryEntitiesFlags flags = EQueryEntitiesFlags.ALL;
		if (m_bOnlyDeleteVisibleEntities)
			flags |= EQueryEntitiesFlags.WITH_OBJECT;

		vector origin = owner.GetOrigin();
		GetWorld().QueryEntitiesBySphere(origin, m_fRadius, PerformDeletion_QueryEntitiesCallback, null, flags);

		bool useList = !m_aPrefabsList.IsEmpty();

		EntityPrefabData prefabData;
		BaseContainer baseContainer;

		float radiusSq = m_fRadius * m_fRadius;
		foreach (IEntity entity : m_aFoundEntities)
		{
			if (!entity) // if one saw its parent already deleted
				continue;

			if (vector.DistanceSq(origin, entity.GetOrigin()) > radiusSq) // sphere query works with bboxes; here we get more accurate by origin
				continue;

			if (m_bDeleteRootEntities)
				entity = entity.GetRootParent();

			if (entity.Type().IsInherited(SCR_PrefabDeleterEntity)) // avoid self and other deleters
				continue;

			if (useList)
			{
				prefabData = entity.GetPrefabData();
				if (prefabData)
				{
					baseContainer = prefabData.GetPrefab();
					if (baseContainer)
					{
						bool found;

						BaseContainer ancestor = baseContainer;
						while (ancestor)
						{
							if (m_aPrefabsList.Contains(ancestor.GetResourceName()))
							{
								found = true;
								break;
							}

							ancestor = ancestor.GetAncestor();
						}

						if (found == m_bUsePrefabsListAsProtectedList)
							continue; // protec
					}
				}
			}

			// default behaviour
			SCR_EntityHelper.DeleteBuilding(entity);
		}

		RplComponent.DeleteRplEntity(owner, false); // destroy self
	}

	//------------------------------------------------------------------------------------------------
	protected bool PerformDeletion_QueryEntitiesCallback(IEntity e)
	{
		m_aFoundEntities.Insert(e);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] src
	//! \param[in] parent
	void SCR_PrefabDeleterEntity(IEntitySource src, IEntity parent)
	{
		if (!GetGame().InPlayMode())
			return;

		SetEventMask(EntityEvent.INIT | EntityEvent.FRAME);
	}

#ifdef WORKBENCH

	//------------------------------------------------------------------------------------------------
	override int _WB_GetAfterWorldUpdateSpecs(IEntitySource src)
	{
		return EEntityFrameUpdateSpecs.CALL_WHEN_ENTITY_VISIBLE | EEntityFrameUpdateSpecs.CALL_WHEN_ENTITY_SELECTED;
	}

	//------------------------------------------------------------------------------------------------
	override void _WB_AfterWorldUpdate(float timeSlice)
	{
		super._WB_AfterWorldUpdate(timeSlice);

		Shape.CreateSphere(0x44FFCC00, ShapeFlags.WIREFRAME | ShapeFlags.ONCE | ShapeFlags.TRANSP, GetOrigin(), m_fRadius);
	}

#endif

}
