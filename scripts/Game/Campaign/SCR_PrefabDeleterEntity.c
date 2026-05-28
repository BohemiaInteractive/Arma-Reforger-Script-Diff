[EntityEditorProps(category: "GameScripted/Utility", description: "Delete entities in a certain radius.")]
class SCR_PrefabDeleterEntityClass : GenericEntityClass
{
}

//! - Deletes entities in the defined area
//! - Can avoid deleting players
//! - Can delete only visible entities (avoids deleting logics)
class SCR_PrefabDeleterEntity : GenericEntity
{
	/*
		Deletion
	*/

	[Attribute(defvalue: "1", desc: "Delete player entities", category: "Deletion")]
	protected bool m_bDeletePlayers;

	[Attribute(defvalue: "0", desc: "Only delete entities with a VObjectComponent, e.g a model/particle/etc; otherwise delete everything", category: "Deletion")]
	protected bool m_bOnlyDeleteVisibleEntities;

	[Attribute(defvalue: "10", uiwidget: UIWidgets.Slider, desc: "Radius in which entities are deleted", params: "0 1000 1", category: "Deletion")]
	protected float m_fRadius;

	[Attribute(defvalue: "0", uiwidget: UIWidgets.Slider, desc: "[s] Delay before deletion", params: "0 10", precision: 1, category: "Deletion")]
	protected float m_fDelay;

	protected static ref array<IEntity> s_aFoundEntities = {};

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		// authority only
		RplComponent rplComponent = RplComponent.Cast(owner.FindComponent(RplComponent));
		if (rplComponent && !rplComponent.IsMaster())
			return;

		// CallLater: delete after first frame to not break init
		GetGame().GetCallqueue().CallLater(PerformDeletion, m_fDelay * 1000, param1: owner);
	}

	//------------------------------------------------------------------------------------------------
	protected void PerformDeletion(IEntity owner)
	{
		EQueryEntitiesFlags flags = EQueryEntitiesFlags.ALL;
		if (m_bOnlyDeleteVisibleEntities)
			flags |= EQueryEntitiesFlags.WITH_OBJECT;

		GetWorld().QueryEntitiesBySphere(owner.GetOrigin(), m_fRadius, PerformDeletion_QueryEntitiesCallback, null, flags);

		foreach (IEntity entity : s_aFoundEntities)
		{
			if (!entity) // if one saw its parent already deleted
				continue;

			if (entity.Type() && entity.Type().IsInherited(SCR_PrefabDeleterEntity)) // avoid self and other deleters
				continue;

			if (vector.Distance(GetOrigin(), entity.GetOrigin()) > m_fRadius) // sphere query works with bboxes; here we get more accurate by origin
				continue;

			IEntity root = entity.GetRootParent();

			if (!m_bDeletePlayers && EntityUtils.IsPlayer(root))
				continue;

			RplComponent.DeleteRplEntity(root, false);
		}

		s_aFoundEntities.Clear();

		RplComponent.DeleteRplEntity(owner, false); // destroy self
	}

	//------------------------------------------------------------------------------------------------
	protected bool PerformDeletion_QueryEntitiesCallback(IEntity e)
	{
		s_aFoundEntities.Insert(e);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] src
	//! \param[in] parent
	void SCR_PrefabDeleterEntity(IEntitySource src, IEntity parent)
	{
		if (m_fRadius <= 0 || !GetGame().InPlayMode())
			return;

		SetEventMask(EntityEvent.INIT);
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
		if (m_fRadius > 0)
			Shape.CreateSphere(0x44FFCC00, ShapeFlags.WIREFRAME | ShapeFlags.ONCE | ShapeFlags.TRANSP, GetOrigin(), m_fRadius);
	}

#endif

}
