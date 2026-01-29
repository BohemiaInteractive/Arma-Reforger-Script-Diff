//------------------------------------------------------------------------------------------------
class SCR_GameModeLastStandClass: SCR_BaseGameModeClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_GameModeLastStand : SCR_BaseGameMode
{
	[Attribute("10", UIWidgets.EditBox, "How long between clearing the last wave and a new one")]
	float m_TimeoutBetweenWaves;

	[Attribute("4", UIWidgets.EditBox, "How many groups spawned each wave")]
	int m_InitialGroupsCount;

	[Attribute("1", UIWidgets.EditBox, "How many new groups should spawn each round")]
	int m_AddedGroupsPerRound;
	
	[Attribute("", UIWidgets.ResourceNamePicker, "What group type should spawn")]
	ResourceName m_GroupType;
	
	[Attribute("USSR", UIWidgets.EditBox, "Enemy faction key")]
	protected FactionKey m_sEnemyFactionKey;
	
	[Attribute("US", UIWidgets.EditBox, "Friendly faction key")]
	protected FactionKey m_sFriendlyFactionKey;
	
	int m_iGroupsToSpawn;
	int m_iRoundNumber;

	Widget m_wRoot;
	TextWidget m_wText;
	ref array<IEntity> m_aEnemySoldiers;
	ref array<SCR_AIGroup> m_aGroups;
	ref array<SCR_SpawnPoint> m_aEnemySpawnPoints;
	ref array<SCR_SpawnPoint> m_aPlayerSpawnPoints;
	float m_fCurrentTimeout;
	AIWaypoint m_AttackWP;
	
	//------------------------------------------------------------------------------------------------
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		//PrintString("printing");
		super.EOnFrame(owner, timeSlice);
		
		if (m_fCurrentTimeout > 0)
		{
			m_fCurrentTimeout -= timeSlice;
			if (m_fCurrentTimeout < 0)
			{
				m_fCurrentTimeout = 0;
				m_iRoundNumber++;
				if (m_iRoundNumber > 0)
				{
					SpawnEnemies(m_iRoundNumber);
					Print("Sending next wave of enemies.");
					ShowHint("Sending next wave of enemies.", 5);
				}
			}
		}
		else if (m_aEnemySoldiers.Count() == 0)
		{
			Print("All enemies are dead. Wave cleared.");
			ShowHint("All enemies are dead. Wave cleared.", 5);
			m_fCurrentTimeout = m_TimeoutBetweenWaves;
		}
	}

	//------------------------------------------------------------------------------------------------
	void ShowHint(string text, float showTime)
	{
		if (!m_wText || !m_wRoot)
			return;
		
		m_wText.SetText(text);
		AnimateWidget.Opacity(m_wRoot, 1, 1);
		
		ScriptCallQueue queue = GetGame().GetCallqueue(); 
		queue.CallLater(this.HideHint, showTime * 1000);
	}
	
	//------------------------------------------------------------------------------------------------
	void HideHint()
	{
		if (m_wRoot)
			AnimateWidget.Opacity(m_wRoot, 0, 1);
	}
	
	//------------------------------------------------------------------------------------------------
	void SpawnEnemies(int round)
	{
		if (m_aEnemySpawnPoints.Count() == 0)
			return;
		
		// Clean up groups
		for (int i = m_aGroups.Count() - 1; i >= 0; i--)
		{
			delete m_aGroups[i];
		}
		m_aGroups.Clear();
		m_aEnemySoldiers.Clear();
		
		m_iGroupsToSpawn = m_InitialGroupsCount + (round - 1) * m_AddedGroupsPerRound;
		
		for (int i = 0; i < m_iGroupsToSpawn; i++)
		{
			RandomGenerator generator = new RandomGenerator;
			generator.SetSeed(Math.RandomInt(0,100));

			SCR_SpawnPoint spawnPoint = m_aEnemySpawnPoints.GetRandomElement();
			if (!spawnPoint)
				return;
			
			vector position = generator.GenerateRandomPointInRadius(0, 2, spawnPoint.GetOrigin());
			position[1] = spawnPoint.GetOrigin()[1];
			EntitySpawnParams params = EntitySpawnParams();
			params.TransformMode = ETransformMode.WORLD;
			params.Transform[3] = position;
			
			Resource res = Resource.Load(m_GroupType);
			SCR_AIGroup newGrp = SCR_AIGroup.Cast(GetGame().SpawnEntityPrefab(res, null, params));
			m_aGroups.Insert(newGrp);
			
			array<AIAgent> agents = new array<AIAgent>;
			
			newGrp.GetAgents(agents);
			foreach (AIAgent agent : agents)
			{
				if (agent)
				{
					m_aEnemySoldiers.Insert(agent.GetControlledEntity());
				}
			}
			
			if (m_AttackWP)
			{
				newGrp.AddWaypointToGroup(m_AttackWP);
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnControllableDestroyed(IEntity entity, IEntity killerEntity, notnull Instigator instigator)
	{
		super.OnControllableDestroyed(entity, killerEntity, instigator);
		
		m_aEnemySoldiers.RemoveItemOrdered(entity);
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);

		m_fCurrentTimeout = m_TimeoutBetweenWaves;

		m_wRoot = GetGame().GetWorkspace().CreateWidgets("{EA1CC57D868E94F9}UI/layouts/HUD/Hint.layout");
		if (m_wRoot)
		{
			m_wText = TextWidget.Cast(m_wRoot.FindAnyWidget("Text"));
			TextWidget title = TextWidget.Cast(m_wRoot.FindAnyWidget("Title"));
			if (title)
				title.SetText("LAST STAND");
	
			m_wRoot.SetOpacity(0);
		}
		BaseWorld world = owner.GetWorld();
		m_AttackWP = AIWaypoint.Cast(world.FindEntityByName("WP1"));
		m_aEnemySoldiers = {};
		m_aGroups = {};
		m_aPlayerSpawnPoints = {};
		m_aEnemySpawnPoints = {};
		array<SCR_SpawnPoint> spawnPoints = {};
		spawnPoints = SCR_SpawnPoint.GetSpawnPoints();

		FactionKey faction;
		SCR_SpawnPoint spawnPoint;
		for (int i = spawnPoints.Count() - 1; i >= 0; i--)
		{
			spawnPoint = spawnPoints[i];
			if (!spawnPoint)
				continue;

			faction = spawnPoint.GetFactionKey();
			if (faction == m_sEnemyFactionKey)
				m_aEnemySpawnPoints.Insert(spawnPoint);

			if (faction == m_sFriendlyFactionKey)
				m_aPlayerSpawnPoints.Insert(spawnPoint);
		}
	}
};

