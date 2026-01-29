//#define FAKE_PLAYER //--- Allow placing AIs which are pretending to be players. Used for video capture.

[ComponentEditorProps(category: "GameScripted/Editor", description: "Debug editor component. Works only with SCR_EditorBaseEntity!", icon: "WBData/ComponentEditorProps/componentEditor.png")]
class SCR_DebugEditorComponentClass: SCR_BaseEditorComponentClass
{
};

/** @ingroup Editor_Components
*/
/*!
Component to handle various debug modes for the editor.
*/
class SCR_DebugEditorComponent : SCR_BaseEditorComponent
{
#ifdef WORKBENCH
	private SCR_EditableEntityComponent m_TestComponent;
	private int m_LayerID;
	private SCR_EditableEntityComponent m_LayerOrig;
#endif
	
	private SCR_BaseEditableEntityFilter m_FocusedManager;
	private SCR_HoverEditableEntityFilter m_HoverManager;
	private SCR_BaseEditableEntityFilter m_SelectedManager;
	
#ifdef FAKE_PLAYER
	private static int m_iFakePlayerIndex = 0;
	private SCR_EditableEntityComponent m_FakePlayer;
#endif
	
	ref set<SCR_EditableEntityComponent> m_LOL = new set<SCR_EditableEntityComponent>;
	
#ifdef FAKE_PLAYER
	static string GetFakePlayerName(int playerID)
	{
		array<string> debugNames = {"", "Hedrik", "Ton", "Roach", "Natasha"};
		return debugNames[-playerID % debugNames.Count()];
	}
	static string GetFakePlayerName()
	{
		return GetFakePlayerName(m_iFakePlayerIndex);
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void CreateFakePlayerServer()
	{
		array<SCR_SpawnPoint> spawnPoints = SCR_SpawnPoint.GetSpawnPointsForFaction("USSR");
		if (!spawnPoints.IsEmpty())
		{
			ResourceName prefab = "{DCB41B3746FDD1BE}Prefabs/Characters/OPFOR/USSR_Army/Character_USSR_Rifleman.et";
			EntitySpawnParams spawnParams = new EntitySpawnParams();
			spawnParams.Transform[3] = spawnPoints[0].GetOrigin();
			IEntity fakePlayer = GetGame().SpawnEntityPrefab(Resource.Load(prefab), GetGame().GetWorld(), spawnParams);
			
			prefab = "{69B7035099CEE1FC}Prefabs/AI/Groups/ChimeraGroupNonEditable.et";
			AIGroup group = AIGroup.Cast(GetGame().SpawnEntityPrefab(Resource.Load(prefab), GetGame().GetWorld(), spawnParams));
			
			AIControlComponent aiControl = AIControlComponent.Cast(fakePlayer.FindComponent(AIControlComponent));
			aiControl.ActivateAI();
			AIAgent agent = aiControl.GetAIAgent();
			
			prefab = "{FFF9518F73279473}PrefabsEditable/Auto/AI/Waypoints/E_AIWaypoint_Move.et";//"{750A8D1695BD6998}Prefabs/AI/Waypoints/AIWaypoint_Move.et";
			MenuRootBase menu = MenuRootBase.Cast(GetGame().GetMenuManager().GetTopMenu());
			SCR_CursorEditorUIComponent cursorComponent = SCR_CursorEditorUIComponent.Cast(menu.GetRootComponent().FindComponent(SCR_CursorEditorUIComponent));
			vector cursorWorldPos;
			cursorComponent.GetCursorWorldPos(cursorWorldPos);
			spawnParams.Transform[3] = cursorWorldPos;
			AIWaypoint wp =  AIWaypoint.Cast(GetGame().SpawnEntityPrefab(Resource.Load(prefab), GetGame().GetWorld(), spawnParams));
			
			group.AddAgent(agent);
			group.AddWaypoint(wp);
			
			m_FakePlayer = SCR_EditableEntityComponent.GetEditableEntity(fakePlayer);
			
			m_iFakePlayerIndex--;
			SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
			gameMode.GetOnPlayerSpawned().Invoke(m_iFakePlayerIndex, fakePlayer);
			//SCR_NotificationsComponent.SendToEveryone(ENotification.PLAYER_JOINED_FACTION, m_iFakePlayerIndex);
		}
	}
	protected void PingFakePlayer()
	{
		SCR_PingEditorComponent pingManager = SCR_PingEditorComponent.Cast(SCR_PingEditorComponent.GetInstance(SCR_PingEditorComponent));
		Print(m_FakePlayer);
		pingManager.CallEvents(GetManager(), true, -1, false, false, vector.Zero, m_FakePlayer); //--- Set to public in the class
	}
#endif
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		/* Measure set<> vs array<> (SPOILER: set<> wins!)
		SCR_EditableEntityCore core = SCR_EditableEntityCore.Cast(SCR_EditableEntityCore.GetInstance(SCR_EditableEntityCore));
		core.GetAllEntities(m_LOL);
		
		DbgUI.Begin("", 0, 0);
		DbgUI.Text("Count: " + m_LOL.Count().ToString());
		DbgUI.End();
		
		set<SCR_EditableEntityComponent> entities = new set<SCR_EditableEntityComponent>;
		SCR_EditableEntityComponent entity;
		for (int i = 0, count = m_LOL.Count(); i < count; i++)
		{
			entity = m_LOL[i];
			if (entities.Find(entity) == -1)
			{
				entities.Insert(entity);
			}
		}
		*/
#ifdef FAKE_PLAYER
		if (Debug.KeyState(KeyCode.KC_U))
		{
			Debug.ClearKey(KeyCode.KC_U);
			
			Rpc(CreateFakePlayerServer);
		}
		if (Debug.KeyState(KeyCode.KC_K))
		{
			Debug.ClearKey(KeyCode.KC_K);
			
			PingFakePlayer();
		}
#endif
		
#ifdef WORKBENCH
		if (m_TestComponent)
		{
			if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_EDITOR_ENTITIES_TEST_ENTITY_ACCESS_KEY_TOGGLE))
			{
				DiagMenu.SetValue(SCR_DebugMenuID.DEBUGUI_EDITOR_ENTITIES_TEST_ENTITY_ACCESS_KEY_TOGGLE, false);
				SCR_AccessKeysEditorComponent accessKeyComponent = SCR_AccessKeysEditorComponent.Cast(SCR_AccessKeysEditorComponent.GetInstance(SCR_AccessKeysEditorComponent));
				if (!accessKeyComponent) return;
				
				typename keysType = EEditableEntityAccessKey;
				int keyValue;
				keysType.GetVariableValue(null, DiagMenu.GetRangeValue(SCR_DebugMenuID.DEBUGUI_EDITOR_ENTITIES_TEST_ENTITY_ACCESS_KEY), keyValue);
				if (!m_TestComponent.HasAccessSelf(keyValue))
				{
					accessKeyComponent.AddEntityAccessKey(m_TestComponent, keyValue);
				}
				else
				{
					accessKeyComponent.RemoveEntityAccessKey(m_TestComponent, keyValue);
				}
				m_TestComponent.LogAccessKey();
			}
			bool show = DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_EDITOR_ENTITIES_TEST_ENTITY_SHOW);
			if (show != m_TestComponent.GetVisibleSelf())
			{
				m_TestComponent.SetVisible(show);
			}
			if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_EDITOR_ENTITIES_TEST_ENTITY_SELECT))
			{
				DiagMenu.SetValue(SCR_DebugMenuID.DEBUGUI_EDITOR_ENTITIES_TEST_ENTITY_SELECT, false);
				Print(m_SelectedManager);
				if (m_SelectedManager) m_SelectedManager.Toggle(m_TestComponent);
			}
			if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_EDITOR_ENTITIES_TEST_ENTITY_FOCUS))
			{
				DiagMenu.SetValue(SCR_DebugMenuID.DEBUGUI_EDITOR_ENTITIES_TEST_ENTITY_FOCUS, false);
				if (m_FocusedManager) m_FocusedManager.Toggle(m_TestComponent);
			}
		}
#endif
		
		//--- Ad-hoc debugs
		/*
		if (Debug.KeyState(KeyCode.KC_M))
		{
			Debug.ClearKey(KeyCode.KC_M);
			Print("MOVE");
			
			SCR_EntitiesManagerEditorComponent entitiesManager = SCR_EntitiesManagerEditorComponent.Cast(GetInstance(SCR_EntitiesManagerEditorComponent));
			if (entitiesManager)
			{
				SCR_SelectedEditableEntityFilter selectedFilter = SCR_SelectedEditableEntityFilter.Cast(entitiesManager.GetFilter(EEditableEntityState.SELECTED));
				if (selectedFilter)
				{
					SCR_EditableEntityComponent entity = selectedFilter.GetFirstEntity();
					if (entity)
					{
						MenuManager menuManager = GetGame().GetMenuManager();
						if (menuManager)
						{
							MenuRootBase menu = MenuRootBase.Cast(menuManager.GetTopMenu());
							if (menu)
							{
								SCR_CursorEditorUIComponent cursorComponent = SCR_CursorEditorUIComponent.Cast(menu.GetRootComponent().FindComponent(SCR_CursorEditorUIComponent));
								if (cursorComponent)
								{
									vector worldPos;
									if (cursorComponent.GetCursorWorldPos(worldPos))
									{
										entity.GetOwner().SetOrigin(worldPos);
										entity.UpdateStaticPos();
									}
								}
							}
						}
					}
				}
			}
		}
		*/
		/*
		if (Debug.KeyState(KeyCode.KC_L))
		{
			Debug.ClearKey(KeyCode.KC_L);
			
			// Custom instance.
			RplTestPropType instance = new RplTestPropType;
			instance.iVal = 1138;
			instance.fVal = 42;
			instance.bVal = true;
			
			// Pure data holder without any index/position. Just capacity and length.
			SSnapshot s = new SSnapshot(32);
			
			// This is your writer. Contains a pointer thats shifted with every write.
			// Starts at 0 and can be moved by Seek(...) method.
			SSnapSerializer writer = SSnapSerializer.MakeWriter(s);
			RplTestPropType.Extract(instance, null, writer);
			
			// This is your reader which contains a pointer, also starts at 0 and moves
			// with every read.
			SSnapSerializer reader = SSnapSerializer.MakeReader(s);
			
			// Comparisons are effectively reads. They do move the pointer. Thats why
			// you can do ops like: reader.Compare(A, 4).Compare(B, 4);
			// The memory from A gets compared with the snapshot contents at the
			// position of the readers pointer. Then the reader increments the pointer.
			Print(RplTestPropType.PropCompare(instance, reader, null), LogLevel.DEBUG);	
			
			instance.fVal = 42;
			reader = SSnapSerializer.MakeReader(s);
			Print(RplTestPropType.PropCompare(instance, reader, null), LogLevel.WARNING);	
		}
		*/
		
		/*
		if (Debug.KeyState(KeyCode.KC_J))
		{
			Debug.ClearKey(KeyCode.KC_J);
			ArmaReforgerScripted game = GetGame();
			if (!game) return;
			
			PlayerController pc = game.GetPlayerController();		
			if (!pc) return;
			
			ChimeraCharacter character = ChimeraCharacter.Cast(pc.GetControlledEntity());
			if (!character) return;		
			
			SCR_CharacterControllerComponent controller = SCR_CharacterControllerComponent.Cast(character.FindComponent(SCR_CharacterControllerComponent));
			if (!controller) return;
			
			Print(controller.GetDisableMovementControls());
			
			GetGame().GetInputManager().ActivateContext("CharacterGeneralContext");
			
			controller.SetDisableWeaponControls(false);
			controller.SetDisableMovementControls(false);
		}
		*/
		/*
		if (Debug.KeyState(KeyCode.KC_L))
		{
			Debug.ClearKey(KeyCode.KC_L);
			
			//--- Init
			RplTestSerializer instance = new RplTestSerializer;
			instance.iVal = 1138;
			instance.fVal = 42;
			instance.bVal = false;
			
			SSnapSerializer snapSerializer = SSnapSerializer.MakeReader(new SSnapshot(32)); 
			
			RplTestSerializer.Extract(instance, null, snapSerializer); 
			
			bool compare1 = RplTestSerializer.PropCompare(instance, snapSerializer, null); 
			Print(compare1, LogLevel.DEBUG);
		}
		*/
	}
	override void EOnEditorDebug(array<string> debugTexts)
	{
		#ifdef WORKBENCH
		if (!SCR_EditorBaseEntity.Cast(GetOwner()).IsOpened()) return;
		if (m_HoverManager)
		{
			string entityName = "N/A";
			bool isDelegate
			SCR_EditableEntityComponent entityUnderCursor = m_HoverManager.GetEntityUnderCursor(isDelegate);
			if (entityUnderCursor) entityName = entityUnderCursor.GetDisplayName();
			debugTexts.Insert("Under Cursor: " + entityName + " / " + isDelegate.ToString());
		}
		if (m_TestComponent)
		{
			debugTexts.Insert("\nTestEntity: " + m_TestComponent.GetDisplayName());
			
			SCR_EditableEntityComponent parent = m_TestComponent.GetParentEntity();
			if (parent)
				debugTexts.Insert("Parent: " + parent.GetDisplayName());
			else
				debugTexts.Insert("Parent: N/A");
			
			typename enumType = EEditableEntityState;
			int enumCount = enumType.GetVariableCount();
			for (int i = 0; i < enumCount; i++)
			{
				int val;
				if (enumType.GetVariableType(i) == int && enumType.GetVariableValue(null, i, val))
				{
					debugTexts.Insert(string.Format("%1: %2", enumType.GetVariableName(i), m_TestComponent.HasEntityState(val) > 0));
				}
				
			}
			debugTexts.Insert("\n");
		}
		#endif
	}
	override void EOnEditorPostActivate()
	{
		SCR_EntitiesManagerEditorComponent entitiesManager = SCR_EntitiesManagerEditorComponent.Cast(SCR_EntitiesManagerEditorComponent.GetInstance(SCR_EntitiesManagerEditorComponent));
		if (entitiesManager)
		{
			m_FocusedManager = entitiesManager.GetFilter(EEditableEntityState.FOCUSED);
			m_HoverManager = SCR_HoverEditableEntityFilter.Cast(entitiesManager.GetFilter(EEditableEntityState.HOVER));
			m_SelectedManager = entitiesManager.GetFilter(EEditableEntityState.SELECTED);
		}
	}
	override void EOnEditorActivate()
	{
#ifdef FAKE_PLAYER
		m_iFakePlayerIndex = 0;
		SetEventMask(GetOwner(), EntityEvent.FRAME);
#endif
		
#ifdef WORKBENCH
		SetEventMask(GetOwner(), EntityEvent.FRAME);

		GenericEntity testEntity = GenericEntity.Cast(GetGame().FindEntity("TestEntity"));
		if (!testEntity) return;
		
		m_TestComponent = SCR_EditableEntityComponent.Cast(testEntity.FindComponent(SCR_EditableEntityComponent));
		if (!m_TestComponent) return;

		m_LayerOrig = m_TestComponent.GetParentEntity();
		
		typename keysType = EEditableEntityAccessKey;
		int keysCount = keysType.GetVariableCount();
		
		DiagMenu.RegisterRange(SCR_DebugMenuID.DEBUGUI_EDITOR_ENTITIES_TEST_ENTITY_ACCESS_KEY, "", "TestEntity: Access Key", "Editable Entities", string.Format("0, %1, 0, 1", keysCount - 1));
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_EDITOR_ENTITIES_TEST_ENTITY_ACCESS_KEY_TOGGLE, "", "TestEntity: Access Key: Toggle", "Editable Entities");
		DiagMenu.RegisterRange(SCR_DebugMenuID.DEBUGUI_EDITOR_ENTITIES_TEST_ENTITY_LAYER, "", "TestEntity: Layer", "Editable Entities", "0, 2, 0, 1");
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_EDITOR_ENTITIES_TEST_ENTITY_SELECT, "", "TestEntity: Toggle Selection", "Editable Entities");
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_EDITOR_ENTITIES_TEST_ENTITY_FOCUS, "", "TestEntity: Toggle Focus", "Editable Entities");
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_EDITOR_ENTITIES_TEST_ENTITY_SHOW, "", "TestEntity: Show", "Editable Entities");
		
		DiagMenu.SetValue(SCR_DebugMenuID.DEBUGUI_EDITOR_ENTITIES_TEST_ENTITY_LAYER, 0);
		DiagMenu.SetValue(SCR_DebugMenuID.DEBUGUI_EDITOR_ENTITIES_TEST_ENTITY_SHOW, true);

#endif
	}
	override protected void EOnEditorDeactivate()
	{
		DiagMenu.Unregister(SCR_DebugMenuID.DEBUGUI_EDITOR_ENTITIES_TEST_ENTITY_ACCESS_KEY);
		DiagMenu.Unregister(SCR_DebugMenuID.DEBUGUI_EDITOR_ENTITIES_TEST_ENTITY_ACCESS_KEY_TOGGLE);
		DiagMenu.Unregister(SCR_DebugMenuID.DEBUGUI_EDITOR_ENTITIES_TEST_ENTITY_LAYER);
		DiagMenu.Unregister(SCR_DebugMenuID.DEBUGUI_EDITOR_ENTITIES_TEST_ENTITY_SELECT);
		DiagMenu.Unregister(SCR_DebugMenuID.DEBUGUI_EDITOR_ENTITIES_TEST_ENTITY_FOCUS);
		DiagMenu.Unregister(SCR_DebugMenuID.DEBUGUI_EDITOR_ENTITIES_TEST_ENTITY_SHOW);
	}
};