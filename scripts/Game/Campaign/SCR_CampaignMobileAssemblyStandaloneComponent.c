class SCR_CampaignMobileAssemblyStandaloneComponentClass : ScriptComponentClass
{
}

class SCR_CampaignMobileAssemblyStandaloneComponent : ScriptComponent
{
	static ref ScriptInvokerVoid s_OnSpawnPointOwnerChanged = new ScriptInvokerVoid();
	static ref ScriptInvokerVoid s_OnUpdateRespawnCooldown = new ScriptInvokerVoid();

	static const int RESPAWN_COOLDOWN = 60000;

	protected RplComponent m_RplComponent;

	protected MapItem m_MapItem;

	protected IEntity m_Vehicle;

	protected SCR_CampaignFaction m_ParentFaction;
	protected SCR_CampaignFaction m_Faction;

	protected bool m_bIsInRadioRange;
	protected bool m_bCooldownDone = true;
	
	protected const EResourceType RESOURCE_TYPE = EResourceType.SUPPLIES;
	protected const EResourceGeneratorID RESOURCE_GENERATOR_ID = EResourceGeneratorID.DEFAULT;

	[RplProp(onRplName: "OnFactionChanged")]
	protected int m_iFaction = SCR_CampaignMilitaryBaseComponent.INVALID_FACTION_INDEX;

	[RplProp(onRplName: "OnParentFactionIDSet")]
	protected int m_iParentFaction = SCR_CampaignMilitaryBaseComponent.INVALID_FACTION_INDEX;

	[RplProp()]
	protected WorldTimestamp m_fRespawnAvailableSince;

	[RplProp()]
	protected int m_iRadioRange;

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
		GetGame().GetCallqueue().CallLater(UpdateRespawnCooldown, 250, true, null);

		if (IsProxy())
		{
			campaign.GetInstance().GetOnFactionAssignedLocalPlayer().Insert(OnParentFactionIDSet);
		}
		else
		{
			// Only update respawn cooldown if a load state has not been applied
			ChimeraWorld world = GetGame().GetWorld();
			if (world.GetWorldTime() > 10000)
			{
				m_fRespawnAvailableSince = world.GetServerTimestamp().PlusMilliseconds(RESPAWN_COOLDOWN);
				m_bCooldownDone = false;
			}
			else
			{
				m_fRespawnAvailableSince = world.GetServerTimestamp();
				m_bCooldownDone = true;
			}
			Replication.BumpMe();
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnDelete(IEntity owner)
	{
		super.OnDelete(owner);

		GetGame().GetCallqueue().Remove(UpdateRadioCoverage);
		GetGame().GetCallqueue().Remove(UpdateRespawnCooldown);

		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();

		if (!campaign)
			return;

		campaign.GetInstance().GetOnFactionAssignedLocalPlayer().Remove(OnParentFactionIDSet);
	}

	//------------------------------------------------------------------------------------------------
	protected bool IsProxy()
	{
		return (m_RplComponent && m_RplComponent.IsProxy());
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] vehicle
	void SetVehicle(IEntity vehicle)
	{
		m_Vehicle = vehicle;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	IEntity GetVehicle()
	{
		return m_Vehicle;
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
		SCR_CampaignFactionManager fManager = SCR_CampaignFactionManager.Cast(GetGame().GetFactionManager());
		m_ParentFaction = SCR_CampaignFaction.Cast(fManager.GetFactionByIndex(m_iParentFaction));

		if (!m_ParentFaction)
			return;
		
		SCR_CoverageRadioComponent radioComponent = SCR_CoverageRadioComponent.Cast(GetOwner().FindComponent(BaseRadioComponent));
				
		if (radioComponent)
			radioComponent.SetEncryptionKey(m_ParentFaction.GetFactionRadioEncryptionKey());

		m_ParentFaction.SetMobileAssembly(this);

		// Delay so map item can initialize
		if (!System.IsConsoleApp())
			GetGame().GetCallqueue().CallLater(UpdateMapItem, SCR_GameModeCampaign.MINIMUM_DELAY);

		SCR_RadioCoverageSystem.UpdateAll();
		UpdateRadioCoverage();
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
	SCR_CampaignFaction GetFaction()
	{
		return m_Faction;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] range
	void SetRadioRange(int range)
	{
		m_iRadioRange = range;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	int GetRadioRange()
	{
		return m_iRadioRange;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateMapItem()
	{
		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();

		if (!campaign)
			return;

		SCR_MapDescriptorComponent mapDescriptor = SCR_MapDescriptorComponent.Cast(GetOwner().FindComponent(SCR_MapDescriptorComponent));

		if (mapDescriptor)
			m_MapItem = mapDescriptor.Item();
		
		Faction playerFaction = SCR_FactionManager.SGetLocalPlayerFaction();

		if (!m_MapItem || m_ParentFaction != playerFaction)
			return;

		m_MapItem.SetVisible(true);

		switch (m_ParentFaction.GetFactionKey())
		{
			case campaign.GetFactionKeyByEnum(SCR_ECampaignFaction.BLUFOR):
			{
				m_MapItem.SetFactionIndex(EFactionMapID.WEST);
				break;
			}

			case campaign.GetFactionKeyByEnum(SCR_ECampaignFaction.OPFOR):
			{
				m_MapItem.SetFactionIndex(EFactionMapID.EAST);
				break;
			}

			default:
			{
				m_MapItem.SetFactionIndex(EFactionMapID.UNKNOWN);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	void OnFactionChanged()
	{
		SCR_CampaignFactionManager fManager = SCR_CampaignFactionManager.Cast(GetGame().GetFactionManager());

		if (!fManager)
			return;

		FactionKey fKey;
		Faction f = fManager.GetFactionByIndex(m_iFaction);

		if (f)
		{
			fKey = f.GetFactionKey();
			m_Faction = SCR_CampaignFaction.Cast(f);
		}

		SCR_SpawnPoint.Cast(GetOwner()).SetFaction(f);
		s_OnSpawnPointOwnerChanged.Invoke();
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	bool IsInRadioRange()
	{
		return m_bIsInRadioRange;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	MapItem GetMapItem()
	{
		return m_MapItem;
	}

	//------------------------------------------------------------------------------------------------
	//!
	void UpdateRadioCoverage()
	{
		if (!m_ParentFaction)
			return;

		bool inRangeNow = SCR_GameModeCampaign.GetInstance().GetBaseManager().IsEntityInFactionRadioSignal(GetOwner(), m_ParentFaction);
		m_bIsInRadioRange = inRangeNow;

		DamageManagerComponent damageComponent = DamageManagerComponent.Cast(GetOwner().FindComponent(DamageManagerComponent));

		if (damageComponent && damageComponent.GetState() == EDamageState.DESTROYED)
		{
			if (m_MapItem)
				m_MapItem.SetVisible(false);

			if (!IsProxy())
			{
				SCR_CoverageRadioComponent radioComponent = SCR_CoverageRadioComponent.Cast(GetOwner().FindComponent(BaseRadioComponent));
				
				if (radioComponent)
					radioComponent.SetPower(false);
			}
		}

		if (IsProxy())
			return;

		if (!m_bIsInRadioRange && m_iFaction != SCR_CampaignMilitaryBaseComponent.INVALID_FACTION_INDEX)
		{
			m_iFaction = SCR_CampaignMilitaryBaseComponent.INVALID_FACTION_INDEX;
			Replication.BumpMe();
			OnFactionChanged();
		}

		if (m_bIsInRadioRange && m_iFaction == SCR_CampaignMilitaryBaseComponent.INVALID_FACTION_INDEX && m_bCooldownDone)
		{
			m_iFaction = m_iParentFaction;
			Replication.BumpMe();
			OnFactionChanged();
		}
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] w
	void UpdateRespawnCooldown(Widget w = null)
	{
		if (!m_ParentFaction)
			return;

		SCR_CampaignMilitaryBaseComponent hq = m_ParentFaction.GetMainBase();

		TextWidget respawn;
		ImageWidget respawnImg;

		if (!hq)
			return;
		
		ResourceName imageset = hq.GetBuildingIconImageset();

		if (w && imageset)
		{
			respawn = TextWidget.Cast(w.FindAnyWidget("Respawn"));
			respawnImg = ImageWidget.Cast(w.FindAnyWidget("RespawnIMG"));
			respawnImg.LoadImageFromSet(0, imageset, "RespawnBig");
		}

		ChimeraWorld world = GetOwner().GetWorld();
		if (m_fRespawnAvailableSince.Greater(world.GetServerTimestamp()))
		{
			if (m_bCooldownDone)
				m_bCooldownDone = false;

			if (RplSession.Mode() != RplMode.Dedicated)
			{
				if (!w)
					s_OnUpdateRespawnCooldown.Invoke();
				else
				{
					respawn.SetVisible(true);
					respawnImg.SetVisible(true);

					float respawnCooldown = Math.Ceil(m_fRespawnAvailableSince.DiffMilliseconds(world.GetServerTimestamp()) * 0.001);
					int d, h, m, s;
					string sStr;
					SCR_DateTimeHelper.GetDayHourMinuteSecondFromSeconds(respawnCooldown, d, h, m, s);
					sStr = s.ToString();

					if (s < 10)
						sStr = "0" + sStr;

					respawn.SetTextFormat("#AR-Campaign_MobileAssemblyCooldown", m, sStr);
				}
			}
		}
		else
		{
			GetGame().GetCallqueue().Remove(UpdateRespawnCooldown);

			if (w)
			{
				if (RplSession.Mode() != RplMode.Dedicated)
				{
					respawn.SetVisible(false);
					respawnImg.SetVisible(false);
				}
			}
			else
			{
				if (!m_bCooldownDone)
				{
					m_bCooldownDone = true;
					s_OnUpdateRespawnCooldown.Invoke();

					if (!IsProxy() && IsInRadioRange())
					{
						m_iFaction = m_iParentFaction;
						Replication.BumpMe();
						OnFactionChanged();
					}
				}
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Sets the value of supplies in widget based on available supplies in Mobile Assembly
	//! \param[in] w
	void UpdateSuppliesAmount(Widget w = null)
	{
		if (!w || !m_ParentFaction)
			return;

		SCR_CampaignMilitaryBaseComponent hq = m_ParentFaction.GetMainBase();
		if (!hq)
			return;

		ResourceName imageset = hq.GetBuildingIconImageset();
		if (!imageset)
			return;

		TextWidget resource = TextWidget.Cast(w.FindAnyWidget("Supplies"));
		ImageWidget resourceImg = ImageWidget.Cast(w.FindAnyWidget("SuppliesIMG"));
		
		if (!resource || !resourceImg)
			return;

		SCR_ResourceComponent resourceComp = SCR_ResourceComponent.FindResourceComponent(GetOwner());
		if (!resourceComp)
			return;

		SCR_ResourceConsumer resourceConsumer;
		if (!resourceComp.GetConsumer(RESOURCE_GENERATOR_ID, RESOURCE_TYPE, resourceConsumer))
			return;

		float resourceAmount = resourceConsumer.GetAggregatedResourceValue();
		float maxResourceAmount = resourceConsumer.GetAggregatedMaxResourceValue();

		resource.SetTextFormat("#AR-Campaign_BaseSuppliesAmount", resourceAmount, maxResourceAmount);
		resourceImg.LoadImageFromSet(0, imageset, "SuppliesBig");

		resource.SetVisible(true);
		resourceImg.SetVisible(true);
	}
}
