//! Handles looped ambient effects around specific entities
[BaseContainerProps(configRoot: true)]
class SCR_ObjectPositionalInsects : SCR_AmbientInsectsEffect
{
	[Attribute(defvalue: "100", UIWidgets.Slider, params: "0 1000 1", desc: "How far the player has to move (squared distance) in order to refresh insects around")]
	protected int m_iMinimumMoveDistanceSq;

	[Attribute(defvalue: "30", UIWidgets.Slider, params: "0 100 1", desc: "How far it will seek the objects in range for the Insects to spawn around")]
	protected float m_fSearchDistance;

	[Attribute(desc: "Enables setting up which effects are to be spawned around which entitites")]
	protected ref array<ref SCR_PositionalInsectType> 	m_aPositionalInsectType;

	protected ref array<ref SCR_InsectSpawnDef> m_aSpawnDef = {};
	protected vector m_vCameraPosLooped;

	//------------------------------------------------------------------------------------------------
	//! Called by SCR_AmbientInsectsComponent.Update()
	override void Update(float worldTime, vector cameraPos, float timeOfDay, float rainIntensity)
	{
		// Limit processing by time and moved distance
		if (vector.DistanceSqXZ(m_vCameraPosLooped, cameraPos) < m_iMinimumMoveDistanceSq)
			return;

		m_vCameraPosLooped = cameraPos;

		// First we get closest entities coresponding to each InsectType
		GetClosestsEntities(cameraPos);

		// Then we trigger update for each InsectType
		foreach (SCR_PositionalInsectType type : m_aPositionalInsectType)
		{
			type.Update(cameraPos);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Finds Entity for each InsectType and adds it to the array of closest entities
	protected void ProcessEntity(notnull IEntity entity)
	{
		EntityPrefabData prefabData = entity.GetPrefabData();
		if (!prefabData)
			return;

		BaseContainer prefabContainer = prefabData.GetPrefab();
		if (!prefabContainer)
			return;

		SCR_PositionalAmbientLeafParticles leaves;
		BaseContainer prefabContainerToTest;
		// According to the prefab, if time and weather conditions are valid, it is added to the respective Insect type 
		foreach (int i, SCR_PositionalInsectType type : m_aPositionalInsectType)
		{
			leaves = SCR_PositionalAmbientLeafParticles.Cast(type);
			if (leaves)
				continue;
			
			if (type.m_fDayTimeCurve <= 0)
				continue;

			if (type.m_fRainIntensityCurve <= 0)
				continue;

			prefabContainerToTest = prefabContainer;
			while (prefabContainerToTest)
			{
				if (type.GetPrefabContainerSet().Contains(prefabContainerToTest))
				{
					type.AddClosestEntity(entity);
					return;
				}
	
				prefabContainerToTest = prefabContainerToTest.GetAncestor();
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Clears ClosestEntities array for each InsectType and then runs new querry to fill it again
	void GetClosestsEntities(vector cameraPos)
	{
		// Clear closest entities for all types and retrieve their curve values for time and weather
		foreach (int i, SCR_PositionalInsectType type : m_aPositionalInsectType)
		{
			type.ClearClosestEntities();

			type.m_fDayTimeCurve = SCR_AmbientSoundsComponent.GetPoint(m_AmbientInsectsComponent.GetTimeOfDay(), type.m_SpawnDef.m_TimeModifier);
			type.m_fRainIntensityCurve = SCR_AmbientSoundsComponent.GetPoint(m_AmbientInsectsComponent.GetRainIntensity(), type.m_SpawnDef.m_RainModifier);
		}
		
		ChimeraWorld world = GetGame().GetWorld();
		if (!world)
			return;

		//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
		// Using TagManager we get array of closest entities where Insects can spawn
		TagSystem tagSystem = TagSystem.Cast(world.FindSystem(TagSystem));
		if (!tagSystem)
		{
			Print("AMBIENT LIFE: SCR_AmbientInsectsComponent: Missing TagManager in the world", LogLevel.WARNING);
			m_AmbientInsectsComponent.RemoveInsectEffect(this);
			
			return;
		}

		array<IEntity> closestEntities = {};
		tagSystem.GetTagsInRange(closestEntities, cameraPos, m_fSearchDistance, ETagCategory.Insect);

		// Process each entity to add it to the respective Insect type
		foreach (IEntity entity : closestEntities)
		{
			ProcessEntity(entity);
		}
		
		//---- REFACTOR NOTE END ----
	}

	//------------------------------------------------------------------------------------------------
	//! Called by SCR_AmbientInsectsComponent in EOnPostInit()
	override void OnPostInit(SCR_AmbientSoundsComponent ambientSoundsComponent, SCR_AmbientInsectsComponent ambientInsectsComponent, SignalsManagerComponent signalsManagerComponent)
	{
		super.OnPostInit(ambientSoundsComponent, ambientInsectsComponent, signalsManagerComponent);

		foreach (SCR_PositionalInsectType type : m_aPositionalInsectType)
		{
			type.Init(ambientSoundsComponent, this, ambientInsectsComponent);
		}
	}
}
