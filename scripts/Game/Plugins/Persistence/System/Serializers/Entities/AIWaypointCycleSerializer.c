class SCR_CyclewWaypointLoadContextShared
{
	int m_iPendingResults;
	ref array<AIWaypoint> m_aWaypoints;
	AIWaypointCycle m_CycleWaypoint;
}

class SCR_CyclewWaypointLoadContext
{
	ref SCR_CyclewWaypointLoadContextShared m_Shared;
	int m_iIdx;
}

class AIWaypointCycleSerializer : GenericEntitySerializer
{
	//------------------------------------------------------------------------------------------------
	override static typename GetTargetType()
	{
		return AIWaypointCycle;
	}

	//------------------------------------------------------------------------------------------------
	override protected ESerializeResult Serialize(notnull IEntity entity, notnull SaveContext context)
	{
		const AIWaypointCycle cycleWaypoint = AIWaypointCycle.Cast(entity);
		BaseContainer source;
		if (cycleWaypoint.GetPrefabData())
			source = cycleWaypoint.GetPrefabData().GetPrefab();

		const int rerunCounter = cycleWaypoint.GetRerunCounter();
		int rerunCounterDefault = -1;
		if (source)
			source.Get("RerunCounter", rerunCounterDefault);

		array<UUID> waypoints();
		array<AIWaypoint> outWaypoints();
		cycleWaypoint.GetWaypoints(outWaypoints);
		foreach (auto waypoint : outWaypoints)
		{
			const UUID uuid = GetSystem().GetId(waypoint);
			if (!uuid.IsNull())
				waypoints.Insert(uuid);
		}

		context.StartObject("base");
		const ESerializeResult baseResult = super.Serialize(entity, context);
		context.EndObject();
		if (baseResult == ESerializeResult.ERROR)
			return baseResult;

		if (baseResult == ESerializeResult.DEFAULT &&
			rerunCounter == rerunCounterDefault &&
			waypoints.IsEmpty())
		{
			return baseResult;
		}

		context.WriteValue("version", 1);
		context.WriteDefault(rerunCounter, rerunCounterDefault);

		if (!waypoints.IsEmpty() || !context.CanSeekMembers())
			context.Write(waypoints);

		return ESerializeResult.OK;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool Deserialize(notnull IEntity entity, notnull LoadContext context)
	{
		auto cycleWaypoint = AIWaypointCycle.Cast(entity);

		if (context.DoesObjectExist("base"))
		{
			if (!context.StartObject("base") ||
				!super.Deserialize(entity, context) ||
				!context.EndObject())
			{
				return false;
			}
		}

		int version;
		context.Read(version);

		int rerunCounter;
		if (context.Read(rerunCounter))
			cycleWaypoint.SetRerunCounter(rerunCounter);

		array<UUID> waypoints();
		if (context.Read(waypoints))
			cycleWaypoint.SetWaypoints({});

		if (!waypoints.IsEmpty())
		{
			SCR_CyclewWaypointLoadContextShared sharedContext();
			sharedContext.m_iPendingResults = waypoints.Count();
			sharedContext.m_aWaypoints = {};
			sharedContext.m_aWaypoints.Resize(sharedContext.m_iPendingResults);
			sharedContext.m_CycleWaypoint = cycleWaypoint;

			foreach (int idx, auto waypoint : waypoints)
			{
				SCR_CyclewWaypointLoadContext waypointContext();
				waypointContext.m_Shared = sharedContext;
				waypointContext.m_iIdx = idx;
				PersistenceWhenAvailableTask waypointTask(OnWaypointAvailable, waypointContext);
				GetSystem().WhenAvailable(waypoint, waypointTask);
			}
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected static void OnWaypointAvailable(Managed instance, PersistenceDeferredDeserializeTask task, bool expired, Managed context)
	{
		auto waypointContext = SCR_CyclewWaypointLoadContext.Cast(context);
		waypointContext.m_Shared.m_aWaypoints[waypointContext.m_iIdx] = AIWaypoint.Cast(instance);
		if (--waypointContext.m_Shared.m_iPendingResults == 0 && waypointContext.m_Shared.m_CycleWaypoint)
		{
			waypointContext.m_Shared.m_aWaypoints.RemoveItem(null);
			waypointContext.m_Shared.m_CycleWaypoint.SetWaypoints(waypointContext.m_Shared.m_aWaypoints);
		}
	}
}
