[EntityEditorProps(category: "GameScripted/DeployableItems", description: "")]
class SCR_BaseDeployableInventoryItemComponentClass : ScriptComponentClass
{
	// Setup
	[Attribute(uiwidget: UIWidgets.ResourcePickerThumbnail, desc: "Prefab which will be spawned when this item is deployed", params: "et", category: "Setup")]
	protected ResourceName m_sReplacementPrefab;

	[Attribute(defvalue: "1", category: "General")]
	protected bool m_bEnableSounds;

	[Attribute(desc: "Should this entity be deleted when replacement prefab is destroyed.")]
	protected bool m_bDeleteWhenDestroyed;

	//------------------------------------------------------------------------------------------------
	ResourceName GetReplacementPrefab()
	{
		return m_sReplacementPrefab;
	}

	//------------------------------------------------------------------------------------------------
	bool IsSoundEnabled()
	{
		return m_bEnableSounds;
	}

	//------------------------------------------------------------------------------------------------
	bool IsDeletedWhenDestroyed()
	{
		return m_bDeleteWhenDestroyed;
	}
}

void DeployableStateChanged(bool newState, SCR_BaseDeployableInventoryItemComponent component);
typedef func DeployableStateChanged;
typedef ScriptInvokerBase<DeployableStateChanged> SCR_DeployableItemState;

//! Base class which all deployable inventory items inherit from
class SCR_BaseDeployableInventoryItemComponent : ScriptComponent
{
	[RplProp(onRplName: "OnRplDeployed")]
	protected bool m_bIsDeployed;

	protected IEntity m_ReplacementEntity;
	protected vector m_aOriginalTransform[4];

	protected RplComponent m_RplComponent;

	protected bool m_bIsDeploying;

	[RplProp()]
	protected int m_iItemOwnerID;

	protected ref SCR_DeployableItemState m_OnDeployedStateChanged;
	
	protected bool m_bWasDeployed;
	protected IEntity m_PreviousOwner;

	//------------------------------------------------------------------------------------------------
	void SetDeploying(bool isDeploying)
	{
		m_bIsDeploying = isDeploying;
	}

	//------------------------------------------------------------------------------------------------
	bool IsDeploying()
	{
		return m_bIsDeploying;
	}

	//------------------------------------------------------------------------------------------------
	SCR_DeployableItemState GetOnDeployedStateChanged()
	{
		if (!m_OnDeployedStateChanged)
			m_OnDeployedStateChanged = new SCR_DeployableItemState();

		return m_OnDeployedStateChanged;
	}

	//------------------------------------------------------------------------------------------------
	//! Callback method that is triggered when composition is destroyed
	void OnCompositionDestroyed(IEntity instigator)
	{
		Dismantle(instigator);

		SCR_BaseDeployableInventoryItemComponentClass data = SCR_BaseDeployableInventoryItemComponentClass.Cast(GetComponentData(GetOwner()));
		if (!data)
			return;

		if (data.IsDeletedWhenDestroyed())//delay entity deltion to ensure that rpc calls will be able to complete
			GetGame().GetCallqueue().CallLater(RplComponent.DeleteRplEntity, param1: GetOwner(), param2: false);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RPC_SetTransformBroadcast(vector transform[4])
	{
		GetOwner().SetTransform(transform);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Unreliable, RplRcver.Broadcast)]
	protected void RPC_PlaySoundOnDeployBroadcast(bool deploy)
	{
		const IEntity owner = GetOwner();
		
		SoundComponent soundComp = SoundComponent.Cast(owner.FindComponent(SoundComponent));
		if (soundComp)
		{
			if (deploy)
				soundComp.SoundEvent(SCR_SoundEvent.SOUND_DEPLOY);
			else
				soundComp.SoundEvent(SCR_SoundEvent.SOUND_UNDEPLOY);

			return;
		}

		if (deploy)
			SCR_SoundManagerModule.CreateAndPlayAudioSource(owner, SCR_SoundEvent.SOUND_DEPLOY);
		else
			SCR_SoundManagerModule.CreateAndPlayAudioSource(owner, SCR_SoundEvent.SOUND_UNDEPLOY);
	}

	//------------------------------------------------------------------------------------------------
	//! Gets called when deploy action is executed by player
	//! \param[in] userEntity
	void Deploy(IEntity userEntity = null, bool reload = false)
	{
		if (!m_RplComponent || m_RplComponent.IsProxy())
			return;

		if (m_bIsDeployed)
			return;
		
		// transform containing information about the position where replacement entity is going to be spawned
		vector transform[4];
		const IEntity owner = GetOwner();
		const World world = owner.GetWorld();
		const TraceParam param = new TraceParam();
		const array<IEntity> ignoredEntities = {owner, userEntity};
		param.ExcludeArray = ignoredEntities;
		param.LayerMask = EPhysicsLayerPresets.Projectile; // to prevent entites from being deployed on leaves
		
		// +10cm buffer to prevent it from being deployed inside other objects
		SCR_TerrainHelper.GetTerrainBasis(owner.GetOrigin() + vector.Up * 0.1, transform, world, false, param);

		owner.GetWorldTransform(m_aOriginalTransform);

		EntitySpawnParams params = new EntitySpawnParams();
		params.Transform = transform;
		params.TransformMode = ETransformMode.WORLD;

		SCR_BaseDeployableInventoryItemComponentClass data = SCR_BaseDeployableInventoryItemComponentClass.Cast(GetComponentData(owner));
		if (!data)
			return;

		Resource resource = Resource.Load(data.GetReplacementPrefab());
		if (!resource.IsValid())
			return;

		m_ReplacementEntity = GetGame().SpawnEntityPrefab(resource, world, params);
		if (!m_ReplacementEntity)
			return;

		SCR_DeployableInventoryItemReplacementComponent replacementComp = SCR_DeployableInventoryItemReplacementComponent.Cast(m_ReplacementEntity.FindComponent(SCR_DeployableInventoryItemReplacementComponent));
		if (!replacementComp)
			return;

		vector compositionTransform[4], combinedTransform[4];
		replacementComp.GetItemTransform(compositionTransform);
		Math3D.MatrixMultiply4(transform, compositionTransform, combinedTransform);

		RPC_SetTransformBroadcast(combinedTransform);
		Rpc(RPC_SetTransformBroadcast, combinedTransform);

		replacementComp.GetOnCompositionDestroyed().Insert(OnCompositionDestroyed);

		m_bIsDeployed = true;
		SetItemOwner(GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(userEntity));
		Replication.BumpMe();

		if (data.IsSoundEnabled() && !reload)
		{
			RPC_PlaySoundOnDeployBroadcast(m_bIsDeployed);
			Rpc(RPC_PlaySoundOnDeployBroadcast, m_bIsDeployed);
		}

		SCR_GarbageSystem garbageSystem = SCR_GarbageSystem.GetByEntityWorld(owner);
		if (garbageSystem)
			garbageSystem.Withdraw(owner);

		if (m_OnDeployedStateChanged)
			m_OnDeployedStateChanged.Invoke(m_bIsDeployed, this);
	}

	//------------------------------------------------------------------------------------------------
	//! Gets called when dismantle action is executed by player
	//! \param[in] userEntity
	void Dismantle(IEntity userEntity = null, bool reload = false)
	{
		if (!m_RplComponent || m_RplComponent.IsProxy())
			return;

		if (!m_bIsDeployed)
			return;

		if (m_ReplacementEntity)
		{
			RplComponent rplComp = RplComponent.Cast(m_ReplacementEntity.FindComponent(RplComponent));
			if (!rplComp)
				return;

			rplComp.DeleteRplEntity(m_ReplacementEntity, false);
		}

		RPC_SetTransformBroadcast(m_aOriginalTransform);
		Rpc(RPC_SetTransformBroadcast, m_aOriginalTransform);

		m_bIsDeployed = false;
		m_iItemOwnerID = 0;
		Replication.BumpMe();

		SCR_BaseDeployableInventoryItemComponentClass data = SCR_BaseDeployableInventoryItemComponentClass.Cast(GetComponentData(GetOwner()));
		if (data && data.IsSoundEnabled() && !reload)
		{
			RPC_PlaySoundOnDeployBroadcast(m_bIsDeployed);
			Rpc(RPC_PlaySoundOnDeployBroadcast, m_bIsDeployed);
		}

		SCR_GarbageSystem garbageSystem = SCR_GarbageSystem.GetByEntityWorld(GetOwner());
		if (garbageSystem)
			garbageSystem.Insert(GetOwner());

		if (m_OnDeployedStateChanged)
			m_OnDeployedStateChanged.Invoke(m_bIsDeployed, this);
	}

	//------------------------------------------------------------------------------------------------
	void SetItemOwner(int playerId)
	{
		m_iItemOwnerID = playerId;
	}

	//------------------------------------------------------------------------------------------------
	//! Dismantle and redeploy to update settings
	void Reload()
	{
		if (!m_RplComponent || m_RplComponent.IsProxy())
			return;

		if (!m_bIsDeployed)
			return;

		IEntity ownerEntity = GetGame().GetPlayerManager().GetPlayerControlledEntity(m_iItemOwnerID);
		Dismantle(null, true);
		Deploy(ownerEntity, true);
	}

	//------------------------------------------------------------------------------------------------
	//!
	protected void OnRplDeployed();

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] userEntity
	//! \return
	bool CanDeployBeShown(notnull IEntity userEntity)
	{
		return !m_bIsDeployed;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] userEntity
	//! \return
	bool CanDismantleBeShown(notnull IEntity userEntity)
	{
		return m_bIsDeployed;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \return
	bool IsDeployed()
	{
		return m_bIsDeployed;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	int GetItemOwnerID()
	{
		return m_iItemOwnerID;
	}

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		SetEventMask(GetOwner(), EntityEvent.INIT);
		m_RplComponent = RplComponent.Cast(GetOwner().FindComponent(RplComponent));
	}

	//------------------------------------------------------------------------------------------------
	override void OnDelete(IEntity owner)
	{
		super.OnDelete(owner);

		if (m_OnDeployedStateChanged)
			m_OnDeployedStateChanged.Invoke(false, this);

		if (!m_bIsDeployed || !m_RplComponent || m_RplComponent.IsProxy())
			return;

		if (!m_ReplacementEntity || m_ReplacementEntity.IsDeleted())
			return;

		RplComponent rplComp = RplComponent.Cast(m_ReplacementEntity.FindComponent(RplComponent));
		if (!rplComp)
			return;

		rplComp.DeleteRplEntity(m_ReplacementEntity, false);
	}
}
