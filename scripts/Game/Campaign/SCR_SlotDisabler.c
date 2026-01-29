[EntityEditorProps(category: "GameScripted/Utility", description: "Disables slots in certain radius.")]
class SCR_SlotDisablerEntityClass : GenericEntityClass
{
};

class SCR_SlotDisablerEntity : GenericEntity
{
	[Attribute(defvalue: "10", uiwidget: UIWidgets.Slider, desc: "Radius in which to disable slots.", "0 1000 1")]
	protected float m_fRadius;
	
	private ref array<SCR_SiteSlotEntity> m_aQueriedSlots;
	
	#ifdef WORKBENCH	
	private bool m_bVisualize;
	#endif
	
	//------------------------------------------------------------------------------------------------
	private bool QueryEntities(IEntity e)
	{
		SCR_SiteSlotEntity slot = SCR_SiteSlotEntity.Cast(e);
		
		if (slot)
			m_aQueriedSlots.Insert(slot);
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	private void GetSlots(float radius)
	{
		BaseWorld world = GetWorld();
		world.QueryEntitiesBySphere(GetOrigin(), radius, QueryEntities);
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		// capture slots around
		m_aQueriedSlots = {};
		GetSlots(m_fRadius);
		
		// disable slots
		foreach (SCR_SiteSlotEntity slot : m_aQueriedSlots)
		{
			SCR_EditableEntityComponent comp = SCR_EditableEntityComponent.Cast(slot.FindComponent(SCR_EditableEntityComponent));
		
			if (comp)
				comp.SetVisible(false);
		}
		
		// cleanup
		m_aQueriedSlots.Clear();
		m_aQueriedSlots = null;
		
		// destroy self
		delete owner;
	}

	//------------------------------------------------------------------------------------------------
	void SCR_SlotDisablerEntity(IEntitySource src, IEntity parent)
	{
		if (SCR_Global.IsEditMode(this))
			return;
		
		SetEventMask(EntityEvent.INIT);
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_SlotDisablerEntity()
	{
	}
	
	#ifdef WORKBENCH	
		
	private void _CaptureDoors()
	{
		m_aQueriedSlots = {};
		GetSlots(m_fRadius);
	}
	
	override bool _WB_OnKeyChanged(BaseContainer src, string key, BaseContainerList ownerContainers, IEntity parent) 
	{
		_CaptureDoors();
		return super._WB_OnKeyChanged(src, key, ownerContainers, parent);
	}
	
	override void _WB_SetExtraVisualiser(EntityVisualizerType type, IEntitySource src)
	{		
		m_bVisualize = false;
		switch (type)
		{
			case EntityVisualizerType.EVT_NONE:
				return;
			
			case EntityVisualizerType.EVT_NORMAL:
				return;
		}
		
		m_bVisualize = true;		
		_CaptureDoors();
		super._WB_SetExtraVisualiser(type, src);
	}
	
	override int _WB_GetAfterWorldUpdateSpecs(IEntitySource src)
	{
		return EEntityFrameUpdateSpecs.CALL_WHEN_ENTITY_VISIBLE;
	}
	
	override void _WB_AfterWorldUpdate(float timeSlice)
	{
		if (m_bVisualize)
		{
			auto origin = GetOrigin();
			auto radiusShape = Shape.CreateSphere(COLOR_YELLOW, ShapeFlags.WIREFRAME | ShapeFlags.ONCE, origin, m_fRadius);
			
			foreach (SCR_SiteSlotEntity slot : m_aQueriedSlots)
			{
				auto arrowShape = Shape.CreateArrow(origin, slot.GetOrigin(), 0.1, COLOR_GREEN, ShapeFlags.ONCE);
			}	
		}
		
		super._WB_AfterWorldUpdate(timeSlice);
	}
	
	#endif

};
