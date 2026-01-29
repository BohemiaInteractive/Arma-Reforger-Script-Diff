//------------------------------------------------------------------------------------------------
class SCR_PossessSpawnData : SCR_SpawnData
{
	protected RplId m_RplId;
	protected bool m_bSkipPreload;

	/*!
		Returns RplId of entity to posses.
	*/
	RplId GetRplId()
	{
		return m_RplId;
	}

	/*!
		Create data from provided RplId of possessable entity.
	*/
	static SCR_PossessSpawnData FromRplId(RplId id)
	{
		SCR_PossessSpawnData data = new SCR_PossessSpawnData();
		data.m_RplId = id;
		return data;
	}

	/*!
		Create data from provided entity.
			Note: Not recommended for non-authority use as entities can stream in and out.
	*/
	static SCR_PossessSpawnData FromEntity(notnull IEntity entity)
	{
		RplId id = RplId.Invalid();
		RplComponent rplComponent = RplComponent.Cast(entity.FindComponent(RplComponent));
		if (rplComponent)
			id = rplComponent.Id();

		return FromRplId(id);
	}

	protected void SCR_PossessSpawnData()
	{
	}

	/*!
		Are data valid in the most basic context?
	*/
	override bool IsValid()
	{
		return m_RplId.IsValid();
	}
	
	override void SetSkipPreload(bool skip)
	{
		m_bSkipPreload = skip;
	}
	
	override bool GetSkipPreload()
	{
		return m_bSkipPreload;
	}
};
