[ComponentEditorProps(category: "GameScripted/Destruction", description: "Multi-Phase destruction component, for objects that go through several damage phases")]
class SCR_DestructionMultiPhaseComponentClass: SCR_DestructionDamageManagerComponentClass
{
	[Attribute("1", UIWidgets.CheckBox, "If true, the object will be deleted after being damaged beyond the final damage phase", category: "Destruction Multi-Phase")]
	bool m_bDeleteAfterFinalPhase;
	[Attribute("", UIWidgets.Object, "List of the individual damage phases (sequential) above initial damage phase", category: "Destruction Multi-Phase")]
	ref array<ref SCR_DamagePhaseData> m_aDamagePhases;

	[Attribute("0", uiwidget: UIWidgets.ComboBox, "Type of material for destruction sound", "", ParamEnumArray.FromEnum(SCR_EMaterialSoundTypeBreak))]
	SCR_EMaterialSoundTypeBreak m_eMaterialSoundType;

	[Attribute("0")]
	float m_fMeshChangeDelay;

	[Attribute()]
	ref DestructionHeatmapEntry m_EntryData;
}

//! Multi-Phase destruction component, for objects that go through several destruction states
class SCR_DestructionMultiPhaseComponent : SCR_DestructionDamageManagerComponent
{
#ifdef ENABLE_BASE_DESTRUCTION
	private static int s_iFirstFreeDestructionMultiPhaseData = -1;
	private static ref array<ref SCR_DestructionMultiPhaseData> s_aDestructionMultiPhaseData = {};

	private int m_iDestructionMultiPhaseDataIndex = -1;

	protected int m_iOriginalHealth;
	protected float m_fNextPhaseHealth;

	static const string DAMAGE_PHASE_SIGNAL_NAME = "DamagePhase";

	//------------------------------------------------------------------------------------------------
	protected notnull SCR_DestructionMultiPhaseData GetDestructionMultiPhaseData()
	{
		if (m_iDestructionMultiPhaseDataIndex == -1)
			m_iDestructionMultiPhaseDataIndex = AllocateDestructionMultiPhaseData();

		return s_aDestructionMultiPhaseData[m_iDestructionMultiPhaseDataIndex];
	}

	//------------------------------------------------------------------------------------------------
	private int AllocateDestructionMultiPhaseData()
	{
		if (s_iFirstFreeDestructionMultiPhaseData == -1)
			return s_aDestructionMultiPhaseData.Insert(new SCR_DestructionMultiPhaseData());
		else
		{
			int returnIndex = s_iFirstFreeDestructionMultiPhaseData;
			SCR_DestructionMultiPhaseData data = s_aDestructionMultiPhaseData[returnIndex];
			s_iFirstFreeDestructionMultiPhaseData = data.m_iNextFreeIndex;
			data.m_iNextFreeIndex = -1;
			return returnIndex;
		}
	}

	//------------------------------------------------------------------------------------------------
	private void FreeDestructionMultiPhaseData(int index)
	{
		s_aDestructionMultiPhaseData[index].Reset();
		s_aDestructionMultiPhaseData[index].m_iNextFreeIndex = s_iFirstFreeDestructionMultiPhaseData;
		s_iFirstFreeDestructionMultiPhaseData = index;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns the input damage phase data (null = initial)
	SCR_DamagePhaseData GetDamagePhaseData(int damagePhase)
	{
		if (damagePhase <= 0 || damagePhase >= GetNumDamagePhases())
			return null;

		return SCR_DestructionMultiPhaseComponentClass.Cast(GetComponentData(GetOwner())).m_aDamagePhases.Get(damagePhase - 1);
	}

	//------------------------------------------------------------------------------------------------
	//! Returns whether the object should disable physics on destruction (before being deleted or changed)
	override bool GetDisablePhysicsOnDestroy()
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns if the object is in the initial damage phase
	bool GetIsInInitialDamagePhase()
	{
		return GetDamagePhase() == 0;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns which damage phase the object is currently in (0 = initial)
	int GetDamagePhase()
	{
		if (m_iDestructionMultiPhaseDataIndex == -1)
			return 0;

		return GetDestructionMultiPhaseData().GetDamagePhase();
	}

	//------------------------------------------------------------------------------------------------
	//! Returns the amount of damage phases (including initial)
	int GetNumDamagePhases()
	{
		SCR_DestructionMultiPhaseComponentClass componentData = SCR_DestructionMultiPhaseComponentClass.Cast(GetComponentData(GetOwner()));

		if (componentData.m_aDamagePhases)
			return componentData.m_aDamagePhases.Count() + 1;
		else
			return 1;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns the target damage stage for the input current health, current phase and damage
	int CalculateDamagePhase(int damagePhase, float health, float damage)
	{
		if (damage < 0)
			return damagePhase;

		while (damage >= 0)
		{
			damagePhase++;
			SCR_DamagePhaseData damagePhaseData = GetDamagePhaseData(damagePhase);
			if (!damagePhaseData)
				break;

			damage -= damagePhaseData.m_fPhaseHealth;
		}

		if (SCR_DestructionMultiPhaseComponentClass.Cast(GetComponentData(GetOwner())).m_bDeleteAfterFinalPhase)
			return Math.ClampInt(damagePhase, 0, GetNumDamagePhases()); // Clamp between initial and 1 past final phase
		else
			return Math.ClampInt(damagePhase, 0, GetNumDamagePhases() - 1);
	}

	//------------------------------------------------------------------------------------------------
	protected void SetOriginalResourceName(ResourceName originalResourceName)
	{
		GetDestructionMultiPhaseData().SetOriginalResourceName(originalResourceName);
	}

	//------------------------------------------------------------------------------------------------
	protected ResourceName GetOriginalResourceName()
	{
		if (m_iDestructionMultiPhaseDataIndex == -1)
			return ResourceName.Empty;

		return GetDestructionMultiPhaseData().GetOriginalResourceName();
	}

	//------------------------------------------------------------------------------------------------
	protected int GetTargetDamagePhase()
	{
		if (m_iDestructionMultiPhaseDataIndex == -1)
			return 0;

		return GetDestructionMultiPhaseData().GetTargetDamagePhase();
	}

	//------------------------------------------------------------------------------------------------
	protected void SetTargetDamagePhase(int targetDamagePhase)
	{
		GetDestructionMultiPhaseData().SetTargetDamagePhase(targetDamagePhase);
	}

	//------------------------------------------------------------------------------------------------
	protected void SetDamagePhase(int damagePhase)
	{
		GetDestructionMultiPhaseData().SetDamagePhase(damagePhase);

		SCR_DestructionUtility.SetDamagePhaseSignal(GetOwner(), damagePhase);
	}

	//------------------------------------------------------------------------------------------------
	//! Switches to the input damage phase (or deletes if past last phase)
	void GoToDamagePhase(int damagePhase, bool delayMeshChange)
	{
		if (damagePhase >= GetNumDamagePhases()) // Going past final phase, so delete if we can
		{
			if (GetDestructionBaseData())
				s_OnDestructibleDestroyed.Invoke(this);

			DeleteDestructibleDelayed();
			return;
		}

		int currentDamagePhase = GetDamagePhase();
		if (currentDamagePhase == damagePhase) // Same phase, ignore
			return;

		SCR_DestructionUtility.RegenerateNavmeshDelayed(GetOwner());

		if (currentDamagePhase == 0)
		{
			VObject vObject = GetOwner().GetVObject();
			if (vObject)
				SetOriginalResourceName(vObject.GetResourceName());
		}

		SCR_DestructionMultiPhaseComponentClass componentData = SCR_DestructionMultiPhaseComponentClass.Cast(GetComponentData(GetOwner()));
		if (!componentData)
			return;

		float combinedPhaseHealth = componentData.m_fBaseHealth;
		foreach (int i, SCR_DamagePhaseData phase : componentData.m_aDamagePhases)
		{
			if (i + 1 <= damagePhase)
				combinedPhaseHealth += phase.m_fPhaseHealth;
		}

		m_fNextPhaseHealth = m_iOriginalHealth - combinedPhaseHealth;

		SetDamagePhase(damagePhase);
		SetTargetDamagePhase(damagePhase + 1);

		ScriptCallQueue callQueue = GetGame().GetCallqueue();
		if (!callQueue)
			return;

		// Cancel any existing model change to ensure the latest phase takes priority
		float remainingTime = callQueue.GetRemainingTime(ChangeModel);
		if (remainingTime > -1)
			callQueue.Remove(ChangeModel);

		int delay;
		if (delayMeshChange)
			delay = componentData.m_fMeshChangeDelay;

		SCR_DamagePhaseData data = GetDamagePhaseData(damagePhase);
		if (!data)
			return;

		callQueue.CallLater(ChangeModel, delay, param1: data.m_PhaseModel, param2: data.m_bUseMaterialsFromParent);
	}

	//------------------------------------------------------------------------------------------------
	//! Changes entity model
	//! \param[in] modelName
	protected void ChangeModel(ResourceName modelName, bool useMaterialFromParent = false)
	{
		IEntity owner = GetOwner();
		if (!owner || owner.IsDeleted())
			return;

		ResourceName modelPath;
		string remap;
		SCR_Global.GetModelAndRemapFromResource(modelName, modelPath, remap);
		if(useMaterialFromParent)
		{
			ResourceName modelPathSource;
			string remapParent;
			ResourceName prefabPathSource = SCR_ResourceNameUtils.GetPrefabName(owner);
			SCR_Global.GetModelAndRemapFromResource(prefabPathSource, modelPathSource, remapParent);
			// Trying to remap slots which are not present in modelPath will result in error in Log Console. This is why we need to sanitize remap string to only contain valid entries.
			// GetModelAndRemapFromResource will return non empty remap only if prefabPath is linking to actual prefab with remaped materials.
			// Above behavior can be used to create a prefab, which remaps only 2 certain slots - those will be retrieved by code and used in sanitization process.
			if (remap)
			{
				remap = SCR_DestructionUtility.SanitizeRemapString(remap, remapParent);
			}
			else
			{
				remap = remapParent;
			}
		}

		SCR_DestructionUtility.SetModel(owner, modelPath, remap);
	}

	//------------------------------------------------------------------------------------------------
	//! Called when the object receives damage, return false if damage should be ignored
	override bool GetCanBeDamaged()
	{
		SCR_DestructionMultiPhaseComponentClass componentData = SCR_DestructionMultiPhaseComponentClass.Cast(GetComponentData(GetOwner()));
		if (!componentData)
			return true;

		if (componentData.m_bDeleteAfterFinalPhase)
			return GetDamagePhase() < GetNumDamagePhases();

		return GetDamagePhase() < GetNumDamagePhases() - 1;
	}

	//------------------------------------------------------------------------------------------------
	//! OnDamage
	override void OnDamage(notnull BaseDamageContext damageContext)
	{
		if (IsProxy())
			return;

		if (GetOwner().IsDeleted() || IsDestroyed())
			return;

		SCR_DestructionMultiPhaseComponentClass componentData = SCR_DestructionMultiPhaseComponentClass.Cast(GetComponentData(GetOwner()));

		float currentHealth = GetHealth();
		float previousHealth = GetDestructionBaseData().GetPreviousHealth();

		SCR_DestructionHitInfo hitInfo = CreateDestructionHitInfo(damageContext.damageValue >= componentData.m_fDamageThresholdMaximum, previousHealth, damageContext.damageValue, damageContext.damageType, damageContext.hitPosition, damageContext.hitDirection, damageContext.hitNormal);

		if (componentData.m_bPassDamageToChildren)
			PassDamageToChildren(damageContext);

		if (currentHealth <= 0)
		{
			int lastPhase = componentData.m_aDamagePhases.Count();

			if (hitInfo.m_TotalDestruction)
			{
				if (!componentData.m_bDeleteAfterFinalPhase && componentData.m_aDamagePhases && !componentData.m_aDamagePhases.IsEmpty() && !(componentData.m_bDestroyChildrenWhenDestroyed && GetOwner().GetParent()))
				{
					GoToDamagePhase(lastPhase, false);
					ReplicateDestructibleState(lastPhase, true);

					// Delete children without spawning effects
					if (componentData.m_bDestroyChildrenWhenDestroyed)
						DeleteDestructibleChildrenDelayed(false);
				}
				else
				{
					//Don't replicate state as there is no particles or sound to be played
					DeleteDestructibleDelayed();
				}

				GetDestructionBaseData().DeleteHitInfo();
				return;
			}

			if (componentData.m_aDamagePhases && !componentData.m_aDamagePhases.IsEmpty()) // MPD has damage phases
			{
				SCR_DamagePhaseData lastPhaseData = GetDamagePhaseData(lastPhase);

				if (componentData.m_eMaterialSoundType > 0)
					SCR_DestructionUtility.PlaySound(GetOwner(), componentData.m_eMaterialSoundType, GetNumDamagePhases(), GetDamagePhase());

				if (lastPhaseData.m_PhaseDestroySpawnObjects && !lastPhaseData.m_PhaseDestroySpawnObjects.IsEmpty())
					SCR_DestructionUtility.SpawnDestroyObjects(GetOwner(), lastPhaseData.m_PhaseDestroySpawnObjects, hitInfo);
				else
					SCR_DestructionUtility.SpawnDestroyObjects(GetOwner(), componentData.m_DestroySpawnObjects, hitInfo);

				if (componentData.m_bDeleteAfterFinalPhase)
				{
					ReplicateDestructibleState(lastPhase);
					DeleteDestructibleDelayed();
				}
				else
				{
					GoToDamagePhase(lastPhase, false);
					ReplicateDestructibleState(lastPhase);
					
					if (componentData.m_bDestroyChildrenWhenDestroyed)
						DeleteDestructibleChildrenDelayed();
				}
			}
			else
			{
				if (componentData.m_eMaterialSoundType > 0)
					SCR_DestructionUtility.PlaySound(GetOwner(), componentData.m_eMaterialSoundType, GetNumDamagePhases(), GetDamagePhase());
				
				if ( !componentData.m_bDestroyParentWhenDestroyed || !GetOwner().GetParent().GetPhysics() )
					SCR_DestructionUtility.SpawnDestroyObjects(GetOwner(), componentData.m_DestroySpawnObjects, hitInfo);
				
				ReplicateDestructibleState();				
				DeleteDestructibleDelayed();
			}

			GetDestructionBaseData().DeleteHitInfo();
			return;
		}

		int targetDamagePhase = GetTargetDamagePhase();
		if (targetDamagePhase <= 0)
			return;

		if (currentHealth <= m_fNextPhaseHealth)
		{
			if (componentData.m_eMaterialSoundType > 0)
				SCR_DestructionUtility.PlaySound(GetOwner(), componentData.m_eMaterialSoundType, GetNumDamagePhases(), GetDamagePhase());

			SCR_DamagePhaseData targetDamagePhaseData = GetDamagePhaseData(targetDamagePhase);
			if (targetDamagePhaseData.m_PhaseDestroySpawnObjects && !targetDamagePhaseData.m_PhaseDestroySpawnObjects.IsEmpty())
				SCR_DestructionUtility.SpawnDestroyObjects(GetOwner(), targetDamagePhaseData.m_PhaseDestroySpawnObjects, hitInfo);
			else
				SCR_DestructionUtility.SpawnDestroyObjects(GetOwner(), componentData.m_DestroySpawnObjects, hitInfo);

			GoToDamagePhase(targetDamagePhase, true);
			ReplicateDestructibleState(targetDamagePhase);

			if (componentData.m_bDestroyChildrenWhenDestroyed)
				DeleteDestructibleChildrenDelayed();

			if (!componentData.m_bDeleteAfterFinalPhase && targetDamagePhase == componentData.m_aDamagePhases.Count()) // Reached last phase
			{
				SetHitZoneHealth(0);
			}
		}

		if (hitInfo)
			GetDestructionBaseData().DeleteHitInfo();
	}

	//------------------------------------------------------------------------------------------------
	//! MPD specific implementation
	void UpdateResponseIndex()
	{
		SCR_DestructionMultiPhaseComponentClass prefabData = SCR_DestructionMultiPhaseComponentClass.Cast(GetComponentData(GetOwner()));
		if (!prefabData || !prefabData.m_bDeleteAfterFinalPhase)
			return; // We don't want to drive through indestructible objects

		Physics physics = GetOwner().GetPhysics();
		if (!physics)
			return; // No physics, no need to update anything

		if (GetDamagePhase() + 1 < GetNumDamagePhases()) // Not the last phase - don't update the response index
			return;

		// Is in last phase and will be destroyed after it
		SCR_DestructionUtility.UpdateResponseIndex(physics, GetHealth(), GetMaxHealth());
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RPC_ReplicateMultiPhaseDestructionState(int phase, vector hitPosDirNorm[3], float damage, EDamageType damageType, bool silent)
	{
		SCR_DestructionMultiPhaseComponentClass componentData = SCR_DestructionMultiPhaseComponentClass.Cast(GetComponentData(GetOwner()));
		if (!componentData)
			return;

		SCR_DestructionHitInfo hitInfo = GetDestructionHitInfo(true);
		hitInfo.m_HitPosition = hitPosDirNorm[0];
		hitInfo.m_HitDirection = hitPosDirNorm[1];
		hitInfo.m_HitNormal = hitPosDirNorm[2];
		hitInfo.m_HitDamage = damage;
		hitInfo.m_DamageType = damageType;

		SCR_DamagePhaseData phaseData = GetDamagePhaseData(phase);
		if (phaseData)
		{
			if (!silent)
			{
				if (phaseData.m_PhaseDestroySpawnObjects && !phaseData.m_PhaseDestroySpawnObjects.IsEmpty())
					SCR_DestructionUtility.SpawnDestroyObjects(GetOwner(), phaseData.m_PhaseDestroySpawnObjects, hitInfo);
				else
					SCR_DestructionUtility.SpawnDestroyObjects(GetOwner(), componentData.m_DestroySpawnObjects, hitInfo);

				if (componentData.m_eMaterialSoundType > 0)
					SCR_DestructionUtility.PlaySound(GetOwner(), componentData.m_eMaterialSoundType, GetNumDamagePhases(), GetDamagePhase());
			}

			GoToDamagePhase(phase, true);
		}
		else
		{
			if (!silent)
			{
				SCR_DestructionUtility.SpawnDestroyObjects(GetOwner(), componentData.m_DestroySpawnObjects, hitInfo);
				if (componentData.m_eMaterialSoundType > 0)
					SCR_DestructionUtility.PlaySound(GetOwner(), componentData.m_eMaterialSoundType, GetNumDamagePhases(), GetDamagePhase());
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	override void ReplicateDestructibleState(int damagePhase = 0, bool silent = false)
	{
		SCR_DestructionHitInfo hitInfo = GetDestructionHitInfo();
		if (hitInfo)
		{
			vector hitPosDirNorm[3];
			hitPosDirNorm[0] = hitInfo.m_HitPosition;
			hitPosDirNorm[1] = hitInfo.m_HitDirection;
			hitPosDirNorm[2] = hitInfo.m_HitNormal;

			Rpc(RPC_ReplicateMultiPhaseDestructionState, damagePhase, hitPosDirNorm, hitInfo.m_HitDamage, hitInfo.m_DamageType, silent);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected override bool HasDataToReplicate()
	{
		if (GetDamagePhase() != 0)
			return true;

		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected override event bool OnRplSave(ScriptBitWriter writer)
	{
		super.OnRplSave(writer);

		bool writeDamagePhase = GetDamagePhase() != 0;
		writer.Write(writeDamagePhase, 1);

		if (writeDamagePhase)
			writer.Write(GetDamagePhase(), 8); // Write which damage phase we are in

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected override event bool OnRplLoad(ScriptBitReader reader)
	{
		super.OnRplLoad(reader);

		bool readDamagePhase;
		reader.Read(readDamagePhase, 1);

		if (!readDamagePhase)
			return true;

		int damagePhase;
		reader.Read(damagePhase, 8); // Read which damage phase we are meant to be in

		if (damagePhase != GetDamagePhase())
			GoToDamagePhase(damagePhase, false);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);

		if (IsProxy())
			return;

		SCR_DestructionMultiPhaseComponentClass componentData = SCR_DestructionMultiPhaseComponentClass.Cast(GetComponentData(GetOwner()));
		if (!componentData)
			return;

		if (!componentData.m_aDamagePhases || componentData.m_aDamagePhases.IsEmpty())
			return;

		m_iOriginalHealth = componentData.m_fBaseHealth;
		foreach (SCR_DamagePhaseData damagePhase : componentData.m_aDamagePhases)
		{
			m_iOriginalHealth += damagePhase.m_fPhaseHealth;
		}

		SetHitZoneHealth(m_iOriginalHealth);
		SetTargetDamagePhase(1);
		m_fNextPhaseHealth = m_iOriginalHealth - componentData.m_fBaseHealth;
	}
#endif
}
