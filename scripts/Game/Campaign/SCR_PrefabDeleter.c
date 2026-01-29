[EntityEditorProps(category: "GameScripted/ScriptWizard", description: "Deletes entity in certain radius.")]
class SCR_PrefabDeleterEntityClass : GenericEntityClass
{
}

//------------------------------------------------------------------------------------------------
class SCR_PrefabDeleterEntity : GenericEntity
{
	[Attribute(defvalue: "10", uiwidget: UIWidgets.Slider, desc: "Radius in which entities are deleted", "0 1000 1")]
	protected float m_fRadius;

	//------------------------------------------------------------------------------------------------
	private bool QueryEntities(IEntity e)
	{
		SCR_EntityHelper.DeleteEntityAndChildren(e);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		// server only
		RplComponent rplComponent = RplComponent.Cast(owner.FindComponent(RplComponent));
		if (rplComponent && !rplComponent.IsMaster())
			return;

		GetGame().GetCallqueue().CallLater(PerformDeletion, 0, false, owner);
	}
	
	//------------------------------------------------------------------------------------------------
	void PerformDeletion(IEntity owner)
	{
		BaseWorld world = GetWorld();
		world.QueryEntitiesBySphere(owner.GetOrigin(), m_fRadius, QueryEntities);
			
		// destroy self
		delete owner;
	}

	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] src
	//! \param[in] parent
	void SCR_PrefabDeleterEntity(IEntitySource src, IEntity parent)
	{
		if (!GetGame().InPlayMode())
			return;
		
		SetEventMask(EntityEvent.INIT);
	}

#ifdef WORKBENCH	
	
	//------------------------------------------------------------------------------------------------
	override int _WB_GetAfterWorldUpdateSpecs(IEntitySource src)
	{
		return EEntityFrameUpdateSpecs.CALL_WHEN_ENTITY_VISIBLE;
	}

	//------------------------------------------------------------------------------------------------
	override void _WB_AfterWorldUpdate(float timeSlice)
	{

		auto origin = GetOrigin();
		auto radiusShape = Shape.CreateSphere(COLOR_YELLOW, ShapeFlags.WIREFRAME | ShapeFlags.ONCE, origin, m_fRadius);

		super._WB_AfterWorldUpdate(timeSlice);
	}
	
#endif

}
