[BaseContainerProps()]
class SCR_TutorialLogic_SeizingCourse : SCR_BaseTutorialCourseLogic
{
	protected const ResourceName COMMS_PREFAB = "{F7DC8BB193BCAF44}PrefabsEditable/Auto/Compositions/Slotted/SlotFlatSmall/E_Antenna_S_US_01.et";
	
	//------------------------------------------------------------------------------------------------
	protected void OnEntitySpawned(IEntity ent)
	{
		if (ent && ent.GetPrefabData().GetPrefabName() == COMMS_PREFAB)
			ent.SetName("BUILDING_ANTENNA");
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnCourseStart()
	{
		ResetRadio();
		
		SCR_EntityHelper.DeleteEntityAndChildren(GetGame().GetWorld().FindEntityByName("BUILDING_ANTENNA"));
		
		SCR_TutorialFakeBaseComponent fakeBaseComponent;
		IEntity fakeBase;
		
		//Show map icons
		fakeBase = GetGame().GetWorld().FindEntityByName("FakeBaseHospital");
		if (fakeBase)
		{
			fakeBaseComponent = SCR_TutorialFakeBaseComponent.Cast(fakeBase.FindComponent(SCR_TutorialFakeBaseComponent));
			if (fakeBaseComponent)
			{
				fakeBaseComponent.m_bVisible = true;
				fakeBaseComponent.CreateLinks();
			}
		}
		
		fakeBase = GetGame().GetWorld().FindEntityByName("FakeBaseHQ");
		if (fakeBase)
		{
			fakeBaseComponent = SCR_TutorialFakeBaseComponent.Cast(fakeBase.FindComponent(SCR_TutorialFakeBaseComponent));
			if (fakeBaseComponent)
			{
				fakeBaseComponent.m_bVisible = true;
				fakeBaseComponent.CreateLinks();
			}
		}
		
		fakeBase = GetGame().GetWorld().FindEntityByName("FakeBaseSignalHill");
		if (fakeBase)
		{
			fakeBaseComponent = SCR_TutorialFakeBaseComponent.Cast(fakeBase.FindComponent(SCR_TutorialFakeBaseComponent));
			if (fakeBaseComponent)
			{
				fakeBaseComponent.m_bVisible = true;
				fakeBaseComponent.CreateLinks();
			}
		}
		
		fakeBase = GetGame().GetWorld().FindEntityByName("FakeBaseHarbor");
		if (fakeBase)
		{
			fakeBaseComponent = SCR_TutorialFakeBaseComponent.Cast(fakeBase.FindComponent(SCR_TutorialFakeBaseComponent));
			if (fakeBaseComponent)
			{
				fakeBaseComponent.m_bVisible = true;
				fakeBaseComponent.CreateLinks();
			}
		}
		
		fakeBase = GetGame().GetWorld().FindEntityByName("FakeBaseArleville");
		if (fakeBase)
		{
			fakeBaseComponent = SCR_TutorialFakeBaseComponent.Cast(fakeBase.FindComponent(SCR_TutorialFakeBaseComponent));
			if (fakeBaseComponent)
			{
				fakeBaseComponent.m_bVisible = true;
				fakeBaseComponent.CreateLinks();
			}
		}
		
		fakeBase = GetGame().GetWorld().FindEntityByName("FakeBaseLighthouseNorth");
		if (fakeBase)
		{
			fakeBaseComponent = SCR_TutorialFakeBaseComponent.Cast(fakeBase.FindComponent(SCR_TutorialFakeBaseComponent));
			if (fakeBaseComponent)
			{
				fakeBaseComponent.m_bVisible = true;
				fakeBaseComponent.m_BaseColor = Color.FromRGBA(249, 210, 103, 255);
				fakeBaseComponent.m_eMilitarySymbolIdentity = EMilitarySymbolIdentity.UNKNOWN;
				fakeBaseComponent.m_sHighlight = "Unknown_Installation_Focus_Land";
			}
			
			SCR_FactionAffiliationComponent factionComp = SCR_FactionAffiliationComponent.Cast(fakeBase.FindComponent(SCR_FactionAffiliationComponent));
			if (factionComp)
				factionComp.SetAffiliatedFactionByKey("USSR");
			
			SCR_TutorialSeizingComponent seizingComp = SCR_TutorialSeizingComponent.Cast(fakeBase.FindComponent(SCR_TutorialSeizingComponent));
			if (seizingComp)
			{
				seizingComp.UpdateFlagsInHierarchyPublic();
				
			}
			
			BaseGameTriggerEntity trigger;
			IEntity child = fakeBase.GetChildren();
			while (child)
			{
				if (child.IsInherited(BaseGameTriggerEntity))
				{
					trigger = BaseGameTriggerEntity.Cast(child);
					break;
				}
				
				child = child.GetSibling();
			}
			
			if (trigger)
				trigger.SetSphereRadius(0.1);
		}
		
		//Fix targets
		SCR_TutorialGamemodeComponent tutorial = SCR_TutorialGamemodeComponent.GetInstance();
		if (tutorial)
		{
			tutorial.SetupTargets("base_target1", null, ETargetState.TARGET_DOWN, false);
			tutorial.GetOnEntitySpawned().Insert(OnEntitySpawned);
		}
		
		//Reset MHQ interaction
		IEntity mobilehq = GetGame().GetWorld().FindEntityByName("Tutorial_MobileHQ");
		if (!mobilehq)
			return;
		
		SlotManagerComponent slotMan = SlotManagerComponent.Cast(mobilehq.FindComponent(SlotManagerComponent));
		if (!slotMan)
			return;
		
		EntitySlotInfo slotInfo = slotMan.GetSlotByName("Radio");
		if (!slotInfo)
			return;
		
		IEntity box = slotInfo.GetAttachedEntity();
		if (!box)
			return;
		
		box.SetName("MHQ_TRUNK");
		
		ActionsManagerComponent actionMan = ActionsManagerComponent.Cast(box.FindComponent(ActionsManagerComponent));
		if (!actionMan)
			return;
		
		array <BaseUserAction> userActions = {};
		
		actionMan.GetActionsList(userActions);
		SCR_TutorialDeployMobileAssembly assemblyAction;
		foreach (BaseUserAction userAction : userActions)
		{
			if (!userAction.IsInherited(SCR_TutorialDeployMobileAssembly))
				continue;
			
			assemblyAction = SCR_TutorialDeployMobileAssembly.Cast(userAction);
			if (assemblyAction)
				assemblyAction.SetActive(false);
		}
		
		IEntity antenna = GetGame().GetWorld().FindEntityByName("Seizing_Course_Antenna");
		if (antenna)
			SCR_EntityHelper.DeleteEntityAndChildren(antenna);
		
		ClearSupplies();
	}
	
	//------------------------------------------------------------------------------------------------
	void ResetRadio()
	{
		SCR_TutorialGamemodeComponent tutorial = SCR_TutorialGamemodeComponent.GetInstance();
		if (!tutorial)
			return;
		
		IEntity radio = tutorial.FindPrefabInPlayerInventory("{73950FBA2D7DB5C5}Prefabs/Items/Equipment/Radios/Radio_ANPRC68.et");
		if (!radio)
			return;
		
		BaseRadioComponent radioComp = BaseRadioComponent.Cast(radio.FindComponent(BaseRadioComponent));
		if (!radioComp)
			return;
		
		RadioTransceiver transceiver = RadioTransceiver.Cast(radioComp.GetTransceiver(0));
		if (transceiver)
			transceiver.SetFrequency(32000);
		
		transceiver = RadioTransceiver.Cast(radioComp.GetTransceiver(1));
		if (transceiver)
			transceiver.SetFrequency(32000);
		
	}
	
	//------------------------------------------------------------------------------------------------
	void ClearSupplies()
	{
		IEntity ent = GetGame().GetWorld().FindEntityByName("LIGHTHOUSE_SUPPLIES");
		if (!ent)
			return;

		SCR_ResourceComponent resComp = SCR_ResourceComponent.FindResourceComponent(ent);
		if (!resComp)
			return;

		SCR_ResourceContainer resourceContainer = resComp.GetContainer(EResourceType.SUPPLIES);
		if (resourceContainer)
			resourceContainer.DepleteResourceValue();
	}
	
	//------------------------------------------------------------------------------------------------
	bool HasPlayerAnyFirearm()
	{
		SCR_TutorialGamemodeComponent tutorial = SCR_TutorialGamemodeComponent.GetInstance();
		if (!tutorial)
			return false;
		
		ChimeraCharacter player = tutorial.GetPlayer();
		if (!player)
			return false;
		
		BaseWeaponManagerComponent weaponManager = BaseWeaponManagerComponent.Cast(player.FindComponent(BaseWeaponManagerComponent));
		if (!weaponManager)
			return false;
		
		array<IEntity> weapons = {};
		weaponManager.GetWeaponsList(weapons);

		BaseWeaponComponent weaponComp;
		foreach (IEntity weapon : weapons)
		{
			weaponComp = BaseWeaponComponent.Cast(weapon.FindComponent(BaseWeaponComponent));
			if (!weaponComp)
				continue;
	
			if (weaponComp.GetWeaponType() == EWeaponType.WT_FRAGGRENADE || weaponComp.GetWeaponType() == EWeaponType.WT_SMOKEGRENADE || weaponComp.GetWeaponType() == EWeaponType.WT_NONE || weaponComp.GetWeaponType() == EWeaponType.WT_ROCKETLAUNCHER)
				continue;
	
			return true;
		}
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnCourseEnd()
	{
		SCR_TutorialGamemodeComponent tutorial = SCR_TutorialGamemodeComponent.GetInstance();
		if (tutorial)
			tutorial.GetOnEntitySpawned().Remove(OnEntitySpawned);
		
		SCR_TutorialFakeBaseComponent fakeBaseComponent;
		IEntity fakeBase;
		
		//hide map icons
		fakeBase = GetGame().GetWorld().FindEntityByName("FakeBaseFarm");
		if (fakeBase)
		{
			fakeBaseComponent = SCR_TutorialFakeBaseComponent.Cast(fakeBase.FindComponent(SCR_TutorialFakeBaseComponent));
			if (fakeBaseComponent)
			{
				fakeBaseComponent.m_bVisible = false;
				fakeBaseComponent.ClearLinks();
				fakeBaseComponent.RemoveLinkName("Tutorial_MobileHQ");
			}
		}
		
		fakeBase = GetGame().GetWorld().FindEntityByName("FakeBaseHarbor");
		if (fakeBase)
		{
			fakeBaseComponent = SCR_TutorialFakeBaseComponent.Cast(fakeBase.FindComponent(SCR_TutorialFakeBaseComponent));
			if (fakeBaseComponent)
			{
				fakeBaseComponent.m_bVisible = false;
				fakeBaseComponent.ClearLinks();
			}
		}
		
		fakeBase = GetGame().GetWorld().FindEntityByName("FakeBaseSignalHill");
		if (fakeBase)
		{
			fakeBaseComponent = SCR_TutorialFakeBaseComponent.Cast(fakeBase.FindComponent(SCR_TutorialFakeBaseComponent));
			if (fakeBaseComponent)
			{
				fakeBaseComponent.m_bVisible = false;
				fakeBaseComponent.ClearLinks();
			}
		}
		
		fakeBase = GetGame().GetWorld().FindEntityByName("FakeBaseHarbor");
		if (fakeBase)
		{
			fakeBaseComponent = SCR_TutorialFakeBaseComponent.Cast(fakeBase.FindComponent(SCR_TutorialFakeBaseComponent));
			if (fakeBaseComponent)
			{
				fakeBaseComponent.m_bVisible = false;
				fakeBaseComponent.ClearLinks();
			}
		}
		
		fakeBase = GetGame().GetWorld().FindEntityByName("FakeBaseHospital");
		if (fakeBase)
		{
			fakeBaseComponent = SCR_TutorialFakeBaseComponent.Cast(fakeBase.FindComponent(SCR_TutorialFakeBaseComponent));
			if (fakeBaseComponent)
			{
				fakeBaseComponent.m_bVisible = false;
				fakeBaseComponent.ClearLinks();
			}
		}
		
		fakeBase = GetGame().GetWorld().FindEntityByName("FakeBaseHQ");
		if (fakeBase)
		{
			fakeBaseComponent = SCR_TutorialFakeBaseComponent.Cast(fakeBase.FindComponent(SCR_TutorialFakeBaseComponent));
			if (fakeBaseComponent)
			{
				fakeBaseComponent.m_bVisible = false;
				fakeBaseComponent.ClearLinks();
			}
		}
		
		fakeBase = GetGame().GetWorld().FindEntityByName("FakeBaseArleville");
		if (fakeBase)
		{
			fakeBaseComponent = SCR_TutorialFakeBaseComponent.Cast(fakeBase.FindComponent(SCR_TutorialFakeBaseComponent));
			if (fakeBaseComponent)
			{
				fakeBaseComponent.m_bVisible = false;
				fakeBaseComponent.ClearLinks();
			}
		}
		
		fakeBase = GetGame().GetWorld().FindEntityByName("FakeBaseLighthouseNorth");
		if (fakeBase)
		{
			fakeBaseComponent = SCR_TutorialFakeBaseComponent.Cast(fakeBase.FindComponent(SCR_TutorialFakeBaseComponent));
			if (fakeBaseComponent)
			{
				fakeBaseComponent.m_bVisible = false;
				fakeBaseComponent.m_bAllowServices = false;
				fakeBaseComponent.m_bAllowServicesSizeOverlay = false;
				fakeBaseComponent.m_eMilitarySymbolIdentity = EMilitarySymbolIdentity.UNKNOWN;
				fakeBaseComponent.ClearLinks();
			}
		}
		
		fakeBase = GetGame().GetWorld().FindEntityByName("Tutorial_MobileHQ");
		if (fakeBase)
		{
			fakeBaseComponent = SCR_TutorialFakeBaseComponent.Cast(fakeBase.FindComponent(SCR_TutorialFakeBaseComponent));
			if (fakeBaseComponent)
			{
				fakeBaseComponent.m_bVisible = false;
				fakeBaseComponent.ClearLinks();
			}
		}
	}
}