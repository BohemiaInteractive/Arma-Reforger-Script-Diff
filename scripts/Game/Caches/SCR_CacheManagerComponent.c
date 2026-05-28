[ComponentEditorProps(category: "GameScripted/GameMode/Components", description: "Manager that spawns caches and provides hints")]
class SCR_CacheManagerComponentClass : SCR_BaseGameModeComponentClass {}

class SCR_CacheManagerComponent : SCR_BaseGameModeComponent
{
	[Attribute(uiwidget: UIWidgets.ResourcePickerThumbnail, category: "Cache Config", desc: "Prefab to spawn at cache locations (e.g. AmmoBox)", params: "et")]
	protected ResourceName m_sCachePrefab;

	[Attribute("3", params: "0 inf", uiwidget: UIWidgets.EditBox, category: "Cache Config", desc: "Amount of caches to spawn")]
	int m_iAmount;

	[Attribute(category: "Cache Config", desc: "Cache coordinates config", params: "conf")]
	protected ResourceName m_sTerrainConfig;

	protected ref array<ref SCR_CacheLocationData> m_aCachePool = {};
	protected ref SCR_CacheTerrainConfig m_TerrainConfig;
	protected static SCR_CacheManagerComponent s_Instance;
	protected ref array<string> m_aActiveCacheHints = {};

	//------------------------------------------------------------------------------------------------
	//!
	//! \return
	static SCR_CacheManagerComponent GetInstance()
	{
		return s_Instance;
	}

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		s_Instance = this;
	}

	//------------------------------------------------------------------------------------------------
	override void OnGameModeStart()
	{
		if (!Replication.IsServer())
			return;
		
		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
		if (!campaign || !campaign.GetSpawnRandomCaches())
			return;
		
		m_TerrainConfig = SCR_CacheTerrainConfig.Cast(SCR_BaseContainerTools.CreateInstanceFromPrefab(m_sTerrainConfig, true));
		if (m_TerrainConfig && m_TerrainConfig.m_aCachePool)
			m_aCachePool = m_TerrainConfig.m_aCachePool;

		if (m_iAmount > m_aCachePool.Count())
			m_iAmount = m_aCachePool.Count();

		SpawnRandomCaches(m_iAmount);
	}

	//------------------------------------------------------------------------------------------------
	protected void SpawnRandomCaches(int count)
	{
		if (m_aCachePool.IsEmpty() || m_sCachePrefab.IsEmpty())
			return;

		const string padding = "0";
		const string coordsFormat = "%1 %2";
		const float startPosOffset = 0.5;
		const float endPosOffset = -1;
		int gridX, gridZ;
		string output;
		array<ref SCR_CacheLocationData> tempPool = {};
		foreach (SCR_CacheLocationData data : m_aCachePool)
		{
			tempPool.Insert(data);
		}

		EntitySpawnParams params = new EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		ArmaReforgerScripted game = GetGame();
		if (!game)
			return;

		BaseWorld world = game.GetWorld();
		if (!world)
			return;
		
		vector startPos;
		vector endPos;
		TraceParam trace = new TraceParam();
		trace.Flags = TraceFlags.ENTS | TraceFlags.WORLD;
		trace.LayerMask = EPhysicsLayerDefs.Projectile;
		Resource cacheResource = Resource.Load(m_sCachePrefab);
		vector snapPosition; 
		vector normal; 
		for (int i = 0; i < count; i++)
		{
			if (tempPool.IsEmpty())
				break;
			
			int idx = Math.RandomInt(0, tempPool.Count());
			SCR_CacheLocationData chosen = tempPool[idx];
			startPos = chosen.m_vPosition;
			startPos[1] = startPos[1] + startPosOffset;
			endPos = chosen.m_vPosition;
			endPos[1] = endPos[1] + endPosOffset;
			trace.Start = startPos;
			trace.End = endPos;
			// Tracing the surface under the cache to set correct angles for the spawn. If check goes too far - snapping to the terrain
			float hitFraction = world.TraceMove(trace, null);
			if (hitFraction < 1.0)
			{
				snapPosition = startPos + ((endPos - startPos) * hitFraction);
				normal = trace.TraceNorm;
			}
			else
			{
				SCR_TerrainHelper.SnapToGeometry(snapPosition, chosen.m_vPosition, null, world, surfaceNormal: normal);
			}
				
			Math3D.MatrixIdentity4(params.Transform);
			if (normal != vector.Zero)
			{
				Math3D.MatrixFromUpVec(normal, params.Transform);
			}
			
			params.Transform[3] = snapPosition;
			IEntity spawnedEntity = game.SpawnEntityPrefab(cacheResource, world, params);
			if (spawnedEntity)
			{
				gridX = 0;
				gridZ = 0;
				SCR_MapEntity.GetGridPos(params.Transform[3], gridX, gridZ);
				output = string.Format(coordsFormat, SCR_StringHelper.PadLeft(gridX.ToString(), 3, padding), SCR_StringHelper.PadLeft(gridZ.ToString(), 3, padding));
				m_aActiveCacheHints.Insert(output);
			}

			tempPool.Remove(idx);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[out] outLines
	void GetRandomCacheData(out notnull array<string> outLines)
	{
		outLines.Copy(m_aActiveCacheHints);
	}
}
