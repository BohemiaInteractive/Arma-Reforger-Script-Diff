class SCR_RaceCheckpointClass : DestructibleEntityClass
{
}

class SCR_RaceCheckpoint : DestructibleEntity
{
	protected bool m_bIsActive = true;
	protected int m_iUsedInteractionLayer;
	protected float m_fDistanceFromStart;

	//------------------------------------------------------------------------------------------------
	override bool OnContact(IEntity owner, IEntity other, Contact contact)
	{
		if (!m_bIsActive)
			return true;

		OnCheckpointReached();
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnCheckpointReached()
	{
		SCR_RaceEntity race = SCR_RaceEntity.Cast(GetParent());
		if (!race)
			return;

		SCR_RacerData data = race.GetRacerData(SCR_PlayerController.GetLocalPlayerId());
		if (!data)
			return;

		SCR_HUDManagerComponent hud = GetGame().GetHUDManager();
		if (!hud)
			return;

		SCR_RaceTimeInfo raceinfo = SCR_RaceTimeInfo.Cast(hud.FindInfoDisplay(SCR_RaceTimeInfo));
		if (!raceinfo)
			return;

		data.IncreaseCheckpointsPassed();
		race.CheckpointsUpdated(this);
	}

	//------------------------------------------------------------------------------------------------
	void DisableCheckpoint()
	{
		if (!m_bIsActive)
			return;

		m_bIsActive = false;
		Physics phys = GetPhysics();
		if (!phys)
			return;

		m_iUsedInteractionLayer = phys.GetInteractionLayer();
		SetInteractionLayerDelayed(0);
	}

	//------------------------------------------------------------------------------------------------
	void EnableCheckpoint()
	{
		m_bIsActive = true;
		Physics phys = GetPhysics();
		if (!phys)
			return;

		SetInteractionLayerDelayed(m_iUsedInteractionLayer);
	}
	
	void SetInteractionLayerDelayed(int interactionLayer)
	{
		Physics phys = GetPhysics();
		if (!phys)
			return;

		phys.SetInteractionLayer(interactionLayer);
	}

	//------------------------------------------------------------------------------------------------
	int GetCheckpointID();
	
	//------------------------------------------------------------------------------------------------
	SCR_RaceEntity GetRaceEntity()
	{
		return SCR_RaceEntity.Cast(GetParent());
	}
	
	//------------------------------------------------------------------------------------------------
	void SetDistanceFromStart(float distance)
	{
		m_fDistanceFromStart = distance;
	}

	//------------------------------------------------------------------------------------------------
	float GetDistanceFromStart()
	{
		return m_fDistanceFromStart;
	}

#ifdef WORKBENCH
	//------------------------------------------------------------------------------------------------
	override void _WB_AfterWorldUpdate(float timeSlice)
	{
		DrawDebug();
	}

	//------------------------------------------------------------------------------------------------
	protected void DrawDebug()
	{
		// do not draw debug if child of SCR_RaceEntity AND grandchild of SCR_RaceTrackGeneratorEntity;
		// SCR_RaceTrackGeneratorEntity takes care of its own debug shapes
		WorldEditorAPI worldEditorAPI = _WB_GetEditorAPI();
		if (worldEditorAPI)
		{
			IEntitySource source = worldEditorAPI.EntityToSource(this);
			if (source)
			{
				source = source.GetParent();
				if (source && source.GetClassName().ToType() && source.GetClassName().ToType().IsInherited(SCR_RaceEntity))
				{
					source = source.GetParent();
					if (source && source.GetClassName().ToType() && source.GetClassName().ToType().IsInherited(SCR_RaceTrackGeneratorEntity))
						return;
				}
			}
		}

		int color;
		if (m_bIsActive)
			color = 0x7DFFFFFF;
		else
			color = 0x7DFF0000;

		const float sphereRadius = 4;
		vector origin = GetOrigin();
		Shape.CreateSphere(color, ShapeFlags.ONCE | ShapeFlags.TRANSP | ShapeFlags.WIREFRAME | ShapeFlags.NOCULL, origin, sphereRadius);

		SCR_RaceEntity race = GetRaceEntity();
		if (!race)
			return;

		SCR_RaceCheckpoint nextCheckpoint = race.FindNextCheckpoint(this);
		if (!nextCheckpoint)
			return;

		vector nextOrigin = nextCheckpoint.GetOrigin();
		Shape.CreateArrow(origin + vector.Up * sphereRadius, nextOrigin + vector.Up * sphereRadius, vector.DistanceXZ(origin, nextOrigin) * 0.1, color, ShapeFlags.ONCE | ShapeFlags.TRANSP | ShapeFlags.NOCULL);
	}
#endif // WORKBENCH

	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] src
	//! \param[in] parent
	void SCR_RaceCheckpoint(IEntitySource src, IEntity parent)
	{
		SetEventMask(EntityEvent.INIT | EntityEvent.FRAME);
		Activate();
	}
}
