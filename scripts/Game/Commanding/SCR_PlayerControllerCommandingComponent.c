[EntityEditorProps(category: "GameScripted/Commanding", description: "This component should be attached to player controller and is used by commanding to send requests to server.")]
class SCR_PlayerControllerCommandingComponentClass : ScriptComponentClass
{
}

class SCR_PlayerControllerCommandingComponent : ScriptComponent
{
	[Attribute(desc: "Config of pairs of actions and commanding menu configs to decide which action opens which menu")]
	protected ResourceName m_sCommandingConfigActionPair;

	[Attribute("{2FFBD92174DDF3E0}Configs/Commanding/CommandingMapMenu.conf")]
	protected ResourceName m_sCommandingMapMenuConfigPath;

	[Attribute()]
	protected ref SCR_RadialMenuController m_RadialMenuController;

	protected SCR_RadialMenu m_RadialMenu;

	protected ref SCR_PlayerCommandingMenuConfig m_CommandingMapMenuConfig;
	protected ref SCR_PlayerCommandingMenuConfig m_CommandingMenuConfig;
	protected ref SCR_PlayerCommandingMenuActionsSetup m_CommandingMenuPairsConfig;

	protected IEntity m_SelectedEntity;

	protected SCR_CommandingManagerComponent m_CommandingManager;
	protected SCR_MapRadialUI m_MapContextualMenu;
	protected SCR_InfoDisplayExtended m_CurrentShownCommand;
	protected SCR_InfoDisplayExtended m_CurrentShownCommandPreview;

	protected string m_sExecutedCommandName;

	protected ref array<ref Shape> m_aShapes = {};

	protected bool m_bIsCommandExecuting = false;
	protected bool m_bSlaveGroupRequested = false;
	protected bool m_bIsCommandSelected;
	protected ref SCR_PhysicsHelper m_PhysicsHelper;

	protected string m_sSelectedCommandEntry;

	//! Used to offset the position to spawn waypoint so it is not spawned in terrain
	protected static const float ABOVE_TERRAIN_OFFSET = 0.5;

	//! Maximal range for command in meters
	protected static const float COMMANDING_VISUAL_RANGE = 10000;

	//------------------------------------------------------------------------------------------------
	//! \param[in] playerID
	//! \return
	static SCR_PlayerControllerCommandingComponent GetPlayerControllerCommandingComponent(int playerID)
	{
		PlayerController playerController = GetGame().GetPlayerManager().GetPlayerController(playerID);
		if (!playerController)
			return null;

		return SCR_PlayerControllerCommandingComponent.Cast(playerController.FindComponent(SCR_PlayerControllerCommandingComponent));
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	static SCR_PlayerControllerCommandingComponent GetLocalPlayerControllerCommandingComponent()
	{
		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (!playerController)
			return null;

		return SCR_PlayerControllerCommandingComponent.Cast(playerController.FindComponent(SCR_PlayerControllerCommandingComponent));
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		SetEventMask(owner, EntityEvent.INIT);

		SCR_GroupsManagerComponent groupManager = SCR_GroupsManagerComponent.GetInstance();
		m_CommandingManager = SCR_CommandingManagerComponent.GetInstance();

		Resource holder = BaseContainerTools.LoadContainer(m_sCommandingConfigActionPair);
		if (!holder)
			return;

		BaseContainer container = holder.GetResource().ToBaseContainer();
		m_CommandingMenuPairsConfig = SCR_PlayerCommandingMenuActionsSetup.Cast(BaseContainerTools.CreateInstanceFromContainer(container));

		if (!m_RadialMenuController || !m_CommandingMenuPairsConfig)
			return;

		m_RadialMenuController.GetOnTakeControl().Insert(OnControllerTakeControl);
		m_RadialMenuController.GetOnControllerChanged().Insert(OnControllerLostControl);

		InputManager input = GetGame().GetInputManager();
		foreach (SCR_PlayerCommandingConfigActionPair actionConfigPair : m_CommandingMenuPairsConfig.m_aActionConfigPairs)
		{
			if (!actionConfigPair)
				continue;

			input.AddActionListener(actionConfigPair.GetActionName(), EActionTrigger.DOWN, OpenCommandingMenu);
		}

		SCR_MapEntity.GetOnMapOpen().Insert(OnMapOpen);
		SCR_MapEntity.GetOnMapClose().Insert(OnMapClose);

		m_PhysicsHelper.InitPhysicsHelper();
	}

	//------------------------------------------------------------------------------------------------
	void OpenCommandingMenu()
	{
		if (m_RadialMenuController.IsMenuOpen())
		{
			m_RadialMenuController.CloseMenu();
			return;
		}

		ResourceName configPath;

		foreach (SCR_PlayerCommandingConfigActionPair actionConfigPair : m_CommandingMenuPairsConfig.m_aActionConfigPairs)
		{
			if (!actionConfigPair || !GetGame().GetInputManager().GetActionTriggered(actionConfigPair.GetActionName()))
				continue;

			configPath = actionConfigPair.GetConfig();
			break;
		}

		Resource holder = BaseContainerTools.LoadContainer(configPath);
		if (!holder || !holder.IsValid())
			return;

		BaseContainer container = holder.GetResource().ToBaseContainer();
		m_CommandingMenuConfig = SCR_PlayerCommandingMenuConfig.Cast(BaseContainerTools.CreateInstanceFromContainer(container));

		if (!m_CommandingMenuConfig)
			return;

		m_RadialMenuController.OnInputOpen();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnControllerTakeControl(SCR_RadialMenuController controller)
	{
		if (GetGame().GetPlayerController() != GetOwner())
			return;

		// Send/update entries in radial menu when control is gained
		controller.UpdateMenuData();
		SetupPlayerRadialMenu();
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	protected void OnControllerLostControl(SCR_RadialMenuController controller, bool hasControl)
	{
		if (!hasControl)
			RemoveListenersFromRadial();
	}

	//------------------------------------------------------------------------------------------------
	//!
	void SetupMapListener()
	{
		SCR_MapRadialUI mapMenu = SCR_MapRadialUI.GetInstance();
		if (!mapMenu)
			return;

		mapMenu.GetOnEntryPerformedInvoker().Insert(OnMapCommandPerformed);
	}

	//------------------------------------------------------------------------------------------------
	//!
	void RemoveMapListener()
	{
		SCR_MapRadialUI mapMenu = SCR_MapRadialUI.GetInstance();
		if (!mapMenu)
			return;

		mapMenu.GetOnEntryPerformedInvoker().Remove(OnMapCommandPerformed);
	}

	//------------------------------------------------------------------------------------------------
	//!
	void SetupPlayerRadialMenu()
	{
		BaseGameMode gameMode = GetGame().GetGameMode();
		if (!gameMode)
			return;

		m_RadialMenu = SCR_RadialMenu.GlobalRadialMenu();
		if (!m_RadialMenu)
			return;

		m_RadialMenu.GetOnBeforeOpen().Insert(OnPlayerRadialMenuBeforeOpen);
		m_RadialMenu.GetOnOpen().Insert(OnPlayerRadialMenuOpen);
		m_RadialMenu.GetOnClose().Insert(OnPlayerRadialMenuClose);
		m_RadialMenu.GetOnPerform().Insert(OnRadialMenuPerformed);
		m_RadialMenu.GetOnSelect().Insert(OnRadialMenuSelected);
		m_RadialMenu.GetOnOpenFailed().Insert(OnRadialMenuOpenFailed);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	protected void RemoveListenersFromRadial()
	{
		BaseGameMode gameMode = GetGame().GetGameMode();
		if (!gameMode)
			return;

		if (!m_RadialMenu)
			return;

		m_RadialMenu.GetOnBeforeOpen().Remove(OnPlayerRadialMenuBeforeOpen);
		m_RadialMenu.GetOnOpen().Remove(OnPlayerRadialMenuOpen);
		m_RadialMenu.GetOnClose().Remove(OnPlayerRadialMenuClose);
		m_RadialMenu.GetOnPerform().Remove(OnRadialMenuPerformed);
		m_RadialMenu.GetOnSelect().Remove(OnRadialMenuSelected);
		m_RadialMenu.GetOnOpenFailed().Remove(OnRadialMenuOpenFailed);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] config
	void OnMapOpen(MapConfiguration config)
	{
		m_MapContextualMenu = SCR_MapRadialUI.GetInstance();
		if (!m_MapContextualMenu)
			return;

		m_MapContextualMenu.GetOnMenuInitInvoker().Insert(SetupMapRadialMenu);
		m_MapContextualMenu.GetOnEntryPerformedInvoker().Insert(OnMapCommandPerformed);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] config
	void OnMapClose(MapConfiguration config)
	{
		if (!m_MapContextualMenu)
			return;

		m_MapContextualMenu.GetOnMenuInitInvoker().Remove(SetupMapRadialMenu);
		m_MapContextualMenu.GetOnEntryPerformedInvoker().Remove(OnMapCommandPerformed);
	}

	//------------------------------------------------------------------------------------------------
	//!
	void SetupMapRadialMenu()
	{
		if (!m_sCommandingMapMenuConfigPath || !m_MapContextualMenu)
			return;
		
		Resource holder = BaseContainerTools.LoadContainer(m_sCommandingMapMenuConfigPath);
		if (!holder || !holder.IsValid())
			return;

		BaseContainer container = holder.GetResource().ToBaseContainer();
		m_CommandingMapMenuConfig = SCR_PlayerCommandingMenuConfig.Cast(BaseContainerTools.CreateInstanceFromContainer(container));

		SCR_PlayerCommandingMenuCategoryElement rootCategory = m_CommandingMapMenuConfig.GetRootCategory();
		if (!rootCategory)
			return;

		AddElementsFromCategoryToMap(rootCategory);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] element
	//! \param[in] worldPos
	void OnMapCommandPerformed(SCR_SelectionMenuEntry element, float[] worldPos)
	{
		SCR_MapMenuCommandingEntry mapEntry = SCR_MapMenuCommandingEntry.Cast(element);
		if (!mapEntry)
			return;

		float height = GetGame().GetWorld().GetSurfaceY(worldPos[0], worldPos[1]);

		vector position;
		position[0] = worldPos[0];
		position[1] = height + ABOVE_TERRAIN_OFFSET;
		position[2] = worldPos[1];

		PrepareExecuteCommand(mapEntry.GetEntryIdentifier(), position);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] groupID
	//! \param[in] playerID
	void OnGroupLeaderChanged(int groupID, int playerID)
	{
		SCR_PlayerControllerGroupComponent playerGroupController = SCR_PlayerControllerGroupComponent.GetLocalPlayerControllerGroupComponent();
		if (!playerGroupController)
			return;

		//we do not care if it's outside of our group
		if (playerGroupController.GetGroupID() != groupID)
			return;

		bool enabled = playerID == GetGame().GetPlayerController().GetPlayerId();
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] groupID
	void OnGroupChanged(int groupID)
	{
		//check if player is the new leader of the group
		SCR_PlayerControllerGroupComponent playerGroupController = SCR_PlayerControllerGroupComponent.GetLocalPlayerControllerGroupComponent();
		if (!playerGroupController)
			return;

		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupsManager)
			return;

		SCR_AIGroup playersGroup = groupsManager.FindGroup(groupID);
		if (!playersGroup)
			return;

		bool enabled = playerGroupController.IsPlayerLeader(GetGame().GetPlayerController().GetPlayerId(), playersGroup);
	}

	//------------------------------------------------------------------------------------------------
	//! If a command is currently being displayed, hide/delete it
	protected void DeleteShownCommand()
	{
		if (!m_CurrentShownCommand)
			return;

		SCR_HUDManagerComponent hudManager = SCR_HUDManagerComponent.GetHUDManager();
		if (!hudManager)
			return;

		hudManager.StopDrawing(m_CurrentShownCommand);
		m_CurrentShownCommand = null;
	}

	//------------------------------------------------------------------------------------------------
	//! Hide command preview
	void HideCommandPreview()
	{
		m_bIsCommandSelected = false;

		if (!m_CurrentShownCommandPreview)
			return;

		SCR_HUDManagerComponent hudManager = SCR_HUDManagerComponent.GetHUDManager();
		if (!hudManager)
			return;

		hudManager.StopDrawing(m_CurrentShownCommandPreview);
		m_CurrentShownCommandPreview = null;
	}

	//------------------------------------------------------------------------------------------------
	//! Prepare the command to be shown
	//! \param[in] command name
	void ShowCommandPreview(string commandName)
	{
		if (m_CurrentShownCommandPreview)
			HideCommandPreview();

		if (commandName.IsEmpty())
			return;

		m_sSelectedCommandEntry = commandName;
		m_bIsCommandSelected = true;

		SCR_CommandingManagerComponent commandingManager = SCR_CommandingManagerComponent.GetInstance();
		if (!commandingManager)
			return;

		SCR_BaseGroupCommand command = SCR_BaseGroupCommand.Cast(commandingManager.FindCommand(m_sSelectedCommandEntry));
		if (!command || !command.CanShowPreview())
			return;

		command.VisualizeCommandPreview(vector.Zero);

		if (!m_CurrentShownCommandPreview)
			return;

		SCR_PlacedCommandInfoDisplay commandInfo = SCR_PlacedCommandInfoDisplay.Cast(m_CurrentShownCommandPreview);
		if (commandInfo)
			commandInfo.SetCanUpdatePosition(true);
	}

	//------------------------------------------------------------------------------------------------
	//! Recalculates worldPosition to screen space.
	//! \param[in] world
	//! \param[in] worldPosition
	//! \param[out] posX X Position on the screen
	//! \param[out] posY Y Position on the screen
	//! \param[in] int Camera thats being used
	protected bool GetWorldToScreenPosition(BaseWorld world, vector worldPosition, out float posX, out float posY, int cameraIndex = -1)
	{
		vector screenPosition = GetGame().GetWorkspace().ProjWorldToScreen(worldPosition, world, cameraIndex);
		posX = screenPosition[0];
		posY = screenPosition[1];

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Perform a TraceSegmented starting from Camera position in camera look direction with givin lengh
	//! \param[in] float, range in meters
	protected void PhysicsCommandTrace(float range = COMMANDING_VISUAL_RANGE)
	{
		PlayerController controller = GetGame().GetPlayerController();
		PlayerCamera camera = PlayerCamera.Cast(GetGame().GetCameraManager().CurrentCamera());
		if (!camera)
			return;

		IEntity controlledEntity = controller.GetControlledEntity();

		vector mat[4];
		camera.GetTransform(mat);
		vector end = mat[3] + mat[2] * range;
		array<IEntity> excludeArray = {};
		excludeArray.Insert(controlledEntity);

		ChimeraCharacter playerCharacter = ChimeraCharacter.Cast(controlledEntity);		
		if (playerCharacter && playerCharacter.IsInVehicle())
		{
			CompartmentAccessComponent compartmentComp = playerCharacter.GetCompartmentAccessComponent();
			if (compartmentComp)
			{
				IEntity vehicleIn = compartmentComp.GetVehicleIn(playerCharacter);
				if (vehicleIn)
					excludeArray.Insert(vehicleIn);
			}
		}

		m_PhysicsHelper = new SCR_PhysicsHelper();
		m_PhysicsHelper.TraceSegmented(mat[3], end, TraceFlags.ENTS | TraceFlags.WORLD | TraceFlags.ANY_CONTACT, EPhysicsLayerDefs.Projectile, excludeArray);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] menu
	//! \param[in] entry
	protected void OnRadialMenuSelected(SCR_SelectionMenu menu, SCR_SelectionMenuEntry entry)
	{
		m_bIsCommandSelected = false;

		if (!entry || entry.GetId() == string.Empty)
		{
			HideCommandPreview();
			return;
		}

		ShowCommandPreview(entry.GetId());
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] menu
	//! \param[in] entry
	protected void OnRadialMenuPerformed(SCR_SelectionMenu menu, SCR_SelectionMenuEntry entry)
	{
		if (!entry)
			return;

		m_bIsCommandSelected = false;

		HideCommandPreview();
		PrepareExecuteCommand(entry.GetId());
	}

	//------------------------------------------------------------------------------------------------
	void OnRadialMenuOpenFailed(SCR_SelectionMenu menu, SCR_ESelectionMenuFailReason reason)
	{
		//for now we handle the empty same as player not being leader since we dont have member commands yet
		//todo: change this when member commands get implemented
		switch (reason)
		{
			case SCR_ESelectionMenuFailReason.MENU_EMPTY:	SCR_NotificationsComponent.SendLocal(ENotification.COMMANDING_NO_RIGHTS); break;
			default: break;
		}
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] commandName
	void PrepareExecuteCommand(string commandName, vector targetPosition = "0 0 0")
	{
		//if another command is waiting for trace, do nothing
		if (m_bIsCommandExecuting || commandName.IsEmpty())
			return;

		m_sExecutedCommandName = commandName;

		if (targetPosition != vector.Zero)
		{
			ExecuteCommand(targetPosition, null);
			return;
		}

		PhysicsCommandTrace();

		if (m_PhysicsHelper)
		{
			m_PhysicsHelper.GetOnTraceFinished().Insert(ExecuteCommand);

			//prevent other commands being executed while this one is waiting for trace
			//todo:kuceramar: after radialmenurework, make radial menu elements disabled when this is true
			m_bIsCommandExecuting = true;
		}
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] targetPosition
	//! \param[in] tracedEntity
	void ExecuteCommand(vector targetPosition, IEntity tracedEntity)
	{
		m_bIsCommandExecuting = false;
		if (m_PhysicsHelper)
		{
			m_PhysicsHelper.GetOnTraceFinished().Remove(ExecuteCommand);
			m_PhysicsHelper = null;
		}

		SCR_CommandingManagerComponent commandingManager = SCR_CommandingManagerComponent.GetInstance();
		if (!commandingManager)
			return;

		int playerID = SCR_PlayerController.GetLocalPlayerId();

		SCR_PlayerControllerGroupComponent groupController = SCR_PlayerControllerGroupComponent.GetLocalPlayerControllerGroupComponent();
		SCR_GroupsManagerComponent groupManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupController || !groupManager)
			return;

		SCR_AIGroup playersGroup = groupManager.FindGroup(groupController.GetGroupID());
		if (!playersGroup)
			return;

		RplComponent rplComp;
		if (!m_bSlaveGroupRequested && !playersGroup.GetSlave())
		{
			rplComp = RplComponent.Cast(playersGroup.FindComponent(RplComponent));
			groupController.RequestCreateSlaveGroup(rplComp.Id());
			m_bSlaveGroupRequested = true;
		}

		SCR_AIGroup slaveGroup = playersGroup.GetSlave();
		//todo:mour Unlink this, only AI commands should be reliant on slave group, so commands should have that as option, not for all commands
		if (!slaveGroup)
		{
			//if there is not slaveGroup, we try to execute the command later because newly created group takes a bit of time to replicate
			GetGame().GetCallqueue().CallLater(ExecuteCommand, 100, false, targetPosition, tracedEntity);
			return;
		}

		m_bSlaveGroupRequested = false;
		rplComp = RplComponent.Cast(slaveGroup.FindComponent(RplComponent));
		RplId groupRplID = rplComp.Id();
		RplId cursorTargetRplID;

		int commandIndex = commandingManager.FindCommandIndex(m_sExecutedCommandName);

		if (m_SelectedEntity)
		{
			rplComp = RplComponent.Cast(m_SelectedEntity.FindComponent(RplComponent));
			if (rplComp)
				cursorTargetRplID = rplComp.Id();
		}

		SCR_BaseRadialCommand command = commandingManager.FindCommand(m_sExecutedCommandName);
		if (command)
		{
			DeleteShownCommand();
			command.VisualizeCommand(targetPosition);
		}

		Rpc(RPC_RequestExecuteCommand, commandIndex, cursorTargetRplID, groupRplID, targetPosition, playerID);
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] commandIndex
	//! \param[in] cursorTargetID
	//! \param[in] groupRplID
	//! \param[in] targetPoisition
	//! \param[in] playerID
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RPC_RequestExecuteCommand(int commandIndex, RplId cursorTargetID, RplId groupRplID, vector targetPoisition, int playerID)
	{
		SCR_CommandingManagerComponent commandingManager = SCR_CommandingManagerComponent.GetInstance();
		if (!commandingManager)
			return;
		
		//generate random seed for voiceline
		float soundEventSeed = Math.RandomFloatInclusive(0, 1);
		commandingManager.RequestCommandExecution(commandIndex, cursorTargetID, groupRplID, targetPoisition, playerID, soundEventSeed);
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] commandIndex
	void CommandExecutedCallback(int commandIndex)
	{
		Rpc(RPC_CommandExecutedCallback, commandIndex);
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] commandIndex
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	void RPC_CommandExecutedCallback(int commandIndex)
	{
		PlayCommandGesture(commandIndex);
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] commandIndex
	void PlayCommandGesture(int commandIndex)
	{
		if (commandIndex <= 0)
			return;

		SCR_MapEntity mapEntity = SCR_MapEntity.GetMapInstance();

		//skip the commanding gesture if player has map open
		if (mapEntity && mapEntity.IsOpen())
			return;

		SCR_BaseGroupCommand command = SCR_BaseGroupCommand.Cast(m_CommandingManager.FindCommand(m_CommandingManager.FindCommandNameFromIndex(commandIndex)));
		if (!command)
			return;

		int gestureID = command.GetCommandGestureID();

		IEntity playerControlledEntity = GetGame().GetPlayerController().GetControlledEntity();

		if (!playerControlledEntity)
			return;

		SCR_CharacterControllerComponent characterComponent = SCR_CharacterControllerComponent.Cast(playerControlledEntity.FindComponent(SCR_CharacterControllerComponent));
		if (!characterComponent)
			return;

		if (characterComponent.IsWeaponADS())
			return;

		characterComponent.TryStartCharacterGesture(gestureID, 3000);
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] owner
	//! \param[in] isOpen
	void UpdateRadialMenu(IEntity owner, bool isOpen)
	{
		if (!m_RadialMenu || !m_CommandingMenuConfig || !isOpen)
			return;

		PlayerCamera camera = PlayerCamera.Cast(GetGame().GetCameraManager().CurrentCamera());
		if (!camera)
			return;

		m_SelectedEntity = camera.GetCursorTarget();

		SCR_PlayerCommandingMenuCategoryElement rootCategory = m_CommandingMenuConfig.GetRootCategory();
		if (!rootCategory)
			return;

		AddElementsFromCategory(rootCategory);
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] category cannot be null
	//! \param[in] rootCategory
	//! \return true if this element is meant to be visible, otherwise false
	bool AddElementsFromCategory(SCR_PlayerCommandingMenuCategoryElement category, SCR_SelectionMenuCategoryEntry rootCategory = null)
	{
		array<ref SCR_PlayerCommandingMenuBaseElement> elements = category.GetCategoryElements();

		SCR_PlayerCommandingMenuCategoryElement elementCategory;
		SCR_SelectionMenuCategoryEntry createdCategory;

		bool canBeShown;
		bool canThisCategoryBeShown;
		foreach (SCR_PlayerCommandingMenuBaseElement element : elements)
		{
			elementCategory = SCR_PlayerCommandingMenuCategoryElement.Cast(element);
			if (elementCategory)
			{
				createdCategory = SCR_SelectionMenuCategoryEntry.Cast(AddRadialMenuElement(elementCategory, rootCategory));
				if (!createdCategory)
					continue;

				canThisCategoryBeShown = AddElementsFromCategory(elementCategory, createdCategory);
				if (!canThisCategoryBeShown)
				{
					if (rootCategory)
						rootCategory.RemoveEntry(createdCategory);
					else
						m_RadialMenu.RemoveEntry(createdCategory);
				}

				canBeShown = canThisCategoryBeShown || canBeShown;
			}
			else
			{
				canBeShown = AddRadialMenuElement(element, rootCategory) || canBeShown;
			}
		}

		return canBeShown;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] category cannot be null
	//! \param[in] parentCategory
	bool AddElementsFromCategoryToMap(notnull SCR_PlayerCommandingMenuCategoryElement category, SCR_SelectionMenuCategoryEntry parentCategory = null)
	{
		SCR_SelectionMenuCategoryEntry mapEntryCategory = m_MapContextualMenu.AddRadialCategory(category.GetCategoryDisplayText(), parentCategory); // add map category entry

		array<ref SCR_PlayerCommandingMenuBaseElement> elements = category.GetCategoryElements();
		SCR_PlayerCommandingMenuCategoryElement elementCategory;
		bool canBeShown;
		foreach (SCR_PlayerCommandingMenuBaseElement element : elements)
		{
			elementCategory = SCR_PlayerCommandingMenuCategoryElement.Cast(element);
			if (elementCategory)
			{
				if (elementCategory.GetCanShowOnMap())
					canBeShown = AddElementsFromCategoryToMap(elementCategory, mapEntryCategory) || canBeShown;
			}
			else
			{
				canBeShown = InsertElementToMapRadial(element, category, mapEntryCategory) || canBeShown;
			}
		}

		if (!canBeShown)
		{
			if (parentCategory)
				parentCategory.RemoveEntry(mapEntryCategory);
			else
				m_MapContextualMenu.RemoveRadialEntry(mapEntryCategory);

			return canBeShown;
		}

		ResourceName imagesetName = category.GetIconImageset();
		string iconName = category.GetIconName();
		if (mapEntryCategory && !imagesetName.IsEmpty() && !iconName.IsEmpty())
			mapEntryCategory.SetIcon(imagesetName, iconName);

		return canBeShown;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] element cannot be null
	//! \param[in] category
	//! \param[in] mapCategory
	SCR_MapMenuCommandingEntry InsertElementToMapRadial(SCR_PlayerCommandingMenuBaseElement element, notnull SCR_PlayerCommandingMenuCategoryElement category, SCR_SelectionMenuCategoryEntry mapCategory)
	{
		SCR_CommandingManagerComponent commandingManager = SCR_CommandingManagerComponent.GetInstance();
		if (!commandingManager)
			return null;

		SCR_ChimeraCharacter user = SCR_ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity());
		if (!user)
			return null;

		SCR_PlayerCommandingMenuCommand commandElement = SCR_PlayerCommandingMenuCommand.Cast(element);
		if (!commandingManager.CanShowOnMap(commandElement.GetCommandName()))
			return null;

		SCR_BaseRadialCommand command = commandingManager.FindCommand(commandElement.GetCommandName());
		if (!command)
			return null;

		SCR_MapMenuCommandingEntry mapEntry = new SCR_MapMenuCommandingEntry(commandElement.GetCommandName());
		mapEntry.SetName(commandingManager.GetCommandDisplayTextByName(commandElement.GetCommandName()));
		mapEntry.SetIcon(command.GetIconImageset(), command.GetIconName());

		bool canPerform = command.CanBePerformed(user);
		mapEntry.Enable(canPerform);
		if (!canPerform)
			mapEntry.SetDescription(command.GetCannotPerformReason());

		m_MapContextualMenu.InsertCustomRadialEntry(mapEntry, mapCategory);
		return mapEntry;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] newElement
	//! \param[in] parentCategory
	//! \return
	SCR_SelectionMenuEntry AddRadialMenuElement(SCR_PlayerCommandingMenuBaseElement newElement, SCR_SelectionMenuCategoryEntry parentCategory = null)
	{
		SCR_PlayerCommandingMenuCommand commandElement = SCR_PlayerCommandingMenuCommand.Cast(newElement);
		if (commandElement)
			return AddCommandElement(commandElement, parentCategory);

		SCR_PlayerCommandingMenuCategoryElement categoryElement = SCR_PlayerCommandingMenuCategoryElement.Cast(newElement);
		if (categoryElement)
			return AddCategoryElement(categoryElement, parentCategory);

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] category cannot be null
	//! \param[in] parentCategory
	//! \return
	SCR_SelectionMenuEntry AddCategoryElement(SCR_PlayerCommandingMenuCategoryElement category, SCR_SelectionMenuCategoryEntry parentCategory = null)
	{
		SCR_SelectionMenuCategoryEntry newCategory = new SCR_SelectionMenuCategoryEntry();

		string id;
		if (parentCategory)
			id = parentCategory.GetId() + SCR_StringHelper.UNDERSCORE + parentCategory.GetEntries().Count().ToString();
		else
			id = m_RadialMenu.GetEntries().Count().ToString();

		newCategory.SetId(id);
		newCategory.SetName(category.GetCategoryDisplayText());

		ResourceName imagesetName = category.GetIconImageset();
		string iconName = category.GetIconName();
		if (!imagesetName.IsEmpty() && !iconName.IsEmpty())
			newCategory.SetIcon(imagesetName, iconName);

		if (!parentCategory)
		{
			m_RadialMenu.AddCategoryEntry(newCategory);
			return newCategory;
		}

		parentCategory.AddEntry(newCategory);

		return newCategory;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] command cannot be null
	//! \param[in] parentCategory
	//! \return
	SCR_SelectionMenuEntry AddCommandElement(SCR_PlayerCommandingMenuCommand command, SCR_SelectionMenuCategoryEntry parentCategory = null)
	{
		SCR_CommandingManagerComponent commandingManager = SCR_CommandingManagerComponent.GetInstance();
		if (!commandingManager)
			return null;

		SCR_ChimeraCharacter user = SCR_ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity());
		if (!user)
			return null;

		if (!commandingManager.CanShowCommand(command.GetCommandName()))
			return null;

		SCR_BaseRadialCommand groupCommand = commandingManager.FindCommand(command.GetCommandName());

		SCR_SelectionMenuEntryCommand entry = new SCR_SelectionMenuEntryCommand();

		string displayName = command.GetCommandCustomName();
		if (displayName.IsEmpty())
			displayName = commandingManager.GetCommandDisplayTextByName(command.GetCommandName());
	
		//entry.SetName(displayName);
		bool canExecute;
		bool canPerform = groupCommand.CanBePerformed(user);
		
		if (canPerform)
		{
			canExecute = groupCommand.CanBeExecuted(m_SelectedEntity);
		}
		
		entry.SetId(command.GetCommandName());
		entry.SetIcon(groupCommand.GetIconImageset(), groupCommand.GetIconName());
		entry.Enable(canPerform && canExecute);
		entry.SetCommandText(displayName);
		if (!canPerform)
			entry.SetDescription(groupCommand.GetCannotPerformReason());
		else if (!canExecute)
			entry.SetDescription(groupCommand.GetCannotExecuteReason());
		
		if (parentCategory)
			parentCategory.AddEntry(entry);
		else
			m_RadialMenu.AddEntry(entry);

		return entry;
	}

	//------------------------------------------------------------------------------------------------
	//!
	void OnPlayerRadialMenuBeforeOpen()
	{
		if (!m_RadialMenu || !m_CommandingMenuConfig)
			return;

		m_RadialMenu.ClearEntries();

		PlayerCamera camera = PlayerCamera.Cast(GetGame().GetCameraManager().CurrentCamera());
		if (!camera)
			return;

		m_SelectedEntity = camera.GetCursorTarget();

		SCR_PlayerCommandingMenuCategoryElement rootCategory = m_CommandingMenuConfig.GetRootCategory();
		if (!rootCategory)
			return;

		AddElementsFromCategory(rootCategory);

		GetGame().GetInputManager().AddActionListener("BindQuickslot4", EActionTrigger.DOWN, OnQuickslotBind4);
		GetGame().GetInputManager().AddActionListener("BindQuickslot5", EActionTrigger.DOWN, OnQuickslotBind5);
		GetGame().GetInputManager().AddActionListener("BindQuickslot6", EActionTrigger.DOWN, OnQuickslotBind6);
		GetGame().GetInputManager().AddActionListener("BindQuickslot7", EActionTrigger.DOWN, OnQuickslotBind7);
		GetGame().GetInputManager().AddActionListener("BindQuickslot8", EActionTrigger.DOWN, OnQuickslotBind8);
		GetGame().GetInputManager().AddActionListener("BindQuickslot9", EActionTrigger.DOWN, OnQuickslotBind9);

		m_bIsCommandSelected = false;
	}
	//------------------------------------------------------------------------------------------------
	void OnPlayerRadialMenuOpen()
	{
		SCR_WeaponSwitchingBaseUI baseUI = SCR_WeaponSwitchingBaseUI.GetWeaponSwitchingBaseUI();
		if (baseUI)
			baseUI.OpenQuickSlots();		
	}
	//------------------------------------------------------------------------------------------------
	void OnPlayerRadialMenuClose()
	{
		GetGame().GetInputManager().RemoveActionListener("BindQuickslot4", EActionTrigger.DOWN, OnQuickslotBind4);
		GetGame().GetInputManager().RemoveActionListener("BindQuickslot5", EActionTrigger.DOWN, OnQuickslotBind5);
		GetGame().GetInputManager().RemoveActionListener("BindQuickslot6", EActionTrigger.DOWN, OnQuickslotBind6);
		GetGame().GetInputManager().RemoveActionListener("BindQuickslot7", EActionTrigger.DOWN, OnQuickslotBind7);
		GetGame().GetInputManager().RemoveActionListener("BindQuickslot8", EActionTrigger.DOWN, OnQuickslotBind8);
		GetGame().GetInputManager().RemoveActionListener("BindQuickslot9", EActionTrigger.DOWN, OnQuickslotBind9);

		HideCommandPreview();

		SCR_WeaponSwitchingBaseUI baseUI = SCR_WeaponSwitchingBaseUI.GetWeaponSwitchingBaseUI();
		if (baseUI)
			baseUI.CloseQuickSlots();

		m_bIsCommandSelected = false;
	}

	//------------------------------------------------------------------------------------------------
	void OnQuickslotBind4() { BindToQuickslot(4); }
	void OnQuickslotBind5() { BindToQuickslot(5); }
	void OnQuickslotBind6() { BindToQuickslot(6); }
	void OnQuickslotBind7() { BindToQuickslot(7); }
	void OnQuickslotBind8() { BindToQuickslot(8); }
	void OnQuickslotBind9() { BindToQuickslot(9); }

	//------------------------------------------------------------------------------------------------
	//!
	void BindToQuickslot(int slotIndex)
	{
		SCR_SelectionMenuEntryCommand commandEntry = SCR_SelectionMenuEntryCommand.Cast(m_RadialMenu.GetSelectionEntry());
		if (!commandEntry)
			return;

		if (!commandEntry.IsEnabled())
			return;

		IEntity character = SCR_PlayerController.GetLocalControlledEntity();
		if (!character)
			return;

		SCR_CharacterControllerComponent characterController = SCR_CharacterControllerComponent.Cast(character.FindComponent(SCR_CharacterControllerComponent));
		if (!characterController)
			return;

		SCR_InventoryStorageManagerComponent storageManager = SCR_InventoryStorageManagerComponent.Cast(characterController.GetInventoryStorageManager());
		if (!storageManager)
			return;

		SCR_CharacterInventoryStorageComponent characterStorage = storageManager.GetCharacterStorage();
		if (!characterStorage)
			return;

		SCR_QuickslotCommandContainer commandContainer = new SCR_QuickslotCommandContainer(commandEntry.GetId());

		characterStorage.InsertContainerIntoQuickslot(commandContainer, slotIndex);

		SCR_WeaponSwitchingBaseUI.RefreshQuickSlots();
	}

	//------------------------------------------------------------------------------------------------
	bool IsOpened()
	{
		if (!m_RadialMenu)
			return false;

		return m_RadialMenu.IsOpened();
	}

	//------------------------------------------------------------------------------------------------
	bool IsCommandSelected()
	{
		return m_bIsCommandSelected;
	}

	//------------------------------------------------------------------------------------------------
	void SetShownCommand(SCR_InfoDisplayExtended infoDisplay)
	{
		m_CurrentShownCommand = infoDisplay;
	}

	//------------------------------------------------------------------------------------------------
	void SetShownCommandPreview(SCR_InfoDisplayExtended infoDisplay)
	{
		m_CurrentShownCommandPreview = infoDisplay;
	}

	//------------------------------------------------------------------------------------------------
	//!
	void DrawWaypointVisualization(vector targetPosition, float wpRadius, bool deletePrevious = true)
	{
		if (deletePrevious)
			m_aShapes.Clear();

		//Shape shape = Shape.CreateSphere(Color.WHITE, ShapeFlags.VISIBLE | ShapeFlags.WIREFRAME, targetPosition, wpRadius);
		//m_aShapes.Insert(shape);
	}
}

enum SCR_ECommandVisualizationDuration
{
	BRIEF = 10,			//!< shows the command visualization only for few seconds
	LONGLASTING,	//!< shows the command visualization for as long as the command is active
	PERMANENT		//!< shows the command visualization permanently, use for debug purposes
}
