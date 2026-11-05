class SCR_FoliageDestructionSystem : GameSystem
{
	protected static const ResourceName FOLIAGE_DESTRUCTION_HELPER = "{62CD55E7879F4C14}Prefabs\Systems\Destruction\FoliageCutHelper.et";

	ref map<int, IEntity> m_mPlayerIDToHelper = new map<int, IEntity>();

	//------------------------------------------------------------------------------------------------
	override static void InitInfo(WorldSystemInfo outInfo)
	{
		outInfo.SetAbstract(false);
		outInfo.SetLocation(WorldSystemLocation.Server);
	}

	//------------------------------------------------------------------------------------------------
	//! Make a dedicated foliage destruction helper entity for a specific player
	//! \param[in] playerId of the player
	protected IEntity RegisterPlayer(int playerId)
	{
		if (RplSession.Mode() == RplMode.Client)
			return null;

		Resource res = Resource.Load(FOLIAGE_DESTRUCTION_HELPER);
		if (!res.IsValid())
			return null;

		EntitySpawnParams params = new EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		params.Transform[3] = params.Transform[3];

		IEntity foliageHelper = GetGame().SpawnEntityPrefab(res, GetWorld(), params);
		m_mPlayerIDToHelper.Insert(playerId, foliageHelper);

		return foliageHelper;
	}

	//------------------------------------------------------------------------------------------------
	//! Remove the dedicated foliage destruction helper entity for a specific player after they disconnected
	//! \param[in] playerId of the player
	protected void UnregisterPlayer(int playerId)
	{
		if (RplSession.Mode() == RplMode.Client)
			return;

		IEntity helper = m_mPlayerIDToHelper.Get(playerId);
		if (!helper)
			return;

		m_mPlayerIDToHelper.Remove(playerId);
		RplComponent.DeleteRplEntity(helper, false);
	}

	//------------------------------------------------------------------------------------------------
	//! Retrieve foliage destruction helper entity for a specific player
	//! \param[in] playerId of the player
	IEntity GetHelper(int playerId)
	{
		IEntity helper = m_mPlayerIDToHelper.Get(playerId);

		if (!helper)
			helper = RegisterPlayer(playerId);

		return helper;
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnInit()
	{
		SCR_BaseGameMode gamemode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (!gamemode)
			return;

		gamemode.GetOnPlayerRegistered().Insert(RegisterPlayer);
		gamemode.GetOnPlayerDisconnected().Insert(UnregisterPlayer);
	}
}
