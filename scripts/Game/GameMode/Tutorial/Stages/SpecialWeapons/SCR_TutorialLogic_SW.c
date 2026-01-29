[BaseContainerProps()]
class SCR_TutorialLogic_SW : SCR_BaseTutorialCourseLogic
{
	ref ScriptInvoker m_OnAmmoDepletedChanged, m_OnAmmoAddedChanged;
	ref array <SCR_PlaceableInventoryItemComponent> m_aPlacedCharges;
	ref array <IEntity> m_aDestructables;
	
	int m_iMisses_M72;
	bool m_bRPGVoiceLinePlayed;
	protected vector m_vCheckOffset = "0 0.5 0";
	
	//------------------------------------------------------------------------------------------------
	override void OnCourseStart()
	{
		SCR_TutorialGamemodeComponent tutorialComponent = SCR_TutorialGamemodeComponent.Cast(GetGame().GetGameMode().FindComponent(SCR_TutorialGamemodeComponent));
		if (!tutorialComponent)
			return;
		
		//respawn TARGETS
		array <IEntity> targets = {};
		targets.Insert(tutorialComponent.SpawnAsset("TARGET_1", "{94803C603CF5651B}Prefabs/Structures/Military/Training/TargetVehicles/TargetBTR_flank.et"));
		targets.Insert(tutorialComponent.SpawnAsset("TARGET_2", "{E1EDC3CC5421436F}Prefabs/Structures/Military/Training/TargetVehicles/TargetBMP_flank.et"));
		targets.Insert(tutorialComponent.SpawnAsset("TARGET_3", "{74EF3F7B90C793AF}Prefabs/Structures/Military/Training/TargetVehicles/TargetUral_flank.et"));
		targets.Insert(tutorialComponent.SpawnAsset("TARGET_4", "{DFD494A96CA95D0B}Prefabs/Structures/Military/Training/TargetVehicles/TargetBTR_front.et"));
		
		SCR_DestructionMultiPhaseComponent destructionComp;
		foreach (IEntity target : targets)
		{
			destructionComp = SCR_DestructionMultiPhaseComponent.Cast(target.GetChildren().FindComponent(SCR_DestructionMultiPhaseComponent));
			if (!destructionComp)
				continue;
			
			destructionComp.EnableDamageHandling(false);
		}
		
		IEntity playerEnt = tutorialComponent.GetPlayer();
		if (playerEnt)
		{
			EventHandlerManagerComponent eventHandlerManager = EventHandlerManagerComponent.Cast(playerEnt.FindComponent(EventHandlerManagerComponent));
			if (eventHandlerManager)
			{
				eventHandlerManager.RegisterScriptHandler("OnProjectileShot", playerEnt, OnPlayerWeaponFired);
				eventHandlerManager.RegisterScriptHandler("OnAmmoCountChanged", playerEnt, OnPlayerAmmoChangeCallback);
			}
		}
		
		if (!m_OnAmmoDepletedChanged)
			m_OnAmmoDepletedChanged = new ScriptInvoker;
		
		if (!m_OnAmmoAddedChanged)
			m_OnAmmoAddedChanged = new ScriptInvoker;
		
		// SETUP Range trigger
		SCR_BaseTriggerEntity rangeTrigger = SCR_BaseTriggerEntity.Cast(GetGame().GetWorld().FindEntityByName("SW_TRIGGER"));
		if (rangeTrigger)
		{
			rangeTrigger.EnablePeriodicQueries(true);
			rangeTrigger.GetOnActivate().Insert(OnCharacterInRange);
			rangeTrigger.SetUpdateRate(1);
		}
		
		// Register destructables for charge setting
		RegisterDestructables();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void RegisterDestructables()
	{
		IEntity ent = GetGame().GetWorld().FindEntityByName("DESTRUCTABLE_OBSTACLE");
		if (!ent)
			return;
		
		SCR_DestructionMultiPhaseComponent destruction;
		ent = ent.GetChildren();
		while (ent)
		{
			destruction = SCR_DestructionMultiPhaseComponent.Cast(ent.FindComponent(SCR_DestructionMultiPhaseComponent));
			if (destruction)
				destruction.EnableDamageHandling(false);
			
			if (ent.FindComponent(DynamicPhysicsObstacleComponent))
			{
				if (!m_aDestructables)
					m_aDestructables = {};
								
				m_aDestructables.Insert(ent);
			}
			
			ent = ent.GetSibling();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	IEntity GetAttachedTo(SCR_PlaceableInventoryItemComponent charge)
	{	
		if (!m_aDestructables)
			return null;
		
		IEntity owner = charge.GetOwner();
		IEntity parent = owner.GetParent();
		IEntity mainParent = SCR_EntityHelper.GetMainParent(owner, true);
		if (mainParent && mainParent.IsInherited(ChimeraCharacter))
			return null;
		
		foreach (IEntity destructable : m_aDestructables)
		{
			if (parent == destructable)
				return destructable;
			
			if (vector.Distance(owner.GetOrigin(), destructable.GetOrigin() + m_vCheckOffset) <= 1.5)
				return destructable;
		}
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	void CheckPlacedCharges()
	{	
		if (!m_aPlacedCharges)
			m_aPlacedCharges = {};
		
		SCR_TutorialGamemodeComponent tutorial = SCR_TutorialGamemodeComponent.GetInstance();
		if (!tutorial)
			return;
		
		if (IsBarricadeDestroyed())
		{
			SCR_HintManagerComponent.HideHint();
			SCR_HintManagerComponent.ClearLatestHint();
			
			tutorial.SetStage("SW_END");
			GetGame().GetCallqueue().Remove(CheckPlacedCharges);
			return;
		}
		
		SCR_BaseTutorialStage stage = tutorial.GetCurrentStage();
		
		IEntity attachedTo;
		for (int i = m_aPlacedCharges.Count()-1; i >= 0; i--)
		{	
			if (!m_aPlacedCharges[i])
			{
				m_aPlacedCharges.Remove(i);
				continue;
			}
			
			attachedTo = GetAttachedTo(m_aPlacedCharges[i]);
			if (attachedTo)
				continue;
			
			m_aPlacedCharges.Remove(i);
		}
		
		if (m_aPlacedCharges.Count() >= m_aDestructables.Count())
		{
			if (stage && stage.Type() == SCR_Tutorial_SW_PLACE_CHARGES)
				stage.FinishStage();
			
			return;
		}
		
		if (stage.IsInherited(SCR_Tutorial_SW_PLACE_CHARGES))
		{
			foreach (IEntity destructable : m_aDestructables)
			{
				if (!destructable)
					continue;
				
				attachedTo = null;
				foreach (SCR_PlaceableInventoryItemComponent charge : m_aPlacedCharges)
				{
					attachedTo = GetAttachedTo(charge);
					
					if (attachedTo == destructable)
						break;
					
					attachedTo = null;
				}
				
				if (!attachedTo)
					stage.RegisterWaypoint(destructable, "", "CUSTOM");
			}
		}
		else
		{
			tutorial.InsertStage("PLACE_CHARGES", tutorial.GetActiveStageIndex());
			tutorial.SetStage("PLACE_CHARGES");
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnCharacterInRange(IEntity char)
	{
		SCR_TutorialGamemodeComponent tutorialComponent = SCR_TutorialGamemodeComponent.Cast(GetGame().GetGameMode().FindComponent(SCR_TutorialGamemodeComponent));
		if (!tutorialComponent)
			return;
		
		tutorialComponent.InsertStage("PlayerInRange", tutorialComponent.GetActiveStageIndex());
		tutorialComponent.SetStage(tutorialComponent.GetStageIndexByName("PlayerInRange"));
	}

	//------------------------------------------------------------------------------------------------
	//TODO: Clean this up, as it is quite dirty
	protected void OnPlayerWeaponFired(int playerID, BaseWeaponComponent weapon, IEntity entity)
	{	
		if (!weapon || weapon.GetWeaponType() != EWeaponType.WT_ROCKETLAUNCHER)
			return;
		
		if (weapon.GetCurrentMuzzle().GetAmmoCount() == 0)
			m_OnAmmoDepletedChanged.Invoke();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnPlayerAmmoChangeCallback(BaseWeaponComponent weapon, BaseMuzzleComponent muzzle, BaseMagazineComponent magazine, int ammoCount, bool isBarrelChambered)
	{	
		//TODO> Check for M72 and RPG
		if (!weapon || !weapon.IsReloadPossible())
			return;
		
		if (m_OnAmmoAddedChanged && ammoCount != 0)
			m_OnAmmoAddedChanged.Invoke();
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsBarricadeDestroyed()
	{
		if (!m_aDestructables)
			return true;
		
		for (int i = m_aDestructables.Count()-1; i >= 0; i--)
		{
			if (!m_aDestructables[i])
				m_aDestructables.Remove(i);
		}
		
		return m_aDestructables.IsEmpty();
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnCourseEnd()
	{
		GetGame().GetCallqueue().Remove(CheckPlacedCharges);
		m_aPlacedCharges = null;
		m_aDestructables = null;
		m_bRPGVoiceLinePlayed = false;
		
		SCR_TutorialGamemodeComponent tutorialComponent = SCR_TutorialGamemodeComponent.Cast(GetGame().GetGameMode().FindComponent(SCR_TutorialGamemodeComponent));
		if (!tutorialComponent)
			return;
		
		tutorialComponent.EnableArsenal("SW_ARSENAL_GRENADES", false);
		tutorialComponent.EnableArsenal("SW_ARSENAL_USSR", false);
		tutorialComponent.EnableArsenal("SW_ARSENAL_M72", false);
		tutorialComponent.EnableArsenal("SW_ARSENAL_AMMO", false);
		tutorialComponent.EnableArsenal("SW_ARSENAL_EXPLOSIVES", false);
		
		IEntity playerEnt = tutorialComponent.GetPlayer();
		if (playerEnt)
		{
			EventHandlerManagerComponent eventHandlerManager = EventHandlerManagerComponent.Cast(playerEnt.FindComponent(EventHandlerManagerComponent));
			if (eventHandlerManager)
				eventHandlerManager.RemoveScriptHandler("OnAmmoCountChanged", playerEnt, OnPlayerAmmoChangeCallback);
		}
		
		m_OnAmmoDepletedChanged = null;
		m_OnAmmoAddedChanged = null;
		
		// SETUP Range trigger
		SCR_BaseTriggerEntity rangeTrigger = SCR_BaseTriggerEntity.Cast(GetGame().GetWorld().FindEntityByName("SW_TRIGGER"));
		if (!rangeTrigger)
			return;
		
		rangeTrigger.EnablePeriodicQueries(false);
		rangeTrigger.GetOnActivate().Clear();
	}
}