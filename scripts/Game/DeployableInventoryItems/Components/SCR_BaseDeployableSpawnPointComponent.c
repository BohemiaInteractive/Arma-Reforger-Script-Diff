[EntityEditorProps(category: "GameScripted/DeployableItems", description: "")]
class SCR_BaseDeployableSpawnPointComponentClass : SCR_BaseDeployableInventoryItemComponentClass
{
}

//! Base class which all deployable spawn points / radios inherit from
class SCR_BaseDeployableSpawnPointComponent : SCR_BaseDeployableInventoryItemComponent
{
	protected static ref array<SCR_BaseDeployableSpawnPointComponent> s_aActiveDeployedSpawnPoints = {};

	protected static ref ScriptInvokerInt s_OnSpawnPointDismantled;

	// Setup
	[Attribute("{35347E7AA6BDF8CE}Prefabs/MP/Spawning/ItemSpecifics/RestrictedDeployableSpawnPoint_Radio.et", uiwidget: UIWidgets.ResourcePickerThumbnail, params: "et", category: "Setup")]
	protected ResourceName m_sSpawnPointPrefab;

	// General
	[Attribute(category: "General")]
	protected FactionKey m_FactionKey;

	protected SCR_DeployableSpawnPoint m_SpawnPoint;

	protected bool m_bDeployableSpawnPointsEnabled;

	//------------------------------------------------------------------------------------------------
	static ScriptInvokerInt GetOnSpawnPointDismantled()
	{
		if (!s_OnSpawnPointDismantled)
			s_OnSpawnPointDismantled = new ScriptInvokerInt();

		return s_OnSpawnPointDismantled;
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnRplDeployed()
	{
		SCR_BaseDeployableInventoryItemComponentClass data = SCR_BaseDeployableInventoryItemComponentClass.Cast(GetComponentData(GetOwner()));
		if (data && data.IsSoundEnabled())
			ToggleRadioChatter(m_bIsDeployed);
	}

	//------------------------------------------------------------------------------------------------
	protected void ToggleRadioChatter(bool enable)
	{
		SignalsManagerComponent signalsManagerComp = SignalsManagerComponent.Cast(GetOwner().FindComponent(SignalsManagerComponent));
		if (!signalsManagerComp)
			return;

		int radioChatter = signalsManagerComp.FindSignal("DeployableRadioChatter");

		if (enable)
			signalsManagerComp.SetSignalValue(radioChatter, 1);
		else
			signalsManagerComp.SetSignalValue(radioChatter, 0);
	}
	
	//------------------------------------------------------------------------------------------------
	protected SCR_DeployableSpawnPoint CreateSpawnPoint()
	{
		Resource resource = Resource.Load(m_sSpawnPointPrefab);
		if (!resource.IsValid())
			return null;

		EntitySpawnParams params();
		params.Transform = m_aOriginalTransform;
		params.TransformMode = ETransformMode.WORLD;
		return SCR_DeployableSpawnPoint.Cast(GetGame().SpawnEntityPrefab(resource, GetGame().GetWorld(), params));
	}

	//------------------------------------------------------------------------------------------------
	//! Spawns replacement composition and attaches owner entity to it - called from SCR_DeployItemBaseAction.PerformAction
	//! \param[in] userEntity
	override void Deploy(IEntity userEntity = null, bool reload = false)
	{
		//~ Not allowed to deploy
		if (!m_bDeployableSpawnPointsEnabled)
			return;

		if (!m_RplComponent || m_RplComponent.IsProxy())
			return;

		if (m_bIsDeployed)
			return;

		super.Deploy(userEntity, reload);

		m_SpawnPoint = CreateSpawnPoint();
		if (!m_SpawnPoint)
			return;

		m_SpawnPoint.SetDeployableSpawnPointComponent(this);
		m_SpawnPoint.SetFactionKey(m_FactionKey);

		s_aActiveDeployedSpawnPoints.Insert(this);

		SCR_BaseDeployableInventoryItemComponentClass data = SCR_BaseDeployableInventoryItemComponentClass.Cast(GetComponentData(GetOwner()));
		if (data && data.IsSoundEnabled())
			ToggleRadioChatter(m_bIsDeployed);
	}

	//------------------------------------------------------------------------------------------------
	//! Delete replacement composition and spawnpoint and set position of owner entity to it's original position - called from SCR_DismantleItemBaseAction.PerformAction
	//! \param[in] userEntity
	override void Dismantle(IEntity userEntity = null, bool reload = false)
	{
		if (!m_RplComponent || m_RplComponent.IsProxy())
			return;

		if (!m_bIsDeployed)
			return;

		super.Dismantle(userEntity, reload);

		if (m_SpawnPoint)
		{
			RplComponent rplComp = RplComponent.Cast(m_SpawnPoint.FindComponent(RplComponent));
			if (!rplComp)
				return;

			rplComp.DeleteRplEntity(m_SpawnPoint, false);
		}

		s_aActiveDeployedSpawnPoints.RemoveItem(this);

		int userID = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(userEntity);
		if (s_OnSpawnPointDismantled)
			s_OnSpawnPointDismantled.Invoke(userID);

		SCR_BaseDeployableInventoryItemComponentClass data = SCR_BaseDeployableInventoryItemComponentClass.Cast(GetComponentData(GetOwner()));
		if (data && data.IsSoundEnabled())
			ToggleRadioChatter(m_bIsDeployed);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnSpawnPointDeployingEnabledChanged(bool enabled)
	{
		m_bDeployableSpawnPointsEnabled = enabled;

		if (!m_RplComponent || m_RplComponent.IsProxy())
			return;

		if (enabled)
		{
			if (!m_bIsDeployed && m_bWasDeployed)
			{
				m_bWasDeployed = false;
				Deploy(m_PreviousOwner);
			}

			return;
		}

		if (m_bIsDeployed)
		{
			m_PreviousOwner = GetGame().GetPlayerManager().GetPlayerControlledEntity(m_iItemOwnerID);
			m_bWasDeployed = true;
			Dismantle();
		}
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	static array<SCR_BaseDeployableSpawnPointComponent> GetActiveDeployedSpawnPoints()
	{
		return s_aActiveDeployedSpawnPoints;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	SCR_SpawnPoint GetSpawnPoint()
	{
		return m_SpawnPoint;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] timeSlice
	void Update(float timeSlice);

	//------------------------------------------------------------------------------------------------
	protected void ConnectToDeployableSpawnPointSystem()
	{
		World world = GetOwner().GetWorld();
		SCR_DeployableSpawnPointSystem updateSystem = SCR_DeployableSpawnPointSystem.Cast(world.FindSystem(SCR_DeployableSpawnPointSystem));
		if (!updateSystem)
			return;

		updateSystem.Register(this);
	}

	//------------------------------------------------------------------------------------------------
	protected void DisconnectFromDeployableSpawnPointSystem()
	{
		World world = GetOwner().GetWorld();
		SCR_DeployableSpawnPointSystem updateSystem = SCR_DeployableSpawnPointSystem.Cast(world.FindSystem(SCR_DeployableSpawnPointSystem));
		if (!updateSystem)
			return;

		updateSystem.Unregister(this);
	}

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);

		BaseGameMode gameMode = GetGame().GetGameMode();
		if (!gameMode)
			return;

		SCR_PlayerSpawnPointManagerComponent playerSpawnPointManager = SCR_PlayerSpawnPointManagerComponent.Cast(gameMode.FindComponent(SCR_PlayerSpawnPointManagerComponent));
		if (!playerSpawnPointManager)
		{
			OnSpawnPointDeployingEnabledChanged(true);
			return;
		}

		OnSpawnPointDeployingEnabledChanged(playerSpawnPointManager.IsDeployingSpawnPointsEnabled());
		playerSpawnPointManager.GetOnSpawnPointDeployingEnabledChanged().Insert(OnSpawnPointDeployingEnabledChanged);
	}

	//------------------------------------------------------------------------------------------------
	override void OnDelete(IEntity owner)
	{
		super.OnDelete(owner);

		DisconnectFromDeployableSpawnPointSystem();

		s_aActiveDeployedSpawnPoints.RemoveItem(this);

		if (s_OnSpawnPointDismantled)
			s_OnSpawnPointDismantled.Invoke(-1);

		if (!m_bIsDeployed || !m_RplComponent || m_RplComponent.IsProxy())
			return;

		if (!m_SpawnPoint || m_SpawnPoint.IsDeleted())
			return;

		RplComponent rplComp = RplComponent.Cast(m_SpawnPoint.FindComponent(RplComponent));
		if (!rplComp)
			return;

		rplComp.DeleteRplEntity(m_SpawnPoint, false);
	}

	//------------------------------------------------------------------------------------------------
	// destructor
	void ~SCR_BaseDeployableSpawnPointComponent()
	{
		BaseGameMode gameMode = GetGame().GetGameMode();
		if (gameMode)
		{
			SCR_PlayerSpawnPointManagerComponent playerSpawnPointManager = SCR_PlayerSpawnPointManagerComponent.Cast(gameMode.FindComponent(SCR_PlayerSpawnPointManagerComponent));
			if (playerSpawnPointManager)
				playerSpawnPointManager.GetOnSpawnPointDeployingEnabledChanged().Remove(OnSpawnPointDeployingEnabledChanged);
		}

		if (m_bIsDeployed && Replication.IsServer())
			s_aActiveDeployedSpawnPoints.RemoveItem(this);
	}
}
