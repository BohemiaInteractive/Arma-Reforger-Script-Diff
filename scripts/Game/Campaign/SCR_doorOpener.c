[EntityEditorProps(category: "GameScripted/Utility", description: "Opens door in certain radius.")]
class SCR_DoorOpenerEntityClass : GenericEntityClass
{
};

// doorComp.GetOwner() would solve the need for this
class DoorStruct
{
	IEntity owner;
	DoorComponent component;
	
	void DoorStruct(IEntity e, DoorComponent comp)
	{
		this.owner = e;
		this.component = comp;
	}
};

class SCR_DoorOpenerEntity : GenericEntity
{
	[Attribute(defvalue: "10", uiwidget: UIWidgets.Slider, desc: "Radius in which to open door.", "0 1000 1")]
	protected float m_fRadius;
	
	[Attribute(defvalue: "1", uiwidget: UIWidgets.Slider, desc: "How much the doors should be opened", "0. 1 0.01")]
	protected float m_fControlValue;

	
	private ref array<ref DoorStruct> m_aQueriedDoors;
	
	#ifdef WORKBENCH	
	private bool m_bVisualize;
	#endif
	
	//------------------------------------------------------------------------------------------------
	private bool QueryEntities(IEntity e)
	{
		DoorComponent door = DoorComponent.Cast(e.FindComponent(DoorComponent));
		if (door)
			m_aQueriedDoors.Insert(new DoorStruct(e, door));
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	private void GetDoors(float radius)
	{
		BaseWorld world = GetWorld();
		world.QueryEntitiesBySphere(GetOrigin(), radius, QueryEntities);
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		RplId instigatorId;
		
		// server only
		RplComponent rplComponent = RplComponent.Cast(owner.FindComponent(RplComponent));
		if (rplComponent)
		{
			 if (!rplComponent.IsMaster())
				return;
			
			instigatorId = rplComponent.Id();
		}
		
		// capture doors around
		m_aQueriedDoors = {};
		GetDoors(m_fRadius);
		// set state
		foreach (DoorStruct door : m_aQueriedDoors)
			door.component.SetControlValue(m_fControlValue, instigatorId);
		
		// cleanup
		m_aQueriedDoors.Clear();
		m_aQueriedDoors = null;
		
		// destroy self
		delete owner;
	}

	//------------------------------------------------------------------------------------------------
	void SCR_DoorOpenerEntity(IEntitySource src, IEntity parent)
	{
		if (SCR_Global.IsEditMode(this))
			return;
		
		SetEventMask(EntityEvent.INIT);
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_DoorOpenerEntity()
	{
	}
	
	#ifdef WORKBENCH	
		
	private void _CaptureDoors()
	{
		m_aQueriedDoors = {};
		GetDoors(m_fRadius);
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
			
			foreach (DoorStruct door : m_aQueriedDoors)
			{
				auto arrowShape = Shape.CreateArrow(origin, door.owner.GetOrigin(), 0.1, COLOR_GREEN, ShapeFlags.ONCE);
			}	
		}
		
		super._WB_AfterWorldUpdate(timeSlice);
	}
	
	#endif

};
