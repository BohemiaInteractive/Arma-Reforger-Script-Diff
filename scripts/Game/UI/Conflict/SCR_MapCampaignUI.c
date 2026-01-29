void BaseHovered(SCR_CampaignMilitaryBaseComponent base);
typedef func BaseHovered;
typedef ScriptInvokerBase<BaseHovered> BaseHoveredInvoker;

//------------------------------------------------------------------------------
class SCR_MapCampaignUI : SCR_MapUIElementContainer
{
	[Attribute("{E48F14D6D3C54BE5}UI/layouts/Campaign/BaseElement.layout", params: "layout")]
	protected ResourceName m_sBaseElement;

	[Attribute("{94F1E2223D7E0588}UI/layouts/Campaign/ServiceHint.layout", params: "layout")]
	protected ResourceName m_sServiceHint;

	[Attribute("{FD71287E68006A26}UI/layouts/Campaign/CampaignPlayerMapIndicator.layout", params: "layout")]
	protected ResourceName m_sSpawnPositionHint;
	
	protected static const int TASK_ICON_Y_OFFSET = 10;

	protected Widget m_wMobileAssembly;
	protected ref ScriptInvokerVoid m_OnBasesInited;
	protected ref BaseHoveredInvoker m_OnBaseHovered;
	protected SCR_CampaignMilitaryBaseComponent m_HoveredBase;
	
	//------------------------------------------------------------------------------
	void InitMobileAssembly(string factionKey, bool deployed)
	{
		if (m_wMobileAssembly)
		{
			m_wMobileAssembly.SetVisible(deployed);
			return;
		}

		FactionManager fm = GetGame().GetFactionManager();
		if (!fm)
			return;

		SCR_CampaignFaction faction = SCR_CampaignFaction.Cast(fm.GetFactionByKey(factionKey));
		if (!faction)
			return;
		
		SCR_CampaignFaction playerFaction = SCR_CampaignFaction.Cast(SCR_FactionManager.SGetLocalPlayerFaction());
		if (faction != playerFaction)
			return;
		
		SCR_CampaignMobileAssemblyStandaloneComponent assembly = faction.GetMobileAssembly();
		if (!assembly)
			return;

		Widget w = GetGame().GetWorkspace().CreateWidgets(m_sBaseElement, m_wIconsContainer);
		if (!w)
			return;

		m_wMobileAssembly = w;

		SCR_CampaignMapUIBase handler = SCR_CampaignMapUIBase.Cast(w.FindHandler(SCR_CampaignMapUIBase));
		if (!handler)
			return;

		handler.SetParent(this);
		handler.InitMobile(assembly);
		m_mIcons.Set(w, handler);

		FrameSlot.SetSizeToContent(w, true);
		FrameSlot.SetAlignment(w, 0.5, 0.5);
	}
	
	//------------------------------------------------------------------------------
	protected void InitBases()
	{
		SCR_MilitaryBaseSystem baseManager = SCR_MilitaryBaseSystem.GetInstance();
		array<SCR_MilitaryBaseComponent> bases = {};
		baseManager.GetBases(bases);
		SCR_CampaignFaction faction = SCR_CampaignFaction.Cast(SCR_FactionManager.SGetLocalPlayerFaction());
		
		if (!faction)
			return;
		
		SCR_CampaignMilitaryBaseComponent base;
		
		for (int i = 0, count = bases.Count(); i < count; ++i)
		{
			base = SCR_CampaignMilitaryBaseComponent.Cast(bases[i]);
			
			if (!base || !base.IsInitialized())
				continue;
			
			// Don't display enemy HQs and established bases which are out of radio range
			if (base.GetFaction() != faction && (base.IsHQ() || (base.GetBuiltByPlayers() && !base.IsHQRadioTrafficPossible(faction))))
				continue;

			Widget w = GetGame().GetWorkspace().CreateWidgets(m_sBaseElement, m_wIconsContainer);
			SCR_CampaignMapUIBase handler = SCR_CampaignMapUIBase.Cast(w.FindHandler(SCR_CampaignMapUIBase));
			
			if (!handler)
				return;

			handler.SetParent(this);
			handler.InitBase(base);
			m_mIcons.Set(w, handler);
			base.SetBaseUI(handler);

			FrameSlot.SetSizeToContent(w, true);
			FrameSlot.SetAlignment(w, 0.5, 0.5);
		}

		if (faction)
		{
			string factionKey = faction.GetFactionKey();
			InitMobileAssembly(factionKey, faction.GetMobileAssembly() != null);
		}
		
		UpdateIcons();

		if (m_OnBasesInited)
			m_OnBasesInited.Invoke();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Offset task icon so it doesn't overlap with base icon
	override void UpdateIconPosition(Widget widget, SCR_MapUIElement icon, float x, float y)
	{	
		SCR_TaskMapUIComponent taskComponent = SCR_TaskMapUIComponent.Cast(icon);
		
		if (taskComponent)
		{
			SCR_CampaignMilitaryBaseTaskEntity task = SCR_CampaignMilitaryBaseTaskEntity.Cast(taskComponent.GetTask());
			
			if (task)
			{
				SCR_CampaignMilitaryBaseComponent base = task.GetMilitaryBase();
				
				if (base)
				{
					SCR_CampaignMapUIBase baseIcon = base.GetMapUI();
					
					if (baseIcon)
					{
						FrameSlot.SetPos(widget, x + (baseIcon.GetBaseIconSize() * 0.5), y + TASK_ICON_Y_OFFSET);
						return;
					}
				}
			}
		}

		super.UpdateIconPosition(widget, icon, x, y);
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void ShowSpawnPoint(notnull SCR_SpawnPoint spawnPoint)
	{
		if (!m_bIsDeployMap && spawnPoint.GetVisibleInDeployMapOnly())
			return;
		
		SCR_GameModeCampaign gameMode = SCR_GameModeCampaign.GetInstance();
		
		if (!gameMode)
			return;
		
		if (spawnPoint.Type() != SCR_CampaignSpawnPointGroup)
		{
			IEntity owner = spawnPoint.GetParent();
		
			// Mobile HQ icon is already shown
			SCR_CampaignMobileAssemblyStandaloneComponent westMHQ = gameMode.GetFactionByEnum(SCR_ECampaignFaction.BLUFOR).GetMobileAssembly();
			SCR_CampaignMobileAssemblyStandaloneComponent eastMHQ = gameMode.GetFactionByEnum(SCR_ECampaignFaction.OPFOR).GetMobileAssembly();
			
			if ((westMHQ && westMHQ.GetOwner() == spawnPoint) || (eastMHQ && eastMHQ.GetOwner() == spawnPoint))
				return;
			
			super.ShowSpawnPoint(spawnPoint); // when spawn point is placed by game master, use default spawn point visualization
			return;
		}
		
		// todo: hotfix for icon duplicates in conflict, figure out a proper solution :wink:

		if (spawnPoint.Type() == SCR_SpawnPoint || spawnPoint.Type() == SCR_CampaignSpawnPointGroup)
			return;

		Widget w = GetGame().GetWorkspace().CreateWidgets(m_sSpawnPointElement, m_wIconsContainer);
		SCR_MapUISpawnPoint handler = SCR_MapUISpawnPoint.Cast(w.FindHandler(SCR_MapUISpawnPoint));
		if (!handler)
			return;

		handler.Init(spawnPoint);
		handler.SetParent(this);
		m_mIcons.Set(w, handler);

		FrameSlot.SetSizeToContent(w, true);
		FrameSlot.SetAlignment(w, 0.5, 0.5);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void CreatePlayerSpawnPositionHint()
	{
		SCR_CampaignFeedbackComponent comp = SCR_CampaignFeedbackComponent.GetInstance();
		
		if (!comp)
			return;
		
		if (!comp.CanShowPlayerSpawn())
			return;
		
		comp.SetMapCampaignUI(this);
		
		vector playerLocation = comp.GetPlayerSpawnPos();
		
		Widget indicator = GetGame().GetWorkspace().CreateWidgets(m_sSpawnPositionHint, m_wIconsContainer);
		
		SCR_CampaignMapUIPlayerHighlight highlightHandler = SCR_CampaignMapUIPlayerHighlight.Cast(indicator.FindHandler(SCR_CampaignMapUIPlayerHighlight));
		
		if (!highlightHandler)
			return;
		
		m_mIcons.Insert(indicator, highlightHandler);
		
		highlightHandler.SetParent(this);
		highlightHandler.SetPos(playerLocation);
		
		FrameSlot.SetPosX(indicator, playerLocation[0]);
		FrameSlot.SetPosY(indicator, playerLocation[1]);
		FrameSlot.SetSizeToContent(indicator, true);
		FrameSlot.SetAlignment(indicator, 0.5, 0.5);
		
		//Called only after spawn, to center map and zoom to player position.
		if (!comp.WasMapOpened())
		{			
			m_MapEntity.ZoomPanSmooth(1.5,playerLocation[0],playerLocation[2]);
			comp.SetMapOpened(true);
		}
		
		// Set image from ImageSet
		// TODO: check for good const usage
		const ResourceName imageSet = "{5E8F77F38C2B9B9F}UI/Imagesets/MilitarySymbol/ICO.imageset";
		ImageWidget image = ImageWidget.Cast(indicator.FindAnyWidget("Image"));
		if(image)
		{
			image.LoadImageFromSet(0, imageSet, "PlayerSpawnHint");
			image.SetColor(Color.FromInt(Color.ORANGE));
		}
		
		//Sets time to spawn icon
		RichTextWidget timeWidget = RichTextWidget.Cast(indicator.FindAnyWidget("TimeString"));
		TimeContainer timeContainer = comp.GetSpawnTime();
		if (timeContainer)
		{
			string hours = timeContainer.m_iHours.ToString();
    		string minutes = timeContainer.m_iMinutes.ToString();
		
			if (timeContainer.m_iHours < 10)
				hours = "0" + hours;
		
			if (timeContainer.m_iMinutes < 10)
				minutes = "0" + minutes;
		
			timeWidget.SetTextFormat("%1:%2",hours, minutes);
		}
		
		HideSpawnPositionHint(m_MapEntity.GetLayerIndex());
		m_MapEntity.GetOnLayerChanged().Insert(HideSpawnPositionHint);
	}
	
	//------------------------------------------------------------------------------------------------
	//!Called to hide Spawn Position hint if player is killed (or his entity disappear for some reason)
	void RemoveSpawnPositionHint()
	{
		SCR_CampaignMapUIPlayerHighlight highlightHandler;
		foreach (Widget w, SCR_MapUIElement handler: m_mIcons)
		{
			highlightHandler = SCR_CampaignMapUIPlayerHighlight.Cast(handler);
			if(highlightHandler)
			{
				w.RemoveFromHierarchy();
				m_mIcons.Remove(w);
				break;
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void HideSpawnPositionHint(int layer)
	{
		SCR_CampaignMapUIPlayerHighlight highlightHandler;
		foreach (Widget w, SCR_MapUIElement handler: m_mIcons)
		{
			highlightHandler = SCR_CampaignMapUIPlayerHighlight.Cast(handler);
			if(highlightHandler)
			{	
				if(layer > 1)
				{
					w.SetVisible(false);
					return;
				}
				w.SetVisible(true);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Get all bases elements
	int GetBases(out notnull array<SCR_CampaignMapUIBase> bases)
	{
		bases.Clear();
		int count;
		foreach (Widget w, SCR_MapUIElement e : m_mIcons)
		{
			SCR_CampaignMapUIBase base = SCR_CampaignMapUIBase.Cast(e);
			if (!base)
				continue;

			bases.Insert(base);
			count++;
		}

		return count;
	}

	//------------------------------------------------------------------------------------------------
	//! Sets provided base as hovered.
	//! \param[in] base
	void SetHoveredBase(SCR_CampaignMilitaryBaseComponent base)
	{
		if (m_HoveredBase == base)
			return;

		m_HoveredBase = base;
		if (m_OnBaseHovered)
			m_OnBaseHovered.Invoke(base);
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	BaseHoveredInvoker GetOnBaseHovered()
	{
		if (!m_OnBaseHovered)
			m_OnBaseHovered = new BaseHoveredInvoker();

		return m_OnBaseHovered;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	ScriptInvokerVoid GetOnBasesInited()
	{
		if (!m_OnBasesInited)
			m_OnBasesInited = new ScriptInvokerVoid();

		return m_OnBasesInited;
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnSpawnPointFactionChange(SCR_SpawnPoint spawnPoint)
	{
		if (spawnPoint.GetFactionKey() == m_PlyFactionAffilComp.GetAffiliatedFaction().GetFactionKey())
		{
			AddSpawnPoint(spawnPoint);		
		}
		else
		{
			if (spawnPoint.Type() != SCR_CampaignSpawnPointGroup) // when changing faction of gm placed spawn points, we want to remove them from the map
				RemoveSpawnPoint(spawnPoint);
		}
	}	

	//------------------------------------------------------------------------------------------------
	void OnMapZoom(float pixelPerUnit)
	{
		UpdateIcons();
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMapOpen(MapConfiguration config)
	{
		super.OnMapOpen(config);

		Widget serviceHint = GetGame().GetWorkspace().CreateWidgets(m_sServiceHint, SCR_MapEntity.GetMapInstance().GetMapMenuRoot());
		
		if (serviceHint)
		{
			FrameSlot.SetSizeToContent(serviceHint, true);
			serviceHint.SetName("ServiceHintRoot");
			serviceHint.SetVisible(false);
		}
		
		SCR_CampaignFeedbackComponent comp = SCR_CampaignFeedbackComponent.GetInstance();
		
		if (comp)
			comp.SetMapCampaignUI(this);

		InitBases();
		CreatePlayerSpawnPositionHint();
		
		SCR_MapEntity.GetOnMapPan().Insert(OnMapPan);
		SCR_MapEntity.GetOnMapZoom().Insert(OnMapZoom);
	}

	//------------------------------------------------------------------------------------------------
	override void OnMapClose(MapConfiguration config)
	{
		super.OnMapClose(config);
		
		SCR_MapEntity.GetOnMapPan().Remove(OnMapPan);
		SCR_MapEntity.GetOnMapZoom().Remove(OnMapZoom);
		
		delete m_wMobileAssembly;
		
		foreach (Widget w, SCR_MapUIElement i : m_mIcons)
		{
			w.RemoveFromHierarchy();
			delete w;
		}

		m_mIcons.Clear();
		m_MapEntity.GetOnLayerChanged().Remove(HideSpawnPositionHint);
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_MapCampaignUI()
	{
		delete m_wMobileAssembly;
		foreach (Widget w, SCR_MapUIElement i : m_mIcons)
		{
			delete w;
		}
	}
};
