class SCR_RaceCustomizationPointEntityClass : GenericEntityClass
{
}

class SCR_RaceCustomizationPointEntity : GenericEntity
{
	// TODO: proper naming/casing (and Prefab/layer renaming with it)

	[Attribute(desc: "TODO", category: "Race")]
	protected ref SCR_CustomizationCameraPointInfo m_vehicleCamPosition;

	[Attribute(desc: "TODO", category: "Race")]
	protected ref SCR_CustomizationCameraPointInfo m_characterCamPosition;

	[Attribute(desc: "TODO", category: "Race")]
	protected ref SCR_CustomizationCameraPointInfo m_SpoilerCamPosition;

	[Attribute(desc: "TODO", category: "Race")]
	protected ref SCR_CustomizationCameraPointInfo m_WheelCamPosition;

	[Attribute(desc: "TODO", category: "Race")]
	protected ref SCR_CustomizationCameraPointInfo m_LightsCamPosition;

	[Attribute(desc: "TODO", category: "Race")]
	protected ref SCR_CustomizationCameraPointInfo m_vehiclePosition;

	[Attribute(desc: "TODO", category: "Race")]
	protected string m_raceToStart;

	[Attribute("{6EF50DBD7B8A8BF2}Configs/VehicleCustomization.conf")]
	protected ResourceName m_sCustomizationConfig;

	int m_iCurrentPreviewVehicleIndex;
	int m_iCurrentPreviewVehicleVariantIndex;

	protected CameraBase m_MainCamera;

	protected ref SCR_RallyVehicleCustomizationConfig m_CustomizationConfig;

	protected IEntity m_PlayerEntity;
	protected IEntity m_CurrentHat;
	protected IEntity m_CurrentOverall;
	protected IEntity m_CurrentGloves;

	protected Vehicle m_CurrentPreviewVehicle;

	protected ref SCR_RallyCustomizationData m_CustomizationData = new SCR_RallyCustomizationData();

	protected static const ResourceName CAMERA_PREFAB = "{D6DE32D1C0FCC1C7}Prefabs/Editor/Camera/ManualCameraBase.et";

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		Resource holder = BaseContainerTools.LoadContainer(m_sCustomizationConfig);
		if (!holder)
			return;

		BaseContainer container = holder.GetResource().ToBaseContainer();
		if (!container)
			return;

		m_CustomizationConfig = SCR_RallyVehicleCustomizationConfig.Cast(BaseContainerTools.CreateInstanceFromContainer(container));
		if (!m_CustomizationConfig)
			return;

		m_CurrentPreviewVehicle = SpawnVehicle();

		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (!gameMode)
			return;

		SCR_AdditionalGameModeSettingsComponent settings = SCR_AdditionalGameModeSettingsComponent.GetInstance();
		if (!settings)
			return;

		settings.SetCustomizationEntity(this);

		gameMode.GetOnPlayerSpawned().Insert(OnPlayerSpawn);
	}

	//------------------------------------------------------------------------------------------------
	void OnPlayerSpawn(int playerId, IEntity player)
	{
		m_vehicleCamPosition.Init(this);
		m_characterCamPosition.Init(this);
		m_SpoilerCamPosition.Init(this);
		m_WheelCamPosition.Init(this);
		m_LightsCamPosition.Init(this);

		m_PlayerEntity = player;

		CameraBase camera = GetGame().GetCameraManager().CurrentCamera();

		vector mat[4];
		m_vehicleCamPosition.GetWorldTransform(mat);
		EntitySpawnParams params = new EntitySpawnParams();
		params.Transform = mat;
		m_MainCamera = CameraBase.Cast(GetGame().SpawnEntityPrefab(Resource.Load(CAMERA_PREFAB), GetGame().GetWorld(), params));
		if (!m_MainCamera)
		{
			Print("SCR_RaceCustomizationPointEntity:: Failed to create camera!", LogLevel.ERROR);
			return;
		}

		m_MainCamera.SetFOVDegree(m_vehiclePosition.m_fFOV);

		GetGame().GetCameraManager().SetCamera(m_MainCamera);
		GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.RallyDLC_CustomizationMenu, 0, true, false);
	}

	//------------------------------------------------------------------------------------------------
	void OnCustomizationConfirmed()
	{
		GetGame().GetCameraManager().SetPreviousCamera();
		delete m_CurrentPreviewVehicle;

		//this should get race that is set somewhere on the map, probably on the customization point entity? As it's one race per map
		SCR_RaceEntity race = SCR_RaceEntity.Cast(GetGame().GetWorld().FindEntityByName(m_raceToStart));
		if (!race)
		{
			Print(string.Format("SCR_RaceCustomizationPointEntity::OnCustomizationConfirmed: could not find race entity named '%1'", m_raceToStart), LogLevel.ERROR);
			return;
		}

		race.StartRaceCustomized(SCR_PlayerController.GetLocalPlayerId(), m_CustomizationData);
	}

	//------------------------------------------------------------------------------------------------
	Vehicle SpawnVehicle()
	{
		array<ref SCR_RallyVehicleCustomizationData> vehicles = m_CustomizationConfig.GetVehicles();

		if (vehicles.IsEmpty())
			return null;

		if (m_CurrentPreviewVehicle)
			delete m_CurrentPreviewVehicle;

		array<ResourceName> vehicleVariants = vehicles.Get(m_iCurrentPreviewVehicleIndex).m_sVehicleVariants;

		//change this so it doesnt throw VME lol
		ResourceName vehiclePrefab = vehicleVariants.Get(m_iCurrentPreviewVehicleVariantIndex);

		Resource carPrefab = Resource.Load(vehiclePrefab);
		if (!carPrefab.IsValid())
			return null;

		EntitySpawnParams params = EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;

		vector mat[4];

		m_vehiclePosition.Init(this);
		m_vehiclePosition.GetWorldTransform(mat);

		params.Transform = mat;

		Vehicle car = Vehicle.Cast(GetGame().SpawnEntityPrefabLocal(carPrefab, null, params));
		if (!car)
			return null;

		m_CurrentPreviewVehicle = car;

		//save the selected car for spawning it later
		if (!m_CustomizationData)
			m_CustomizationData = new SCR_RallyCustomizationData();

		m_CustomizationData.SetBaseVehicle(vehiclePrefab);
		m_CustomizationData.SetVehicleName(vehicles.Get(m_iCurrentPreviewVehicleIndex).m_sVehicleTypeName);

		return car;
	}

	//------------------------------------------------------------------------------------------------
	SCR_RallyVehicleCustomizationConfig GetCustomizationConfig()
	{
		return m_CustomizationConfig;
	}

	//------------------------------------------------------------------------------------------------
	Vehicle GetCurrentPreviewVehicle()
	{
		return m_CurrentPreviewVehicle;
	}

	//------------------------------------------------------------------------------------------------
	void SetVehiclePartByPrefab(SCR_ERallyCustomizationParts part, ResourceName prefab = string.Empty)
	{
		string partName;

		switch (part)
		{
			case SCR_ERallyCustomizationParts.SPOILER:
				partName = "Spoiler";
				break;

			case SCR_ERallyCustomizationParts.BODYKIT:
				partName = "BodyKit";
				break;

			case SCR_ERallyCustomizationParts.LIGHTS:
				partName = "RallyLights";
				break;
		}

		SlotManagerComponent slotManager = SlotManagerComponent.Cast(m_CurrentPreviewVehicle.FindComponent(SlotManagerComponent));
		if (!slotManager)
			return;

		array<EntitySlotInfo> slots = {};
		slotManager.GetSlotInfos(slots);

		EntitySlotInfo foundSlot;
		foreach (EntitySlotInfo slot : slots)
		{
			if (slot.GetSourceName() == partName)
			{
				foundSlot = slot;
				break;
			}
		}
		
		if (!foundSlot)
		{
			Print("SCR_RaceCustomizationPointEntity:: Slot with name: " + partName + " not found on the vehicle!", LogLevel.ERROR);
			return;
		}

		// An empty prefab detaches the part instead of replacing it. Only lights can be
		// removed entirely this way; for other parts an empty prefab is a no-op.
		if (prefab.IsEmpty() && part != SCR_ERallyCustomizationParts.LIGHTS)
			return;

		IEntity attachedEntity = foundSlot.GetAttachedEntity();
		if (attachedEntity)
		{
			foundSlot.DetachEntity();
			delete attachedEntity;
		}

		if (prefab.IsEmpty())
			return;

		Resource partPrefab = Resource.Load(prefab);
		if (!partPrefab.IsValid())
			return;

		//change this if going to MP?
		IEntity spawnedPart = GetGame().SpawnEntityPrefabLocal(partPrefab, null, null);

		m_CustomizationData.SetVehiclePart(part, prefab);
		foundSlot.AttachEntity(spawnedPart);
	}

	//------------------------------------------------------------------------------------------------
	void SetCharacterPartByPrefab(string slotName, ResourceName prefab)
	{
		SCR_InventoryStorageManagerComponent inventoryManager = SCR_InventoryStorageManagerComponent.Cast(m_PlayerEntity.FindComponent(SCR_InventoryStorageManagerComponent));
		if (!inventoryManager)
			return;

		SCR_CharacterInventoryStorageComponent storageComponent = SCR_CharacterInventoryStorageComponent.Cast(m_PlayerEntity.FindComponent(SCR_CharacterInventoryStorageComponent));
		if (!storageComponent)
			return;

		//separate this into deletion method maybe?
		switch (slotName)
		{
			case "Hat":
				inventoryManager.TryDeleteItem(m_CurrentHat);
				m_CurrentHat = null;
				break;

			case "Overal":
				inventoryManager.TryDeleteItem(m_CurrentOverall);
				m_CurrentOverall = null;
				break;

			case "Gloves":
				inventoryManager.TryDeleteItem(m_CurrentGloves);
				m_CurrentGloves = null;
				break;
		}

		Resource partPrefab = Resource.Load(prefab);
		if (!partPrefab.IsValid())
			return;

		EntitySpawnParams params = EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;

		vector mat[4];

		m_vehiclePosition.Init(this);
		m_vehiclePosition.GetWorldTransform(mat);

		params.Transform = mat;

		IEntity spawnedPart = GetGame().SpawnEntityPrefabLocal(partPrefab, null, params);

		switch (slotName)
		{
			case "Hat":
				m_CurrentHat = spawnedPart;
				break;

			case "Overal":
				m_CurrentOverall = spawnedPart;
				break;

			case "Gloves":
				m_CurrentGloves = spawnedPart;
				break;
		}

		if (spawnedPart)
			inventoryManager.EquipAny(storageComponent, spawnedPart);
	}

	//------------------------------------------------------------------------------------------------
	string GetAssignedRace()
	{
		return m_raceToStart;
	}

	//------------------------------------------------------------------------------------------------
	int GetCurrentVehicleIndex()
	{
		return m_iCurrentPreviewVehicleIndex;
	}

	//------------------------------------------------------------------------------------------------
	SCR_RallyCustomizationData GetCustomizationData()
	{
		return m_CustomizationData;
	}

	//------------------------------------------------------------------------------------------------
	void MoveCamera(SCR_ERallyCustomizationCameraPosition position)
	{
		if (!m_MainCamera)
			return;

		SCR_CustomizationCameraPointInfo positionPointInfo;

		switch (position)
		{
			case SCR_ERallyCustomizationCameraPosition.VEHICLE:
				positionPointInfo = m_vehicleCamPosition;
				break;

			case SCR_ERallyCustomizationCameraPosition.CHARACTER:
				positionPointInfo = m_characterCamPosition;
				break;

			case SCR_ERallyCustomizationCameraPosition.RACE:
				SCR_RaceEntity race = SCR_RaceEntity.Cast(GetGame().GetWorld().FindEntityByName(m_raceToStart));
				if (!race)
				{
					Print("SCR_RaceCustomizationPointEntity:: MoveCamera(RACE): could not find race entity named '" + m_raceToStart + "'!", LogLevel.ERROR);
					return;
				}

				positionPointInfo = race.GetRaceCamPosition();
				if (!positionPointInfo)
				{
					Print("SCR_RaceCustomizationPointEntity:: MoveCamera(RACE): race '" + m_raceToStart + "' has no m_raceCamPosition set!", LogLevel.ERROR);
					return;
				}

				break;

			case SCR_ERallyCustomizationCameraPosition.SPOILER:
				positionPointInfo = m_SpoilerCamPosition;
				break;

			case SCR_ERallyCustomizationCameraPosition.WHEEL:
				positionPointInfo = m_WheelCamPosition;
				break;

			case SCR_ERallyCustomizationCameraPosition.LIGHTS:
				positionPointInfo = m_LightsCamPosition;
				break;
		}

		vector mat[4];
		positionPointInfo.GetWorldTransform(mat);
		m_MainCamera.SetWorldTransform(mat);
		m_MainCamera.SetFOVDegree(positionPointInfo.m_fFOV);
	}

	//------------------------------------------------------------------------------------------------
	void SCR_RaceCustomizationPointEntity(IEntitySource src, IEntity parent)
	{
		SetEventMask(EntityEvent.INIT | EntityEvent.FRAME);
		Activate();
	}
}

[EnumLinear()]
enum SCR_ERallyCustomizationCameraPosition
{
	VEHICLE,
	CHARACTER,
	RACE,
	SPOILER,
	WHEEL,
	LIGHTS,
}

[EnumLinear()]
enum SCR_ERallyCustomizationParts
{
	SPOILER,
	BODYKIT,
	LIGHTS,
}

class SCR_RallyCustomizationData
{
	ResourceName m_sBaseVehicle;
	protected ref map<SCR_ERallyCustomizationParts, ResourceName> m_mVehicleParts;
	protected string m_sVehicleName;
	protected string m_sWeather;
	protected string m_sStartingTime;
	protected bool m_bShowGhost = true;

	//------------------------------------------------------------------------------------------------
	ResourceName GetBaseVehicle()
	{
		return m_sBaseVehicle;
	}

	//------------------------------------------------------------------------------------------------
	void SetBaseVehicle(ResourceName vehiclePrefab)
	{
		m_sBaseVehicle = vehiclePrefab;

		//if you change vehicle the customization for vehicle is lost
		if (m_mVehicleParts)
			m_mVehicleParts.Clear();
	}

	//------------------------------------------------------------------------------------------------
	ResourceName GetVehiclePart(SCR_ERallyCustomizationParts part)
	{
		if (!m_mVehicleParts)
			return string.Empty;

		return m_mVehicleParts.Get(part);
	}

	//------------------------------------------------------------------------------------------------
	void SetVehiclePart(SCR_ERallyCustomizationParts part, ResourceName prefab)
	{
		if (!m_mVehicleParts)
			m_mVehicleParts = new map<SCR_ERallyCustomizationParts, ResourceName>();

		m_mVehicleParts.Set(part, prefab);
	}

	//------------------------------------------------------------------------------------------------
	string GetVehicleName()
	{
		return m_sVehicleName;
	}

	//------------------------------------------------------------------------------------------------
	void SetVehicleName(string vehicleName)
	{
		m_sVehicleName = vehicleName;
	}

	//------------------------------------------------------------------------------------------------
	string GetWeather()
	{
		return m_sWeather;
	}

	//------------------------------------------------------------------------------------------------
	void SetWeather(string weather)
	{
		m_sWeather = weather;
	}

	//------------------------------------------------------------------------------------------------
	string GetStartingTime()
	{
		return m_sStartingTime;
	}

	//------------------------------------------------------------------------------------------------
	void SetStartingTime(string startingTime)
	{
		m_sStartingTime = startingTime;
	}

	//------------------------------------------------------------------------------------------------
	bool GetShowGhost()
	{
		return m_bShowGhost;
	}

	//------------------------------------------------------------------------------------------------
	void SetShowGhost(bool showGhost)
	{
		m_bShowGhost = showGhost;
	}
}
