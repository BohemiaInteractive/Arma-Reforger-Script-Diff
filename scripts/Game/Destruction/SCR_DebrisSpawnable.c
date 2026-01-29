[BaseContainerProps(), SCR_Spawnable_SmallDebrisTitle()]
class SCR_DebrisSpawnable : SCR_BaseSpawnable
{
	[Attribute("0 0 0", UIWidgets.Coords, "Random spawn position offset range (X Y Z in meters)", params: "0 inf")]
	vector m_vRandomPositionOffset;
	[Attribute(ResourceName.Empty, UIWidgets.ResourcePickerThumbnail, "Debris model prefabs to spawn (spawns ALL of them)", "et xob")]
	ref array<ResourceName> m_ModelPrefabs;
	[Attribute("0", desc: "Remap materials on all models defined in Phase Model array to those used on owner prefab. Remap is applied based on material slot names.")]
	bool m_bUseMaterialsFromParent;
	[Attribute("", UIWidgets.Object, "List of slots which should be remaped. If Materials From Parent is active, materials from parent will be extented by this list.", category: "Destruction Multi-Phase")]
	ref array<ref SCR_DamageRemapData> m_aRemapData;
	[Attribute("10", UIWidgets.Slider, "Mass of the debris", "0.01 1000 0.01")]
	float m_fMass;
	[Attribute("5", UIWidgets.Slider, "Minimum lifetime value for the debris (in s)", "0 3600 0.5")]
	float m_fLifetimeMin;
	[Attribute("10", UIWidgets.Slider, "Maximum lifetime value for the debris (in s)", "0 3600 0.5")]
	float m_fLifetimeMax;
	[Attribute("200", UIWidgets.Slider, "Maximum distance from camera above which the debris is not spawned (in m)", "0 3600 0.5")]
	float m_fDistanceMax;
	[Attribute("0", UIWidgets.Slider, "Higher priority overrides lower priority if at or over debris limit", "0 100 1")]
	int m_fPriority;
	[Attribute("0.1", UIWidgets.Slider, "Damage received to physics impulse (speed / mass) multiplier", "0 10000 0.01")]
	float m_fDamageToImpulse;
	[Attribute("2", UIWidgets.Slider, "Damage to speed multiplier, used when objects get too much damage to impulse", "0 10000 0.01")]
	float m_fMaxDamageToSpeedMultiplier;
	[Attribute("0.5", UIWidgets.Slider, "Random linear velocity multiplier (m/s)", "0 200 0.1")]
	float m_fRandomVelocityLinear;
	[Attribute("180", UIWidgets.Slider, "Random angular velocity multiplier (deg/s)", "0 3600 0.1")]
	float m_fRandomVelocityAngular;
	[Attribute("0", uiwidget: UIWidgets.ComboBox, "Type of material for debris sound", "", ParamEnumArray.FromEnum(SCR_EMaterialSoundTypeDebris))]
	SCR_EMaterialSoundTypeDebris m_eMaterialSoundType;

#ifdef WORKBENCH
	//------------------------------------------------------------------------------------------------
	//! Returns true when attributes are the same
	//! Returns false otherwise
	override bool CompareAttributes(SCR_BaseSpawnable other)
	{
		SCR_DebrisSpawnable otherDebris = SCR_DebrisSpawnable.Cast(other);

		if (!super.CompareAttributes(other))
			return false;

		if (otherDebris.m_ModelPrefabs != m_ModelPrefabs)
			return false;

		if (otherDebris.m_fLifetimeMin != m_fLifetimeMin)
			return false;

		if (otherDebris.m_fLifetimeMax != m_fLifetimeMax)
			return false;

		if (otherDebris.m_fDistanceMax != m_fDistanceMax)
			return false;

		if (otherDebris.m_fPriority != m_fPriority)
			return false;

		if (otherDebris.m_fDamageToImpulse != m_fDamageToImpulse)
			return false;

		if (otherDebris.m_fMaxDamageToSpeedMultiplier != m_fMaxDamageToSpeedMultiplier)
			return false;

		if (otherDebris.m_fRandomVelocityLinear != m_fRandomVelocityLinear)
			return false;

		if (otherDebris.m_fRandomVelocityAngular != m_fRandomVelocityAngular)
			return false;

		if (otherDebris.m_vRandomPositionOffset != m_vRandomPositionOffset)
			return false;

		if (otherDebris.m_eMaterialSoundType != m_eMaterialSoundType)
			return false;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void SetVariables(WorldEditorAPI api, IEntitySource source, array<ref ContainerIdPathEntry> path, int index)
	{
		super.SetVariables(api, source, path, index);

		string prefabsArray = "";
		// Set all variables of the spawn object
		for (int i = 0, count = m_ModelPrefabs.Count(); i < count; i++)
		{
			prefabsArray += m_ModelPrefabs[i];

			if (i != count - 1) // Not last item
				prefabsArray += ",";
		}

		api.SetVariableValue(source, path, "m_ModelPrefabs", prefabsArray);

		api.SetVariableValue(source, path, "m_fMass", m_fMass.ToString());
		api.SetVariableValue(source, path, "m_fLifetimeMin", m_fLifetimeMin.ToString());
		api.SetVariableValue(source, path, "m_fLifetimeMax", m_fLifetimeMax.ToString());
		api.SetVariableValue(source, path, "m_fDistanceMax", m_fDistanceMax.ToString());
		api.SetVariableValue(source, path, "m_fPriority", m_fPriority.ToString());
		api.SetVariableValue(source, path, "m_fDamageToImpulse", m_fDamageToImpulse.ToString());
		api.SetVariableValue(source, path, "m_fMaxDamageToSpeedMultiplier", m_fMaxDamageToSpeedMultiplier.ToString());
		api.SetVariableValue(source, path, "m_fRandomVelocityLinear", m_fRandomVelocityLinear.ToString());
		api.SetVariableValue(source, path, "m_fRandomVelocityAngular", m_fRandomVelocityAngular.ToString());
		api.SetVariableValue(source, path, "m_vRandomPositionOffset", m_vRandomPositionOffset.ToString());
		api.SetVariableValue(source, path, "m_eMaterialSoundType", m_eMaterialSoundType.ToString());
	}

	//------------------------------------------------------------------------------------------------
	override bool CreateObject(WorldEditorAPI api, IEntitySource source, array<ref ContainerIdPathEntry> path, int index)
	{
		if (!AlreadyExists(api, source, index))
		{
			api.CreateObjectArrayVariableMember(source, path, "m_aPhaseDestroySpawnObjects", "SCR_DebrisSpawnable", index);
			return true;
		}

		return false;
	}
#endif

	//------------------------------------------------------------------------------------------------
	//! Spawns the object
	override IEntity Spawn(IEntity owner, Physics parentPhysics, SCR_HitInfo hitInfo, bool snapToTerrain = false)
	{
		if (!hitInfo)
			return null;

		int numModelPrefabs = 0;
		if (m_ModelPrefabs)
			numModelPrefabs = m_ModelPrefabs.Count();

		ResourceName modelEmpty;
		string remapParent;
		if(m_bUseMaterialsFromParent)
		{
			ResourceName parentModel = SCR_ResourceNameUtils.GetPrefabName(owner);
			SCR_Global.GetModelAndRemapFromResource(parentModel, modelEmpty, remapParent);
		}
		string remapData;
		foreach (SCR_DamageRemapData remapEntry: m_aRemapData)
		{
			remapData += "$remap '" + remapEntry.m_sMaterialSlotName + "' '" + remapEntry.m_sMaterialName + "';";
		}

		for (int i = 0; i < numModelPrefabs; i++)
		{
			ResourceName prefabPath = m_ModelPrefabs[i];

			ResourceName modelPath;
			string remap;
			SCR_Global.GetModelAndRemapFromResource(prefabPath, modelPath, remap);
			// Trying to remap slots which are not present in modelPath will result in error in Log Console. This is why we need to sanitize remap string to only contain valid entries.
			// GetModelAndRemapFromResource will return non empty remap only if prefabPath is linking to actual prefab with remaped materials.
			// Above behavior can be used to create a prefab, which remaps only 2 certain slots - those will be retrieved by code and used in sanitization process.
			if (m_bUseMaterialsFromParent)
			{
				if (remap)
				{
					remap = SCR_DestructionUtility.SanitizeRemapString(remap,remapParent);
				}
				else
				{
					remap = remapParent;
				}
			}

			remap += remapData;

			if (modelPath == ResourceName.Empty)
				continue;

			vector spawnMat[4];
			GetSpawnTransform(owner, spawnMat);

			// Add random offset to spawn position
			vector randomOffset = vector.Zero;
			for (int axis = 0; axis < 3; axis++)
			{
				if (m_vRandomPositionOffset[axis] != 0)
					randomOffset[axis] = Math.RandomFloat(-m_vRandomPositionOffset[axis], m_vRandomPositionOffset[axis]);
			}
			spawnMat[3] = spawnMat[3] + randomOffset;

			SCR_DestructionDamageManagerComponent destructionComponent = SCR_DestructionDamageManagerComponent.Cast(owner.FindComponent(SCR_DestructionDamageManagerComponent));

			float dmgSpeed = Math.Clamp(hitInfo.m_HitDamage * m_fDamageToImpulse / m_fMass, 0, m_fMaxDamageToSpeedMultiplier);

			vector linearVelocity = hitInfo.m_HitDirection * Math.RandomFloat(0, 1);
			linearVelocity += Vector(Math.RandomFloat(-1, 1), Math.RandomFloat(-1, 1), Math.RandomFloat(-1, 1)) * m_fRandomVelocityLinear;
			linearVelocity *= dmgSpeed;
			vector angularVelocity = Vector(Math.RandomFloat(-1, 1), Math.RandomFloat(-1, 1), Math.RandomFloat(-1, 1)) * Math.RandomFloat(0.25, 4) * m_fRandomVelocityAngular;
			angularVelocity *= dmgSpeed;

			if (parentPhysics)
			{
				linearVelocity += parentPhysics.GetVelocity();
				angularVelocity += parentPhysics.GetAngularVelocity();
			}
#ifdef ENABLE_BASE_DESTRUCTION
			SCR_DebrisSmallEntity.SpawnDebris(owner.GetWorld(), spawnMat, modelPath, m_fMass, Math.RandomFloat(m_fLifetimeMin, m_fLifetimeMax), m_fDistanceMax, m_fPriority, linearVelocity, angularVelocity, remap, false, m_eMaterialSoundType);
#endif
		}

		return null;
	}
}