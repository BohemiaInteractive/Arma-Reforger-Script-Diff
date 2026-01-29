[EntityEditorProps(category: "GameScripted/Campaign", description: "Allow respawning at this vehicle in Campaign", color: "0 0 255 255")]
class SCR_CampaignMobileAssemblyComponentClass : ScriptComponentClass
{
	[Attribute("{6D282026AB95FC81}Prefabs/MP/Campaign/CampaignMobileAssemblySpawnpoint.et", UIWidgets.ResourceNamePicker, "", "et")]
	protected ResourceName m_sSpawnpointPrefab;

	//------------------------------------------------------------------------------------------------
	//! \return
	ResourceName GetSpawnpointPrefab()
	{
		return m_sSpawnpointPrefab;
	}
}

class SCR_CampaignMobileAssemblyComponent : ScriptComponent
{
	static const float MAX_WATER_DEPTH = 2.5;
	
	protected RplComponent m_RplComponent;
	
	protected SCR_CampaignFaction m_ParentFaction;
	
	protected bool m_bIsInRadioRange;

	protected SCR_SpawnPoint m_SpawnPoint;
	
	protected SCR_CampaignMobileAssemblyStandaloneComponent m_StandaloneComponent;

	[RplProp(onRplName: "OnParentFactionIDSet")]
	protected int m_iParentFaction = SCR_CampaignMilitaryBaseComponent.INVALID_FACTION_INDEX;
	
	[RplProp(onRplName: "OnSpawnpointCreated")]
	protected int m_iSpawnpointId = RplId.Invalid();
	
	[RplProp(onRplName: "OnDeployChanged")]
	protected bool m_bIsDeployed;
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		if (!GetGame().InPlayMode())
			return;
		
		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
		
		if (!campaign)
			return;
		
		super.OnPostInit(owner);
		
		m_RplComponent = RplComponent.Cast(owner.FindComponent(RplComponent));
		
		GetGame().GetCallqueue().CallLater(UpdateRadioCoverage, 1000, true);
		
		if (IsProxy())
		{
			campaign.GetInstance().GetOnFactionAssignedLocalPlayer().Insert(OnParentFactionIDSet);
			campaign.GetInstance().GetOnFactionAssignedLocalPlayer().Insert(OnDeployChanged);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnDelete(IEntity owner)
	{
		super.OnDelete(owner);
		
		GetGame().GetCallqueue().Remove(UpdateRadioCoverage);
		
		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
		
		if (!campaign)
			return;
		
		campaign.GetInstance().GetOnFactionAssignedLocalPlayer().Remove(OnParentFactionIDSet);
		campaign.GetInstance().GetOnFactionAssignedLocalPlayer().Remove(OnDeployChanged);
	}

	//------------------------------------------------------------------------------------------------	
	protected bool IsProxy()
	{
		return (m_RplComponent && m_RplComponent.IsProxy());
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] factionID
	void SetParentFactionID(int factionID)
	{
		m_iParentFaction = factionID;
		Replication.BumpMe();
		OnParentFactionIDSet();
	}
	
	//------------------------------------------------------------------------------------------------
	void OnParentFactionIDSet()
	{
		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
		if (!campaign)
			return;
		
		SCR_CampaignFactionManager fManager = SCR_CampaignFactionManager.Cast(GetGame().GetFactionManager());
		m_ParentFaction = SCR_CampaignFaction.Cast(fManager.GetFactionByIndex(m_iParentFaction));
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	int GetParentFactionID()
	{
		return m_iParentFaction;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	SCR_CampaignFaction GetParentFaction()
	{
		return m_ParentFaction;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	int GetRadioRange()
	{
		IEntity truck = GetOwner().GetParent();
		
		if (!truck)
			return 0;
		
		BaseRadioComponent comp = BaseRadioComponent.Cast(truck.FindComponent(BaseRadioComponent));
		
		if (!comp)
			return 0;
		
		BaseTransceiver tsv = comp.GetTransceiver(0);
		
		if (!tsv)
			return 0;
		
		return tsv.GetRange();
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] status
	//! \param[in] playerId
	//! \return
	bool Deploy(SCR_EMobileAssemblyStatus status, int playerId = 0, bool silent = false)
	{
		if (!m_bIsInRadioRange && status == SCR_EMobileAssemblyStatus.DEPLOYED)
			return false;
		
		if (!m_ParentFaction)
			return false;
		
		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
		if (!campaign)
			return false;
		
		if (m_ParentFaction.GetMobileAssembly() && status == SCR_EMobileAssemblyStatus.DEPLOYED)
			return false;
		
		if (status == SCR_EMobileAssemblyStatus.DEPLOYED)
		{
			CreateSpawnpoint();
			m_bIsDeployed = true;
		}
		else
		{
			m_bIsDeployed = false;
			
			if (m_SpawnPoint)
				RplComponent.DeleteRplEntity(m_SpawnPoint, false);
		}
		
		OnDeployChanged();
		Replication.BumpMe();
		
		if (!silent)
			campaign.BroadcastMHQFeedback(status, playerId, GetGame().GetFactionManager().GetFactionIndex(m_ParentFaction));

		return true;
	}

	//------------------------------------------------------------------------------------------------
	void OnDeployChanged()
	{
		// No need for SFX on headless
		if (!System.IsConsoleApp())
		{
			SCR_VehicleSoundComponent sndComp = SCR_VehicleSoundComponent.Cast(GetOwner().GetRootParent().FindComponent(SCR_VehicleSoundComponent));
			
			if (sndComp)
			{
				if (m_bIsDeployed)
					sndComp.SoundEvent(SCR_SoundEvent.SOUND_MHQ_DEPLOY);
				else
					sndComp.SoundEvent(SCR_SoundEvent.SOUND_MHQ_DISMANTLE);
			}
		}

		if (IsProxy())
			return;
		
		if (m_bIsDeployed)
			GetGame().GetCallqueue().CallLater(CheckStatus, 500, true);
		else
			GetGame().GetCallqueue().Remove(CheckStatus);

		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
		if (!campaign)
			return;

		SCR_CampaignMilitaryBaseManager baseManager = campaign.GetBaseManager();
		if (!baseManager)
			return;

		baseManager.DelayedEvaluateControlPoints(0);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	void CheckStatus()
	{
		IEntity truck = GetOwner().GetParent();
		
		if (!truck)
			return;
		
		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
			
		if (!campaign)
			return;
		
		DamageManagerComponent damageComponent = DamageManagerComponent.Cast(truck.FindComponent(DamageManagerComponent));
		
		// Destroyed?
		if (damageComponent && damageComponent.GetState() == EDamageState.DESTROYED)
		{
			Deploy(SCR_EMobileAssemblyStatus.DESTROYED);
			
			return;
		}
		
		// Moved?
		Physics physicsComponent = truck.GetPhysics();
		
		if (!physicsComponent)
			return;
		
		vector vel = physicsComponent.GetVelocity();
		vel[1] = 0;
		
		if (vel.LengthSq() <= 0.01)
			return;
		
		Deploy(SCR_EMobileAssemblyStatus.DISMANTLED);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnSpawnpointCreated()
	{
		// Delay so spawnpoint has time to be streamed in for clients
		GetGame().GetCallqueue().CallLater(RegisterSpawnpoint, 1000);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	void RegisterSpawnpoint()
	{
		// On server the assignment is done in CreateSpawnpoint()
		if (!IsProxy())
			return;
		
		m_SpawnPoint = SCR_SpawnPoint.Cast(Replication.FindItem(m_iSpawnpointId));
		
		if (!m_SpawnPoint)
			return;
		
		m_StandaloneComponent = SCR_CampaignMobileAssemblyStandaloneComponent.Cast(m_SpawnPoint.FindComponent(SCR_CampaignMobileAssemblyStandaloneComponent));
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	SCR_SpawnPoint GetSpawnPoint()
	{
		return m_SpawnPoint;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	SCR_CampaignMobileAssemblyStandaloneComponent GetStandaloneComponent()
	{
		return m_StandaloneComponent;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	bool IsDeployed()
	{
		return m_bIsDeployed;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	bool IsInRadioRange()
	{
		return m_bIsInRadioRange;
	}

	//------------------------------------------------------------------------------------------------
	//!
	void UpdateRadioCoverage()
	{
		if (!m_ParentFaction)
			return;

		m_bIsInRadioRange = SCR_GameModeCampaign.GetInstance().GetBaseManager().IsEntityInFactionRadioSignal(GetOwner(), m_ParentFaction);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	void CreateSpawnpoint()
	{
		SCR_CampaignMobileAssemblyComponentClass componentData = SCR_CampaignMobileAssemblyComponentClass.Cast(GetComponentData(GetOwner()));

		if (!componentData)
			return;
		
		Resource spawnpointResource = Resource.Load(componentData.GetSpawnpointPrefab());
		
		if (!spawnpointResource || !spawnpointResource.IsValid())
			return;
		
		EntitySpawnParams params = EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		GetOwner().GetTransform(params.Transform);
		m_SpawnPoint = SCR_SpawnPoint.Cast(GetGame().SpawnEntityPrefab(spawnpointResource, null, params));
		
		if (!m_SpawnPoint)
			return;
		
		m_StandaloneComponent = SCR_CampaignMobileAssemblyStandaloneComponent.Cast(m_SpawnPoint.FindComponent(SCR_CampaignMobileAssemblyStandaloneComponent));

		if (m_StandaloneComponent)
		{
			m_StandaloneComponent.SetRadioRange(GetRadioRange());
			m_StandaloneComponent.SetVehicle(SCR_EntityHelper.GetMainParent(GetOwner(), true));
			m_StandaloneComponent.SetParentFactionID(m_iParentFaction);
		}
		
		m_iSpawnpointId = Replication.FindId(m_SpawnPoint);
		OnSpawnpointCreated();
		Replication.BumpMe();
	}
	
	//------------------------------------------------------------------------------------------------
	// destructor
	void ~SCR_CampaignMobileAssemblyComponent()
	{
		if (Replication.IsClient())
			return;
		
		if (m_SpawnPoint)
			RplComponent.DeleteRplEntity(m_SpawnPoint, false);
		
		if (!m_ParentFaction || !IsDeployed())
			return;
		
		Deploy(SCR_EMobileAssemblyStatus.DISMANTLED);
	}
}

enum SCR_EMobileAssemblyStatus
{
	DEPLOYED,
	DISMANTLED,
	DESTROYED
}
