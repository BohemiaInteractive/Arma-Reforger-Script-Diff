class SCR_SpawnPositionComponentClass : ScriptComponentClass
{
	[Attribute(defvalue: "-0.4 0 -0.3", category: "Position Boundaries")]
	protected vector m_vMinBounds;

	[Attribute(defvalue: "0.4 1.8 0.4", category: "Position Boundaries")]
	protected vector m_vMaxBounds;

	//------------------------------------------------------------------------------------------------
	//! \return
	vector GetMinBoundsVector()
	{
		return m_vMinBounds;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	vector GetMaxBoundsVector()
	{
		return m_vMaxBounds;
	}
}

class SCR_SpawnPositionComponent : ScriptComponent
{
#ifdef WORKBENCH
	protected ref Shape m_DebugShape;
#endif

	//------------------------------------------------------------------------------------------------
	//! Returns true, if there is no living character inside of defined boundaries.
	bool IsFree()
	{
		SCR_SpawnPositionComponentClass prefabData = SCR_SpawnPositionComponentClass.Cast(GetComponentData(GetOwner()));
		if (!prefabData)
			return false;

		vector transform[4];
		GetOwner().GetTransform(transform);

		return GetGame().GetWorld().QueryEntitiesByOBB(prefabData.GetMinBoundsVector(), prefabData.GetMaxBoundsVector(), transform, CheckEntity, FilterEntity, EQueryEntitiesFlags.ALL | EQueryEntitiesFlags.WITH_OBJECT);
	}

	//------------------------------------------------------------------------------------------------
	protected bool CheckEntity(IEntity ent)
	{
		DamageManagerComponent damageManager = DamageManagerComponent.Cast(ent.FindComponent(DamageManagerComponent));
		if (!damageManager || !damageManager.IsDestroyed())
			return false;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected bool FilterEntity(IEntity ent)
	{
		return ent.IsInherited(ChimeraCharacter);
	}

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		SCR_SpawnPositionComponentManager manager = SCR_SpawnPositionComponentManager.GetInstance();
		if (manager)
			manager.AddSpawnPosition(this);
	}

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		SetEventMask(owner, EntityEvent.INIT);
	}

#ifdef WORKBENCH
	//------------------------------------------------------------------------------------------------
	protected void DrawDebugShape()
	{
		SCR_SpawnPositionComponentClass prefabData = SCR_SpawnPositionComponentClass.Cast(GetComponentData(GetOwner()));
		if (!prefabData)
			return;

		EntitySpawnParams params = new EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;

		IEntity owner = GetOwner();
		owner.GetTransform(params.Transform);

		int shapeFlags = ShapeFlags.WIREFRAME;
		m_DebugShape = Shape.Create(ShapeType.BBOX, Color.CYAN, shapeFlags, prefabData.GetMinBoundsVector(), prefabData.GetMaxBoundsVector());

		m_DebugShape.SetMatrix(params.Transform);
	}

	//------------------------------------------------------------------------------------------------
	override int _WB_GetAfterWorldUpdateSpecs(IEntity owner, IEntitySource src)
	{
		return EEntityFrameUpdateSpecs.CALL_WHEN_ENTITY_VISIBLE;
	}

	//------------------------------------------------------------------------------------------------
	override event void _WB_AfterWorldUpdate(IEntity owner, float timeSlice)
	{
		if (GetGame().InPlayMode())
			return;

		if (m_DebugShape)
		{
			vector transform[4];
			owner.GetTransform(transform);
			m_DebugShape.SetMatrix(transform);
		}
		else
		{
			DrawDebugShape();
		}
	}
#endif

	//------------------------------------------------------------------------------------------------
	// destructor
	void ~SCR_SpawnPositionComponent()
	{
		SCR_SpawnPositionComponentManager manager = SCR_SpawnPositionComponentManager.GetInstance(false);
		if (manager)
			manager.DeleteSpawnPosition(this);
	}
}
