class SCR_ScenarioFrameworkDebug : ScriptAndConfig
{
	//------------------------------------------------------------------------------------------------
	//! Shows debug menus depending on what is enabled
	static void OnDiag()
	{
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_SCENARIO_FRAMEWORK_TASKS))
			Tasks();

		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_SCENARIO_FRAMEWORK_REGISTERED_AREAS))
			RegisteredAreas();

		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_SCENARIO_FRAMEWORK_DEBUG_AREAS))
			DebugAreas();

		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_SCENARIO_FRAMEWORK_LAYER_INSPECTOR))
			LayerInspector();

		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_SCENARIO_FRAMEWORK_ACTION_INSPECTOR))
			ActionInspector();

		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_SCENARIO_FRAMEWORK_LOGIC_INSPECTOR))
			LogicInspector();

		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_SCENARIO_FRAMEWORK_PLUGIN_INSPECTOR))
			PluginInspector();

		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_SCENARIO_FRAMEWORK_CONDITION_INSPECTOR))
			ConditionInspector();
		
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_SCENARIO_FRAMEWORK_DEBUG_ACTIONS))
			DebugActions();
	}

	//------------------------------------------------------------------------------------------------
	//! The method displays active scenario framework tasks, allows inspection of areas, layer tasks, slot tasks, and provides options manipulating with them further
	static void Tasks()
	{
		SCR_TaskSystem taskSystem = SCR_TaskSystem.GetInstance();
	    if (!taskSystem)
	        return;

		DbgUI.Begin("ScenarioFramework Tasks");
		{
			array<SCR_ScenarioFrameworkTask> scenarioFrameworkTasks = {};
			array<SCR_Task> finishedTasks = {};
			array<string> listElements = {};
			array<SCR_Task> tasks = {};
			taskSystem.GetTasks(tasks);
			SCR_ScenarioFrameworkTask scenarioFrameworkTask;
			SCR_ScenarioFrameworkSlotTask scenarioFrameworkSlotTask;
			SCR_ScenarioFrameworkLayerTask scenarioFrameworkLayerTask;
			SCR_ScenarioFrameworkArea scenarioFrameworkArea;
			foreach (SCR_Task task : tasks)
			{
				scenarioFrameworkTask = SCR_ScenarioFrameworkTask.Cast(task);
				if (!scenarioFrameworkTask)
					continue;
				
				scenarioFrameworkSlotTask = scenarioFrameworkTask.GetSlotTask();
				if (!scenarioFrameworkSlotTask)
					continue;
				
				scenarioFrameworkLayerTask = scenarioFrameworkTask.GetLayerTask();
				if (!scenarioFrameworkLayerTask)
					continue;
				
				scenarioFrameworkArea = scenarioFrameworkLayerTask.GetParentArea();
				if (!scenarioFrameworkArea)
					continue;

				listElements.Insert(WidgetManager.Translate(string.Format("%1 - Area: %2 - LayerTask: %3 - SlotTask: %4", scenarioFrameworkTask.GetTaskName(), scenarioFrameworkArea.GetName(), scenarioFrameworkLayerTask.GetName(), scenarioFrameworkSlotTask.GetName())));
				scenarioFrameworkTasks.Insert(scenarioFrameworkTask);
			}

			if (!listElements.IsEmpty())
			{
				int taskListIndex;
				DbgUI.List("Active Tasks", taskListIndex, listElements);

				scenarioFrameworkTask = scenarioFrameworkTasks[taskListIndex];

				bool inspectArea;
				DbgUI.Check("Inspect Area", inspectArea);
				if (inspectArea)
					LayerInspector(scenarioFrameworkTask.GetLayerTask().GetParentArea().GetName());

				bool inspectLayerTask;
				DbgUI.Check("Inspect Layer Task", inspectLayerTask);
				if (inspectLayerTask)
					LayerInspector(scenarioFrameworkTask.GetLayerTask().GetName());

				bool inspectSlotTask;
				DbgUI.Check("Inspect Slot Task", inspectSlotTask);
				if (inspectSlotTask)
					LayerInspector(scenarioFrameworkTask.GetSlotTask().GetName());

				ProcessSlotTask(scenarioFrameworkTask.GetSlotTask());

				DbgUI.Spacer(16);

				bool includeChildren = true;
				DbgUI.Check("Restore Include Children", includeChildren);

				bool reinitAfterRestoration = true;
				DbgUI.Check("Reinit After Restoration", reinitAfterRestoration);

				bool affectRandomization = false;
				DbgUI.Check("Restore Affects Randomization", affectRandomization);

				if (DbgUI.Button("Restore To Default"))
					scenarioFrameworkTask.GetLayerTask().RestoreToDefault(includeChildren, reinitAfterRestoration, affectRandomization);
			}
		}
		DbgUI.End();
	}

	//------------------------------------------------------------------------------------------------
	//! Registers scenario areas, lists them, allows inspection.
	static void RegisteredAreas()
	{
		DbgUI.Begin("ScenarioFramework Registered Areas");
		{

			SCR_ScenarioFrameworkSystem scenarioFrameworkSystem = SCR_ScenarioFrameworkSystem.GetInstance();
			if (!scenarioFrameworkSystem)
				return;

			array<string> listElements = {};
			foreach (SCR_ScenarioFrameworkArea area : scenarioFrameworkSystem.m_aAreas)
			{
				listElements.Insert(area.GetName());
			}

			if (!listElements.IsEmpty())
			{
				int selectedAreaIndex;
				DbgUI.List("Areas", selectedAreaIndex, listElements);

				bool inspectArea;
				DbgUI.Check("Inspect Area", inspectArea);
				if (inspectArea)
				{
					string layerName = listElements[selectedAreaIndex];
					LayerInspector(layerName);
				}
			}
		}
		DbgUI.End();
	}

	//------------------------------------------------------------------------------------------------
	//! The method displays debug actions prepared for scenario
	static void DebugActions()
	{
		SCR_ScenarioFrameworkSystem scenarioFrameworkSystem = SCR_ScenarioFrameworkSystem.GetInstance();
		if (!scenarioFrameworkSystem)
			return;
		
		DbgUI.Begin("ScenarioFramework Debug Actions");
		{
			array<string> listElements = {};
			if (!scenarioFrameworkSystem.m_aDebugActions || scenarioFrameworkSystem.m_aDebugActions.IsEmpty())
				return;
			
			foreach (SCR_ScenarioFrameworkDebugAction action : scenarioFrameworkSystem.m_aDebugActions)
			{
				listElements.Insert(action.m_sDebugActionName);
			}
			
			if (listElements.IsEmpty())
				return;
			
			int selectedActionIndex;
			DbgUI.List("Available actions", selectedActionIndex, listElements);
			
			if (scenarioFrameworkSystem.m_aDebugActions.IsIndexValid(selectedActionIndex))
				ProcessDebugAction(scenarioFrameworkSystem.m_aDebugActions[selectedActionIndex]);
		}
		DbgUI.End();
	}
	
	//------------------------------------------------------------------------------------------------
	static void ProcessDebugAction(SCR_ScenarioFrameworkDebugAction debugAction)
	{
		if (!DbgUI.Button("ACTIVATE"))
			return;
		
		foreach (SCR_ScenarioFrameworkActionBase action : debugAction.m_aDebugActions)
		{
			action.Init(null);
			action.OnActivate(null);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Debugs areas UI for managing scenario framework debug areas, including adding, removing, and clearing them.
	static void DebugAreas()
	{
		DbgUI.Begin("ScenarioFramework Debug Areas");
		{
			SCR_ScenarioFrameworkSystem scenarioFrameworkSystem = SCR_ScenarioFrameworkSystem.GetInstance();
			if (!scenarioFrameworkSystem)
				return;

			array<string> listElements = {};
			foreach (SCR_ScenarioFrameworkDebugArea debugArea : scenarioFrameworkSystem.m_aDebugAreas)
			{
				listElements.Insert(string.Format("%1 - LayerTask: %2 - SlotTask: %3", debugArea.m_sForcedArea, debugArea.m_sForcedLayerTask, debugArea.m_sForcedSlotTask));
			}

			if (!listElements.IsEmpty())
			{
				int selectedAreaIndex;
				DbgUI.List("Debug Areas", selectedAreaIndex, listElements);

				string toRemoveDebugArea = scenarioFrameworkSystem.m_aDebugAreas[selectedAreaIndex].m_sForcedArea;
				if (DbgUI.Button(string.Format("Remove Debug Area: %1", toRemoveDebugArea)))
					RemoveDebugArea(toRemoveDebugArea);
				
				if (DbgUI.Button("Clear All Debug Areas"))
					scenarioFrameworkSystem.m_aDebugAreas.Clear();
			}

			DbgUI.Spacer(16);
			DbgUI.Text("New Debug Area");
			string newDebugArea;
			string newDebugLayerTask;
			string newDebugSlotTask;
			DbgUI.InputText("Area (Requiered)", newDebugArea);
			DbgUI.InputText("Layer Task (Optional)", newDebugLayerTask);
			DbgUI.InputText("Slot Task (Optional)", newDebugSlotTask);
			if (DbgUI.Button("Add Debug Area"))
					AddNewDebugArea(newDebugArea, newDebugLayerTask, newDebugSlotTask);
			
			// This part is currently not working properly and is under investigation
			DbgUI.Spacer(16);
			bool possiblePresets;
			DbgUI.Check("Show possible Debug Area Presets", possiblePresets);
			if (possiblePresets)
			{
				DbgUI.Text("All possible Debug Area Presets");
				array<string> listPossibleElements = {};
				array<ref array<ref SCR_ScenarioFrameworkDebugArea>> allDebugArea = {};
				allDebugArea = SCR_ScenarioFrameworkDebug.GetDebugAreaPresets();
				
				foreach (int index, array<ref SCR_ScenarioFrameworkDebugArea> debugArea : allDebugArea)
				{
					listPossibleElements.Insert(string.Format("Debug Preset %1", index));
				}
				
				if (!listPossibleElements.IsEmpty())
				{
					int selectedPresetIndex;
					DbgUI.List("Debug Area Presets", selectedPresetIndex, listPossibleElements);
					
					array<string> listSelectedPresetElements = {};
					foreach (SCR_ScenarioFrameworkDebugArea debugSelectedPresetArea : allDebugArea[selectedPresetIndex])
					{
						listSelectedPresetElements.Insert(string.Format("%1 - LayerTask: %2 - SlotTask: %3", debugSelectedPresetArea.m_sForcedArea, debugSelectedPresetArea.m_sForcedLayerTask, debugSelectedPresetArea.m_sForcedSlotTask));
					}
		
					if (!listSelectedPresetElements.IsEmpty())
					{
						int selectedPresetListIndex;
						DbgUI.List("Selected Preset Details", selectedPresetListIndex, listSelectedPresetElements);
					}

					if (DbgUI.Button(string.Format("Add Debug Preset %1", selectedPresetIndex)))
					{
						foreach (SCR_ScenarioFrameworkDebugArea newSelectedDebugArea : allDebugArea[selectedPresetIndex])
						{
							AddNewDebugArea(newSelectedDebugArea)
						}
					}
				}
			}

			DbgUI.Spacer(16);
			DbgUI.Text("Remove Debug Area Removal");

			if (DbgUI.Button("Reinit ScenarioFramework"))
				ReinitScenarioFramework(scenarioFrameworkSystem);
		}
		DbgUI.End();
	}

	//------------------------------------------------------------------------------------------------
	//! Resets scenario framework, clears all data, restores default for non-core areas, then initializes it again.
	//! \param[in] scenarioFrameworkSystem 
	static void ReinitScenarioFramework(SCR_ScenarioFrameworkSystem scenarioFrameworkSystem)
	{
		if (!scenarioFrameworkSystem)
			return;
		
		SCR_ScenarioFrameworkSystem.GetCallQueuePausable().Clear();
		scenarioFrameworkSystem.m_bDebugInit = true;
		scenarioFrameworkSystem.m_aSelectedAreas.Clear();
		scenarioFrameworkSystem.m_aLayerTasksToBeInitialized.Clear();
		scenarioFrameworkSystem.m_aLayerTasksForRandomization.Clear();
		scenarioFrameworkSystem.m_aAreasTasksToSpawn.Clear();
		scenarioFrameworkSystem.m_aLayersTaskToSpawn.Clear();
		scenarioFrameworkSystem.m_aSlotsTaskToSpawn.Clear();
		scenarioFrameworkSystem.m_aESFTaskTypesAvailable.Clear();
		scenarioFrameworkSystem.m_aESFTaskTypeForRandomization.Clear();
		scenarioFrameworkSystem.m_aSpawnedAreas.Clear();
		scenarioFrameworkSystem.m_aDespawnedAreas.Clear();
		scenarioFrameworkSystem.m_mVariableMap.Clear();

		foreach (SCR_ScenarioFrameworkArea registeredArea : scenarioFrameworkSystem.m_aAreas)
		{
			if (!scenarioFrameworkSystem.m_aCoreAreas.Contains(registeredArea.GetName()))
				registeredArea.RestoreToDefault(true);
		}

		scenarioFrameworkSystem.Init();
	}

	//------------------------------------------------------------------------------------------------
	//! Adds new debug area with specified parameters to scenario framework system.
	//! \param[in] newDebugArea Represents the rea for new debug area in scenario framework system.
	//! \param[in] newDebugLayerTask Represents the layer task for new debug area in scenario framework system.
	//! \param[in] newDebugSlotTask Represents the specific task within the debug layer for the new debug area.
	static void AddNewDebugArea(string newDebugArea, string newDebugLayerTask, string newDebugSlotTask)
	{
		SCR_ScenarioFrameworkSystem scenarioFrameworkSystem = SCR_ScenarioFrameworkSystem.GetInstance();
		if (!scenarioFrameworkSystem)
			return;

		foreach (SCR_ScenarioFrameworkDebugArea debugArea : scenarioFrameworkSystem.m_aDebugAreas)
		{
			// If there is already the same area inserted, we don't want to continue
			if (debugArea.m_sForcedArea == newDebugArea)
				return;
		}

		SCR_ScenarioFrameworkDebugArea debugArea = new SCR_ScenarioFrameworkDebugArea();
		debugArea.m_sForcedArea = newDebugArea;
		debugArea.m_sForcedLayerTask = newDebugLayerTask;
		debugArea.m_sForcedSlotTask = newDebugSlotTask;

		scenarioFrameworkSystem.m_aDebugAreas.Insert(debugArea);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Adds new debug area to scenario framework system, checks for forced area duplicates, inserts if not present.
	//! \param[in] newDebugArea
	static void AddNewDebugArea(SCR_ScenarioFrameworkDebugArea newDebugArea)
	{
		SCR_ScenarioFrameworkSystem scenarioFrameworkSystem = SCR_ScenarioFrameworkSystem.GetInstance();
		if (!scenarioFrameworkSystem)
			return;

		foreach (SCR_ScenarioFrameworkDebugArea debugArea : scenarioFrameworkSystem.m_aDebugAreas)
		{
			// If there is already the same area inserted, we don't want to continue
			if (debugArea.m_sForcedArea == newDebugArea.m_sForcedArea)
				return;
		}
		
		scenarioFrameworkSystem.m_aDebugAreas.Insert(newDebugArea);
	}

	//------------------------------------------------------------------------------------------------
	//! Removes debug area with specified name from scenario framework system.
	//! \param[in] toRemoveDebugArea Target Area.
	static void RemoveDebugArea(string toRemoveDebugArea)
	{
		SCR_ScenarioFrameworkSystem scenarioFrameworkSystem = SCR_ScenarioFrameworkSystem.GetInstance();
		if (!scenarioFrameworkSystem)
			return;

		for (int i = scenarioFrameworkSystem.m_aDebugAreas.Count() - 1; i >= 0; i--)
		{
			if (scenarioFrameworkSystem.m_aDebugAreas[i].m_sForcedArea == toRemoveDebugArea)
				scenarioFrameworkSystem.m_aDebugAreas.Remove(i);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return an array of debug area presets for scenario framework debug areas.
	static array<ref array<ref SCR_ScenarioFrameworkDebugArea>> GetDebugAreaPresets()
	{
		SCR_ScenarioFrameworkSystem scenarioFrameworkSystem = SCR_ScenarioFrameworkSystem.GetInstance();
		if (!scenarioFrameworkSystem)
			return null;
	
		array<ref array<ref SCR_ScenarioFrameworkDebugArea>> allDebugArea = {};
		
	
		array<SCR_ScenarioFrameworkSlotTask> aSlotTasks = {};
		foreach (SCR_ScenarioFrameworkArea area : scenarioFrameworkSystem.m_aAreas)
		{
			area.GetAllSlotTasks(aSlotTasks);
			if (!aSlotTasks || aSlotTasks.IsEmpty())
				continue;
			
			foreach (SCR_ScenarioFrameworkSlotTask slotTask : aSlotTasks)
			{
				if (!slotTask)
					continue;
				
				SCR_ScenarioFrameworkDebugArea debugArea = new SCR_ScenarioFrameworkDebugArea();
				debugArea.m_sForcedArea = area.GetName();
				debugArea.m_sForcedLayerTask = slotTask.GetLayerTask().GetName();
				debugArea.m_sForcedSlotTask = slotTask.GetName();
				
				bool newAreaCreated;
				array<ref SCR_ScenarioFrameworkDebugArea> temporaryDebugAreas = {};
				
				if (allDebugArea.IsEmpty())
				{
					temporaryDebugAreas.Insert(debugArea);
					allDebugArea.Insert(temporaryDebugAreas);
					continue;
				}
				
				array<ref SCR_ScenarioFrameworkDebugArea> temporaryDebugAreas2 = {};
				foreach (array<ref SCR_ScenarioFrameworkDebugArea> debugAreasFromAll : allDebugArea)
				{
					bool unusedAreaFound = true;
					foreach (SCR_ScenarioFrameworkDebugArea innerDebugArea : debugAreasFromAll)
					{
						if (innerDebugArea.m_sForcedArea == area.GetName())
						{
							unusedAreaFound = false;
							break;
						}
					}
					
					if (unusedAreaFound)
					{
						debugAreasFromAll.Insert(debugArea);
						break;
					}
					else
					{
						temporaryDebugAreas2.Insert(debugArea);
						newAreaCreated = true;
					}
				}
				
				if (newAreaCreated)
				{
					allDebugArea.Insert(temporaryDebugAreas2);
				}
			}
		}
		
		SCR_ScenarioFrameworkDebugArea debugArea;
		for (int i = allDebugArea.Count() - 1; i >= 0; i--)
		{
			array<ref SCR_ScenarioFrameworkDebugArea> debugAreasFromAll = allDebugArea[i];
			
			for (int j = debugAreasFromAll.Count() - 1; j >= 0; j--)
			{
				debugArea = debugAreasFromAll[j];
				foreach (array<ref SCR_ScenarioFrameworkDebugArea> debugAreasFromAll2 : allDebugArea)
				{
					bool unusedAreaFound = true;
					foreach (SCR_ScenarioFrameworkDebugArea innerDebugArea : debugAreasFromAll2)
					{
						if (innerDebugArea.m_sForcedArea == debugArea.m_sForcedArea)
						{
							unusedAreaFound = false;
							break;
						}
					}
					
					if (unusedAreaFound)
					{
						debugAreasFromAll2.Insert(debugArea);
						debugAreasFromAll.Remove(j);
						break;
					}
				}
			}
			
			if (debugAreasFromAll.IsEmpty())
				allDebugArea.Remove(i);	
		}
		
		return allDebugArea;
	}

	//------------------------------------------------------------------------------------------------
	//! Allows inspection of layer details
	//! \param[in] inputLayerName Specifies the name of the scenario framework layer to inspect.
	static void LayerInspector(string inputLayerName = "")
	{
		DbgUI.Begin(string.Format("ScenarioFramework Layer Inspector %1", inputLayerName));
		{
			if (SCR_StringHelper.IsEmptyOrWhiteSpace(inputLayerName))
				DbgUI.InputText("Layer Name to inspect", inputLayerName);

			IEntity layerEntity = GetGame().GetWorld().FindEntityByName(inputLayerName);
			if (layerEntity)
			{
				bool layerTaskHasFinishConditions;
				bool layerTaskHasFinishActions;
				bool layerTaskHasCreateActions;
				bool layerTaskHasFailedActions;
				bool layerTaskHasCancelledActions;
				bool layerTaskHasProgressActions;
				bool layerTaskHasAssignedActions;
				
				
				bool slotTaskHasFinishConditions;
				bool slotTaskHasFinishActions;
				bool slotTaskHasCreateActions;
				bool slotTaskHasFailedActions;
				bool slotTaskHasCancelledActions;
				bool slotTaskHasProgressActions;
				bool slotTaskHasAssignedActions;
				SCR_ScenarioFrameworkLayerBase layerBase = SCR_ScenarioFrameworkLayerBase.Cast(layerEntity.FindComponent(SCR_ScenarioFrameworkLayerBase));
				if (layerBase)
				{
					bool detailsChecked = true;
					if (detailsChecked)
					{
						if (layerBase.m_bInitiated)
							DbgUI.Text("Initiated");
						else
							DbgUI.Text("Not Initiated");

						DbgUI.Text((string.Format("Activation type: %1", SCR_Enum.GetEnumName(SCR_ScenarioFrameworkEActivationType, layerBase.m_eActivationType))));

						if (layerBase.m_bIsTerminated)
							DbgUI.Text("Terminated");
						else
							DbgUI.Text("Not Terminated");

						if (layerBase.m_iCurrentlySpawnedChildren < layerBase.m_iSupposedSpawnedChildren)
						{
							DbgUI.Text(string.Format("Currently spawned %1 out of %2 children", layerBase.m_iCurrentlySpawnedChildren, layerBase.m_iSupposedSpawnedChildren));
						}

						if (layerBase.m_Area)
							DbgUI.Text((string.Format("Parent Area Name: %1", layerBase.m_Area.GetName())));

						if (layerBase.m_ParentLayer)
							DbgUI.Text((string.Format("Parent Layer Name: %1", layerBase.m_ParentLayer.GetName())));
					}

					SCR_ScenarioFrameworkLayerTask layerTask = SCR_ScenarioFrameworkLayerTask.Cast(layerBase);
					if (layerTask)
					{
						ProcessSlotTask(layerTask.m_SlotTask);
						if ((layerTask.m_aFinishConditions && !layerTask.m_aFinishConditions.IsEmpty()))
							layerTaskHasFinishConditions = true;
						
						if ((layerTask.m_aTriggerActionsOnFinish && !layerTask.m_aTriggerActionsOnFinish.IsEmpty()))
							layerTaskHasFinishActions = true;
						
						if ((layerTask.m_aActionsOnCreated && !layerTask.m_aActionsOnCreated.IsEmpty()))
							layerTaskHasCreateActions = true;

						if ((layerTask.m_aActionsOnFailed && !layerTask.m_aActionsOnFailed.IsEmpty()))
							layerTaskHasFailedActions = true;
						
						if ((layerTask.m_aActionsOnCancelled && !layerTask.m_aActionsOnCancelled.IsEmpty()))
							layerTaskHasCancelledActions = true;

						if ((layerTask.m_aActionsOnProgress && !layerTask.m_aActionsOnProgress.IsEmpty()))
							layerTaskHasProgressActions = true;

						if ((layerTask.m_aActionsOnAssigned && !layerTask.m_aActionsOnAssigned.IsEmpty()))
							layerTaskHasAssignedActions = true;
					}

					SCR_ScenarioFrameworkSlotBase slotBase = SCR_ScenarioFrameworkSlotBase.Cast(layerBase);
					if (slotBase)
					{
						string spawnedEntityDisplayName;
						spawnedEntityDisplayName = slotBase.GetSpawnedEntityDisplayName();
						if (!SCR_StringHelper.IsEmptyOrWhiteSpace(spawnedEntityDisplayName))
							DbgUI.Text((string.Format("Spawned Entity Display Name: %1", spawnedEntityDisplayName)));

						IEntity spawnedEntity = slotBase.GetSpawnedEntity();

						BaseGameTriggerEntity baseGameTrigger = BaseGameTriggerEntity.Cast(spawnedEntity);
						if (baseGameTrigger)
						{
							if (baseGameTrigger.IsPeriodicQueriesEnabled())
							{
								DbgUI.Text("Trigger Periodic Queries Enabled");
								if (DbgUI.Button("Disable Periodic Queries"))
									baseGameTrigger.EnablePeriodicQueries(false);
							}
							else
							{
								DbgUI.Text("Trigger Periodic Queries Disabled");
								if (DbgUI.Button("Enable Periodic Queries"))
									baseGameTrigger.EnablePeriodicQueries(true);
							}
						}

						SCR_ScenarioFrameworkSlotTask slotTask = SCR_ScenarioFrameworkSlotTask.Cast(slotBase);
						if (slotTask)
						{
							ProcessSlotTask(slotTask);
							if ((slotTask.m_aFinishConditions && !slotTask.m_aFinishConditions.IsEmpty()))
								slotTaskHasFinishConditions = true;

							if ((slotTask.m_aActionsOnFinished && !slotTask.m_aActionsOnFinished.IsEmpty()))
								slotTaskHasFinishActions = true;

							if ((slotTask.m_aActionsOnCreated && !slotTask.m_aActionsOnCreated.IsEmpty()))
								slotTaskHasCreateActions = true;

							if ((slotTask.m_aActionsOnFailed && !slotTask.m_aActionsOnFailed.IsEmpty()))
								slotTaskHasFailedActions = true;
							
							if ((slotTask.m_aActionsOnCancelled && !slotTask.m_aActionsOnCancelled.IsEmpty()))
								slotTaskHasCancelledActions = true;

							if ((slotTask.m_aActionsOnProgress && !slotTask.m_aActionsOnProgress.IsEmpty()))
								slotTaskHasProgressActions = true;

							if ((slotTask.m_aActionsOnAssigned && !slotTask.m_aActionsOnAssigned.IsEmpty()))
								slotTaskHasAssignedActions = true;
						}
						else if (spawnedEntity)
						{
							ProcessTeleport(spawnedEntity.GetOrigin(), "Teleport to Spawned Entity");
						}

						if (layerBase.m_aPlugins && !layerBase.m_aPlugins.IsEmpty())
						{
							bool inspectPlugins;
							DbgUI.Check("Inspect Plugins", inspectPlugins);
							if (inspectPlugins)
								PluginInspector(inputLayerName);
						}
					}
					else
					{
						array<string> childrenLayerElements = {};
						ProcessLayerHierarchy(layerBase, childrenLayerElements);

						if (!childrenLayerElements.IsEmpty())
						{
							DbgUI.Spacer(16);

							DbgUI.Text("Layer Hierarchy:");
							int childLayerIndex;
							DbgUI.List("Layer Hierarchy", childLayerIndex, childrenLayerElements);

							bool inspectChildLayer;
							DbgUI.Check("Inspect Child Layer", inspectChildLayer);
							if (inspectChildLayer)
							{
								string layerName = childrenLayerElements[childLayerIndex];
								layerName.Replace("-", "");
								LayerInspector(layerName);
							}
						}

						array<string> logicLayerElements = {};
						array<SCR_ScenarioFrameworkLogic> m_aLogic = {};
						layerBase.GetLogics(m_aLogic);
						foreach (SCR_ScenarioFrameworkLogic logic : m_aLogic)
						{
							logicLayerElements.Insert(logic.GetName());
						}

						if (!logicLayerElements.IsEmpty())
						{
							DbgUI.Text("Layer Logics:");
							int logicIndex;
							DbgUI.List("Logics Hierarchy", logicIndex, logicLayerElements);

							bool inspectLogic;
							DbgUI.Check("Inspect Logic", inspectLogic);
							if (inspectLogic)
							{
								string logicName = logicLayerElements[logicIndex];
								LogicInspector(logicName);
							}
						}

						if (layerBase.m_aPlugins && !layerBase.m_aPlugins.IsEmpty())
						{
							bool inspectPlugins;
							DbgUI.Check("Inspect Plugins", inspectPlugins);
							if (inspectPlugins)
								PluginInspector(inputLayerName);
						}
					}

					if ((layerBase.m_aActivationActions && !layerBase.m_aActivationActions.IsEmpty()) || layerTaskHasFinishActions 
						|| layerTaskHasCreateActions || layerTaskHasFailedActions || layerTaskHasCancelledActions || layerTaskHasProgressActions || layerTaskHasAssignedActions || slotTaskHasFinishActions
						|| slotTaskHasCreateActions || slotTaskHasFailedActions || slotTaskHasCancelledActions || slotTaskHasProgressActions || slotTaskHasAssignedActions
						)
					{
						bool inspectActivationActions;
						DbgUI.Check("Inspect Actions", inspectActivationActions);
						if (inspectActivationActions)
							ActionInspector(inputLayerName);
					}

					if ((layerBase.m_aActivationConditions && !layerBase.m_aActivationConditions.IsEmpty()) || slotTaskHasFinishConditions || layerTaskHasFinishConditions)
					{
						bool inspectActivationConditions;
						DbgUI.Check("Inspect Conditions", inspectActivationConditions);
						if (inspectActivationConditions)
							ConditionInspector(inputLayerName);
					}

					DbgUI.Spacer(16);
					ProcessTeleport(layerBase.GetOwner().GetOrigin(), "Teleport to Layer");

					if (layerBase.m_bInitiated)
					{
						if (DbgUI.Button("Initiate Dynamic Despawn"))
							layerBase.DynamicDespawn(layerBase);
					}
					else
					{
						if (DbgUI.Button("Initiate Dynamic Spawn"))
							layerBase.DynamicReinit();
					}

					DbgUI.Spacer(16);

					bool includeChildren = true;
					DbgUI.Check("Restore Include Children", includeChildren);

					bool reinitAfterRestoration = true;
					DbgUI.Check("Reinit After Restoration", reinitAfterRestoration);

					bool affectRandomization = true;
					DbgUI.Check("Restore Affects Randomization", affectRandomization);

					if (DbgUI.Button("Restore To Default"))
						layerBase.RestoreToDefault(includeChildren, reinitAfterRestoration, affectRandomization);
				}
			}
		}
		DbgUI.End();
	}

	//------------------------------------------------------------------------------------------------
	//! Processes layer hierarchy by recursively calling itself with children layers, adding their names with indentation.
	//! \param[in] layerBase Processes child layers of given layerBase
	//! \param[in,out] childrenLayerElements Containins names of child layers in the hierarchy.
	//! \param[in] level Indentation based on level.
	static void ProcessLayerHierarchy(SCR_ScenarioFrameworkLayerBase layerBase, inout array<string> childrenLayerElements, int level = 0)
	{
		string levelString;
		for (int i = 0; i < level; i++)
		{
			levelString += "-";
		}

		array<SCR_ScenarioFrameworkLayerBase> childrenLayers = {};
		layerBase.GetChildren(childrenLayers);

		childrenLayers.RemoveItem(null);
		foreach (SCR_ScenarioFrameworkLayerBase layerChild : childrenLayers)
		{
			childrenLayerElements.Insert(string.Format("%1%2", levelString, layerChild.GetName()));
			ProcessLayerHierarchy(layerChild, childrenLayerElements, level + 1);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Processes slot tasks, checks their state, finishes tasks if not finished, finished, or cancelled, and handles spawned entities
	//! \param[in] slotTask Represents a slot task in the scenario framework, used for managing tasks within the scenario.
	static void ProcessSlotTask(SCR_ScenarioFrameworkSlotTask slotTask)
	{
		if (!slotTask)
			return;

		SCR_ETaskState state = slotTask.GetTaskState();
		DbgUI.Text((string.Format("Task State: %1", SCR_Enum.GetEnumName(SCR_ETaskState, state))));

		if (state != SCR_ETaskState.COMPLETED && state != SCR_ETaskState.FAILED && state != SCR_ETaskState.CANCELLED)
		{
			if (DbgUI.Button("Finish Task"))
			{
				SCR_ScenarioFrameworkLayerTask layerTask = slotTask.GetLayerTask();
				if (layerTask)
				{
					layerTask.ProcessLayerTaskState(SCR_ETaskState.COMPLETED);
				}
			}
			
			if (DbgUI.Button("Finish Task (Forced)"))
			{
				SCR_ScenarioFrameworkLayerTask layerTask = slotTask.GetLayerTask();
				if (layerTask)
				{
					layerTask.ProcessLayerTaskState(SCR_ETaskState.COMPLETED, true);
				}
			}
		}

		IEntity spawnedEntity = slotTask.GetSpawnedEntity();
		if (spawnedEntity)
			ProcessTeleport(spawnedEntity.GetOrigin(), "Teleport to Spawned Entity");
	}

	//------------------------------------------------------------------------------------------------
	//! Processes teleportation for local player character when button is pressed, setting destination origin.
	//! \param[in] destination Destination is the target location for teleportation.
	//! \param[in] buttonName ButtonName is the name of the button used to trigger teleportation action.
	static void ProcessTeleport(vector destination, string buttonName)
	{
		if (DbgUI.Button(buttonName))
		{
			ChimeraCharacter char = ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity());
			if (char)
				char.SetOrigin(destination)
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! The method displays actions associated with a specified scenario layer in an inspector interface.
	//! \param[in] inputLayerName Input Layer Name represents the name of the scenario layer to inspect for its actions in the ScenarioFramework.
	static void ActionInspector(string inputLayerName = "")
	{
		DbgUI.Begin(string.Format("ScenarioFramework Action Inspector %1", inputLayerName));
		{
			if (SCR_StringHelper.IsEmptyOrWhiteSpace(inputLayerName))
				DbgUI.InputText("Layer Name to inspect", inputLayerName);

			IEntity layerEntity = GetGame().GetWorld().FindEntityByName(inputLayerName);
			if (layerEntity)
			{
				SCR_ScenarioFrameworkLayerBase layerBase = SCR_ScenarioFrameworkLayerBase.Cast(layerEntity.FindComponent(SCR_ScenarioFrameworkLayerBase));
				if (layerBase)
				{
					PrepareActionStrings(layerBase.m_aActivationActions, layerEntity, "Activation Actions");

					SCR_ScenarioFrameworkLayerTask layerTask = SCR_ScenarioFrameworkLayerTask.Cast(layerBase);
					if (layerTask)
					{
						PrepareActionStrings(layerTask.m_aTriggerActionsOnFinish, layerEntity, "Finish Actions");
						PrepareActionStrings(layerTask.m_aActionsOnCreated, layerEntity, "Created Actions");
						PrepareActionStrings(layerTask.m_aActionsOnFailed, layerEntity, "Failed Actions");
						PrepareActionStrings(layerTask.m_aActionsOnCancelled, layerEntity, "Cancelled Actions");
						PrepareActionStrings(layerTask.m_aActionsOnProgress, layerEntity, "Progress Actions");
						PrepareActionStrings(layerTask.m_aActionsOnAssigned, layerEntity, "Assigned Actions");
					}

					SCR_ScenarioFrameworkSlotTask slotTask = SCR_ScenarioFrameworkSlotTask.Cast(layerBase);
					if (slotTask)
					{
						PrepareActionStrings(slotTask.m_aActionsOnFinished, layerEntity, "Finish Actions");
						PrepareActionStrings(slotTask.m_aActionsOnCreated, layerEntity, "Created Actions");
						PrepareActionStrings(slotTask.m_aActionsOnFailed, layerEntity, "Failed Actions");
						PrepareActionStrings(slotTask.m_aActionsOnCancelled, layerEntity, "Cancelled Actions");
						PrepareActionStrings(slotTask.m_aActionsOnProgress, layerEntity, "Progress Actions");
						PrepareActionStrings(slotTask.m_aActionsOnAssigned, layerEntity, "Assigned Actions");
					}
				}
			}
		}
		DbgUI.End();
	}

	//------------------------------------------------------------------------------------------------
	//! This method displays debug information about an action and allows certain manipulations
	//! \param[in] action Target action.
	//! \param[in] layerEntity Layer entity represents an in-game object or entity that is associated with the action being processed in the method.
	//! \param[in] categoryName CategoryName represents the name of the action being debugged or inspected in the method.
	static void ProcessAction(SCR_ScenarioFrameworkActionBase action, IEntity layerEntity, string categoryName)
	{
		if (action.m_bDebug)
		{
			DbgUI.Text(string.Format("%1 Debug Action enabled", categoryName));

			if (DbgUI.Button(string.Format("%1 Disable Action debug", categoryName)))
				action.m_bDebug = false;
		}
		else
		{
			DbgUI.Text(string.Format("%1 Debug Action disabled", categoryName));

			if (DbgUI.Button(string.Format("%1 Enable Action debug", categoryName)))
				action.m_bDebug = true;
		}

		DbgUI.Text(string.Format("%1 Number of activations %1 out of %2", categoryName, action.m_iNumberOfActivations, action.m_iMaxNumberOfActivations));

		if (DbgUI.Button(string.Format("%1 Init again", categoryName)))
			action.Init(layerEntity);

		if (DbgUI.Button(string.Format("%1 Activate again", categoryName)))
			action.OnActivate(layerEntity);

		array<ref SCR_ScenarioFrameworkActionBase> subActions = action.GetSubActions();
		if (!subActions || subActions.IsEmpty())
		{
			SCR_ScenarioFrameworkActionAI actionAI = SCR_ScenarioFrameworkActionAI.Cast(action);
			if (actionAI)
			{
				array<ref SCR_ScenarioFrameworkAIAction> subActionsAI = actionAI.GetSubActionsAI();
				if (!subActionsAI || subActionsAI.IsEmpty())
					return;

				bool debugCheck;
				DbgUI.Check(string.Format("%1 Show SubAction AI Inspector", categoryName), debugCheck);
				if (debugCheck)
					ProcessSubActions(actionAI, layerEntity);
			}

			SCR_ScenarioFrameworkActionMedical actionMedical = SCR_ScenarioFrameworkActionMedical.Cast(action);
			if (actionMedical)
			{
				array<ref SCR_ScenarioFrameworkMedicalAction> subActionsMedical = actionMedical.GetSubActionsMedical();
				if (!subActionsMedical || subActionsMedical.IsEmpty())
					return;

				bool debugCheck;
				DbgUI.Check(string.Format("%1 Show SubAction Medical Inspector", categoryName), debugCheck);
				if (debugCheck)
					ProcessSubActions(actionMedical, layerEntity);
			}

			return;
		}

		bool debugCheck;
		DbgUI.Check(string.Format("%1 Show SubAction Inspector", categoryName), debugCheck);
		if (debugCheck)
			ProcessSubActions(action, layerEntity);
	}

	//------------------------------------------------------------------------------------------------
	//! Enables/disables debugging for an action, provides buttons for action activation, and checks for sub-actions
	//! \param[in] action Target action.
	//! \param[in] layerEntity LayerEntity represents an in-game object or entity that is used as a reference for processing actions within the scenario framework.
	//! \param[in] categoryName CategoryName represents the name of the action category for debugging purposes in the method.
	static void ProcessAction(SCR_ScenarioFrameworkAIAction action, IEntity layerEntity, string categoryName)
	{
		if (action.m_bDebug)
		{
			DbgUI.Text(string.Format("%1 Debug Action enabled", categoryName));

			if (DbgUI.Button(string.Format("%1 Disable Action debug", categoryName)))
				action.m_bDebug = false;
		}
		else
		{
			DbgUI.Text(string.Format("%1 Debug Action disabled", categoryName));

			if (DbgUI.Button(string.Format("%1 Enable Action debug", categoryName)))
				action.m_bDebug = true;
		}

		if (DbgUI.Button(string.Format("%1 Activate again", categoryName)))
			action.OnActivate();

		array<ref SCR_ScenarioFrameworkActionBase> subActions = action.GetSubActions();
		if (!subActions || subActions.IsEmpty())
			return;

		bool debugCheck;
		DbgUI.Check(string.Format("%1 Show SubAction Inspector", categoryName), debugCheck);
		if (debugCheck)
			ProcessSubActions(action, layerEntity);
	}

	//------------------------------------------------------------------------------------------------
	//! Enables/disables debugging for medical actions, provides buttons for action activation, and checks for sub-actions
	//! \param[in] action Target action.
	//! \param[in] layerEntity LayerEntity represents an in-game object or entity that interacts with the medical action being processed.
	//! \param[in] categoryName CategoryName represents the name of the medical action being processed, used for debugging and enabling/disabling action debugging.
	static void ProcessAction(SCR_ScenarioFrameworkMedicalAction action, IEntity layerEntity, string categoryName)
	{
		if (action.m_bDebug)
		{
			DbgUI.Text(string.Format("%1 Debug Action enabled", categoryName));

			if (DbgUI.Button(string.Format("%1 Disable Action debug", categoryName)))
				action.m_bDebug = false;
		}
		else
		{
			DbgUI.Text(string.Format("%1 Debug Action disabled", categoryName));

			if (DbgUI.Button(string.Format("%1 Enable Action debug", categoryName)))
				action.m_bDebug = true;
		}

		if (DbgUI.Button(string.Format("%1 Activate again", categoryName)))
			action.OnActivate();

		array<ref SCR_ScenarioFrameworkActionBase> subActions = action.GetSubActions();
		if (!subActions || subActions.IsEmpty())
			return;

		bool debugCheck;
		DbgUI.Check(string.Format("%1 Show SubAction Inspector", categoryName), debugCheck);
		if (debugCheck)
			ProcessSubActions(action, layerEntity);
	}

	//------------------------------------------------------------------------------------------------
	//! Processes sub-actions of an action, iterates through sub-actions, prepares action strings, and displays their list
	//! \param[in] action Target action.
	//! \param[in] layerEntity Layer entity represents an in-game object or entity that is used as a reference for processing sub-actions within the given
	static void ProcessSubActions(SCR_ScenarioFrameworkActionBase action, IEntity layerEntity)
	{
		array<string> listElements = {};
		array<ref SCR_ScenarioFrameworkActionBase> subActions = action.GetSubActions();
		if (!subActions || subActions.IsEmpty())
			return;

		string name = action.ToString();
		name.Replace("SCR_ScenarioFrameworkAction", "");
		name.Replace("SCR_ScenarioFrameworkAIAction", "");
		name.Replace("SCR_ScenarioFrameworkMedicalAction", "");
		DbgUI.Begin(string.Format("SubAction %1", name));
		{
			PrepareActionStrings(subActions, layerEntity, "Sub Actions");
		}
		DbgUI.End();
	}

	//------------------------------------------------------------------------------------------------
	//! Processes sub-actions of an AI action, iterates through sub-actions, prepares action strings, and displays their list
	//! \param[in] action Target action.
	//! \param[in] layerEntity LayerEntity represents an in-game object or entity used as a target for sub-actions in the scenario action.
	static void ProcessSubActions(SCR_ScenarioFrameworkActionAI action, IEntity layerEntity)
	{
		array<string> listElements = {};
		array<ref SCR_ScenarioFrameworkAIAction> subActions = action.GetSubActionsAI();
		if (!subActions || subActions.IsEmpty())
			return;

		string name = action.ToString();
		name.Replace("SCR_ScenarioFrameworkAIAction", "");
		DbgUI.Begin(string.Format("SubAction %1", name));
		{
			PrepareActionStrings(subActions, layerEntity, "Sub Actions");
		}
		DbgUI.End();
	}

	//------------------------------------------------------------------------------------------------
	//! Processes sub-actions of an action, iterates through sub-actions, prepares action strings, and displays their list
	//! \param[in] action Target action.
	//! \param[in] layerEntity Layer entity represents an in-game object or entity that is used as a target or reference for the sub-actions within
	static void ProcessSubActions(SCR_ScenarioFrameworkAIAction action, IEntity layerEntity)
	{
		array<string> listElements = {};
		array<ref SCR_ScenarioFrameworkActionBase> subActions = action.GetSubActions();
		if (!subActions || subActions.IsEmpty())
			return;

		string name = action.ToString();
		name.Replace("SCR_ScenarioFrameworkAction", "");
		name.Replace("SCR_ScenarioFrameworkAIAction", "");
		name.Replace("SCR_ScenarioFrameworkMedicalAction", "");
		DbgUI.Begin(string.Format("SubAction %1", name));
		{
			PrepareActionStrings(subActions, layerEntity, "Sub Actions");
		}
		DbgUI.End();
	}

	//------------------------------------------------------------------------------------------------
	//! Processes sub-actions of a medical action, displays them in debug UI.
	//! \param[in] action Target action.
	//! \param[in] layerEntity LayerEntity represents the entity on which sub-actions of a medical action are processed.
	static void ProcessSubActions(SCR_ScenarioFrameworkActionMedical action, IEntity layerEntity)
	{
		array<string> listElements = {};
		array<ref SCR_ScenarioFrameworkMedicalAction> subActions = action.GetSubActionsMedical();
		if (!subActions || subActions.IsEmpty())
			return;

		string name = action.ToString();
		name.Replace("SCR_ScenarioFrameworkMedicalAction", "");
		DbgUI.Begin(string.Format("SubAction %1", name));
		{
			PrepareActionStrings(subActions, layerEntity, "Sub Actions");
		}
		DbgUI.End();
	}

	//------------------------------------------------------------------------------------------------
	//! Processes sub-actions of a medical action, iterates through sub-actions, prepares action strings for sub-actions and lists them
	//! \param[in] action Target action.
	//! \param[in] layerEntity Layer entity represents the entity on which sub-actions of the given medical action are processed.
	static void ProcessSubActions(SCR_ScenarioFrameworkMedicalAction action, IEntity layerEntity)
	{
		array<string> listElements = {};
		array<ref SCR_ScenarioFrameworkActionBase> subActions = action.GetSubActions();
		if (!subActions || subActions.IsEmpty())
			return;

		string name = action.ToString();
		name.Replace("SCR_ScenarioFrameworkAction", "");
		name.Replace("SCR_ScenarioFrameworkAIAction", "");
		name.Replace("SCR_ScenarioFrameworkMedicalAction", "");
		DbgUI.Begin(string.Format("SubAction %1", name));
		{
			PrepareActionStrings(subActions, layerEntity, "Sub Actions");
		}
		DbgUI.End();
	}

	//------------------------------------------------------------------------------------------------
	//! Prepares action strings for category, lists them, and processes selected action if chosen.
	//! \param[in] actions Target actions.
	//! \param[in] activationEntity Activation entity represents the entity triggering the action selection process in the scenario framework.
	//! \param[in] categoryName CategoryName represents the name of the action category for which actions are being prepared for selection in the debug UI list.
	static void PrepareActionStrings(array<ref SCR_ScenarioFrameworkActionBase> actions, IEntity activationEntity, string categoryName)
	{
		array<string> listElements = {};
		if (!actions || actions.IsEmpty())
			return;

		foreach (int index, SCR_ScenarioFrameworkActionBase action : actions)
		{
			string name = action.ToString();
			name.Replace("SCR_ScenarioFrameworkAction", "");
			name.Replace("SCR_ScenarioFrameworkAIAction", "");
			name.Replace("SCR_ScenarioFrameworkMedicalAction", "");
			listElements.Insert(string.Format("%1 - %2", index, name));
		}

		if (listElements.IsEmpty())
			return;

		int selectedActionIndex;
		DbgUI.Text(string.Format("%1 Hierarchy:", categoryName));
		DbgUI.List(string.Format("%1", categoryName), selectedActionIndex, listElements);

		if (actions.IsIndexValid(selectedActionIndex))
			ProcessAction(actions[selectedActionIndex], activationEntity, categoryName);
	}

	//------------------------------------------------------------------------------------------------
	//! Prepares action strings for category, lists them, and processes selected action if chosen.
	//! \param[in] actions Target actions.
	//! \param[in] activationEntity Activation entity represents the entity triggering the action selection process in the scenario framework.
	//! \param[in] categoryName CategoryName represents the name of the action category for which actions are being prepared for selection in the debug UI list.
	static void PrepareActionStrings(array<ref SCR_ScenarioFrameworkAIAction> actions, IEntity activationEntity, string categoryName)
	{
		array<string> listElements = {};
		if (!actions || actions.IsEmpty())
			return;

		foreach (int index, SCR_ScenarioFrameworkAIAction action : actions)
		{
			string name = action.ToString();
			name.Replace("SCR_ScenarioFrameworkAction", "");
			name.Replace("SCR_ScenarioFrameworkAIAction", "");
			name.Replace("SCR_ScenarioFrameworkMedicalAction", "");
			listElements.Insert(string.Format("%1 - %2", index, name));
		}

		if (listElements.IsEmpty())
			return;

		int selectedActionIndex;
		DbgUI.Text(string.Format("%1 Hierarchy:", categoryName));
		DbgUI.List(string.Format("%1", categoryName), selectedActionIndex, listElements);

		if (actions.IsIndexValid(selectedActionIndex))
			ProcessAction(actions[selectedActionIndex], activationEntity, categoryName);
	}

	//------------------------------------------------------------------------------------------------
	//! Prepares action strings for medical actions, lists them, and processes selected action if chosen.
	//! \param[in] actions Target actions.
	//! \param[in] activationEntity Activation Entity represents the entity triggering the medical action selection process in the scenario.
	//! \param[in] categoryName CategoryName represents the name of the medical action category for which actions are being prepared for selection in the debug UI list.
	static void PrepareActionStrings(array<ref SCR_ScenarioFrameworkMedicalAction> actions, IEntity activationEntity, string categoryName)
	{
		array<string> listElements = {};
		if (!actions || actions.IsEmpty())
			return;

		foreach (int index, SCR_ScenarioFrameworkMedicalAction action : actions)
		{
			string name = action.ToString();
			name.Replace("SCR_ScenarioFrameworkAction", "");
			name.Replace("SCR_ScenarioFrameworkAIAction", "");
			name.Replace("SCR_ScenarioFrameworkMedicalAction", "");
			listElements.Insert(string.Format("%1 - %2", index, name));
		}

		if (listElements.IsEmpty())
			return;

		int selectedActionIndex;
		DbgUI.Text(string.Format("%1 Hierarchy:", categoryName));
		DbgUI.List(string.Format("%1", categoryName), selectedActionIndex, listElements);

		if (actions.IsIndexValid(selectedActionIndex))
			ProcessAction(actions[selectedActionIndex], activationEntity, categoryName);
	}

	//------------------------------------------------------------------------------------------------
	//! Logic inspector for scenario framework logic entities with debug, termination status, and counter interaction.
	//! \param[in] inputLogicName Input Logic Name is a string parameter representing the name of the logic entity to inspect in the ScenarioFramework Logic In
	static void LogicInspector(string inputLogicName = "")
	{
		DbgUI.Begin(string.Format("ScenarioFramework Logic Inspector %1", inputLogicName));
		{
			if (SCR_StringHelper.IsEmptyOrWhiteSpace(inputLogicName))
				DbgUI.InputText("Logic Name to inspect", inputLogicName);

			IEntity logicEntity = GetGame().GetWorld().FindEntityByName(inputLogicName);
			if (logicEntity)
			{
				SCR_ScenarioFrameworkLogic logic = SCR_ScenarioFrameworkLogic.Cast(logicEntity);
				if (logic)
				{
					if (logic.m_bDebug)
					{
						DbgUI.Text("Debug Logic enabled");

						if (DbgUI.Button("Disable Logic debug"))
							logic.m_bDebug = false;
					}
					else
					{
						DbgUI.Text("Debug Logic disabled");

						if (DbgUI.Button("Enable Logic debug"))
							logic.m_bDebug = true;
					}

					if (logic.m_bIsTerminated)
						DbgUI.Text("Terminated");
					else
						DbgUI.Text("Not Terminated");

					PrepareActionStrings(logic.m_aActions, logicEntity, "Actions");

					SCR_ScenarioFrameworkLogicCounter logicCounter = SCR_ScenarioFrameworkLogicCounter.Cast(logicEntity);
					if (logicCounter)
					{
						DbgUI.Text(string.Format("Logic Counter value %1 out of %2.", logicCounter.m_iCnt, logicCounter.m_iCountTo));

						PrepareActionStrings(logicCounter.m_aOnIncreaseActions, logicEntity, "OnIncrease Action");
						PrepareActionStrings(logicCounter.m_aOnDecreaseActions, logicEntity, "OnDecrease Action");

						if (DbgUI.Button("Increase Counter"))
							logicCounter.Increase(logicEntity);

						if (DbgUI.Button("Decrease Counter"))
							logicCounter.Decrease(logicEntity);
					}
				}
			}
		}
		DbgUI.End();
	}

	//------------------------------------------------------------------------------------------------
	//! Inspects Plugins for input layer
	//! \param[in] inputLayerName Input Layer Name is the name of the scenario layer to inspect in the ScenarioFramework Plugin Inspector.
	static void PluginInspector(string inputLayerName = "")
	{
		DbgUI.Begin(string.Format("ScenarioFramework Plugin Inspector %1", inputLayerName));
		{
			if (SCR_StringHelper.IsEmptyOrWhiteSpace(inputLayerName))
				DbgUI.InputText("Layer Name to inspect", inputLayerName);

			IEntity layerEntity = GetGame().GetWorld().FindEntityByName(inputLayerName);
			if (layerEntity)
			{
				SCR_ScenarioFrameworkLayerBase layerBase = SCR_ScenarioFrameworkLayerBase.Cast(layerEntity.FindComponent(SCR_ScenarioFrameworkLayerBase));
				if (layerBase)
				{
					PreparePluginStrings(layerBase.m_aPlugins, layerEntity, "Plugins");
				}
			}
		}
		DbgUI.End();
	}

	//------------------------------------------------------------------------------------------------
	//! Prepares plugin strings for selection in category, lists them, and processes selected one if valid index.
	//! \param[in] plugins
	//! \param[in] activationEntity Activation entity represents an in-game object that triggers the plugin selection process in the method.
	//! \param[in] categoryName CategoryName represents the name of the category for displaying plugin hierarchy in debug UI list.
	static void PreparePluginStrings(array<ref SCR_ScenarioFrameworkPlugin> plugins, IEntity activationEntity, string categoryName)
	{
		array<string> listElements = {};
		if (!plugins || plugins.IsEmpty())
			return;

		foreach (int index, SCR_ScenarioFrameworkPlugin action : plugins)
		{
			string name = action.ToString();
			name.Replace("SCR_ScenarioFrameworkPlugin", "");
			listElements.Insert(string.Format("%1 - %2", index, name));
		}

		if (listElements.IsEmpty())
			return;

		int selectedPluginIndex;
		DbgUI.Text(string.Format("%1 Hierarchy:", categoryName));
		DbgUI.List(string.Format("%1", categoryName), selectedPluginIndex, listElements);

		if (plugins.IsIndexValid(selectedPluginIndex))
			ProcessPlugin(plugins[selectedPluginIndex], activationEntity);
	}

	//------------------------------------------------------------------------------------------------
	//! The method enables or disables debugging for a plugin and checks for various plugin types.
	//! \param[in] plugin Target Plugin.
	//! \param[in] layerEntity Layer entity represents an in-game object or entity within the scenario for which the plugin is being processed.
	static void ProcessPlugin(SCR_ScenarioFrameworkPlugin plugin, IEntity layerEntity)
	{
		if (plugin.m_bDebug)
		{
			DbgUI.Text("Debug Plugin enabled");
			if (DbgUI.Button("Disable Plugin debug"))
				plugin.m_bDebug = false;
		}
		else
		{
			DbgUI.Text("Debug Plugin disabled");
			if (DbgUI.Button("Enable Plugin debug"))
				plugin.m_bDebug = true;
		}

		SCR_ScenarioFrameworkPluginTrigger pluginTrigger = SCR_ScenarioFrameworkPluginTrigger.Cast(plugin);
		if (pluginTrigger)
		{
			ProcessPluginTrigger(pluginTrigger);
			return;
		}

		SCR_ScenarioFrameworkPluginOnInventoryChange pluginOnInventoryChange = SCR_ScenarioFrameworkPluginOnInventoryChange.Cast(plugin);
		if (pluginOnInventoryChange)
		{
			ProcessPluginOnInventoryChange(pluginOnInventoryChange, layerEntity);
			return;
		}

		SCR_ScenarioFrameworkPluginSpawnPoint pluginSpawnPoint = SCR_ScenarioFrameworkPluginSpawnPoint.Cast(plugin);
		if (pluginSpawnPoint)
		{
			ProcessPluginSpawnPoint(pluginSpawnPoint, layerEntity);
			return;
		}

		SCR_ScenarioFrameworkPluginOnDestroyEvent pluginOnDestroyEvent = SCR_ScenarioFrameworkPluginOnDestroyEvent.Cast(plugin);
		if (pluginOnDestroyEvent)
		{
			ProcessPluginOnDestroyEvent(pluginOnDestroyEvent, layerEntity);
			return;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Controls periodic queries and forces trigger finish for an entity based on user input in debug UI.
	//! \param[in] pluginTrigger ProcessPluginTrigger handles interaction with scenario triggers, enabling or disabling periodic queries and optionally finishing it.
	static void ProcessPluginTrigger(SCR_ScenarioFrameworkPluginTrigger pluginTrigger)
	{
		IEntity entity = pluginTrigger.m_Object.GetSpawnedEntity();
		if (!entity)
			return;

		BaseGameTriggerEntity baseGameTrigger = BaseGameTriggerEntity.Cast(entity);
		if (!baseGameTrigger)
			return;

		if (baseGameTrigger.IsPeriodicQueriesEnabled())
		{
			DbgUI.Text("Periodic Queries Enabled");
			if (DbgUI.Button("Disable Periodic Queries"))
				baseGameTrigger.EnablePeriodicQueries(false);
		}
		else
		{
			DbgUI.Text("Periodic Queries Disabled");
			if (DbgUI.Button("Enable Periodic Queries"))
				baseGameTrigger.EnablePeriodicQueries(true);
		}

		SCR_ScenarioFrameworkTriggerEntity frameworkTrigger = SCR_ScenarioFrameworkTriggerEntity.Cast(entity);
		if (!frameworkTrigger)
			return;

		if (DbgUI.Button("Force Finish Trigger"))
			frameworkTrigger.FinishTrigger(entity);
	}

	//------------------------------------------------------------------------------------------------
	//! Prepares action strings for added/removed items in inventory layer entity.
	//! \param[in] pluginOnInventoryChange Target Plugin.
	//! \param[in] layerEntity LayerEntity represents an in-game object or entity in the scenario where inventory changes occur.
	static void ProcessPluginOnInventoryChange(SCR_ScenarioFrameworkPluginOnInventoryChange pluginOnInventoryChange, IEntity layerEntity)
	{
		PrepareActionStrings(pluginOnInventoryChange.m_aActionsOnItemAdded, layerEntity, "Actions On Item Added");
		PrepareActionStrings(pluginOnInventoryChange.m_aActionsOnItemRemoved, layerEntity, "Actions On Item Removed");
	}

	//------------------------------------------------------------------------------------------------
	//! Processes a spawn point plugin, assigning actions on spawn point used to an entity.
	//! \param[in] pluginSpawnPoint Target Plugin.
	//! \param[in] layerEntity LayerEntity represents an in-game object or entity used in the scenario.
	static void ProcessPluginSpawnPoint(SCR_ScenarioFrameworkPluginSpawnPoint pluginSpawnPoint, IEntity layerEntity)
	{
		PrepareActionStrings(pluginSpawnPoint.m_aActionsOnSpawnPointUsed, layerEntity, "Actions On Spawn Point Used");
	}

	//------------------------------------------------------------------------------------------------
	//! Processes plugin's actions on destroy event for given layer entity.
	//! \param[in] pluginOnDestroyEvent Target Plugin.
	//! \param[in] layerEntity LayerEntity represents an in-game object in the scenario being processed by the method.
	static void ProcessPluginOnDestroyEvent(SCR_ScenarioFrameworkPluginOnDestroyEvent pluginOnDestroyEvent, IEntity layerEntity)
	{
		PrepareActionStrings(pluginOnDestroyEvent.m_aActionsOnDestroy, layerEntity, "Actions On Destroy");
	}

	//------------------------------------------------------------------------------------------------
	//! Inspects scenario framework conditions for specified layer entity.
	//! \param[in] inputLayerName Input Layer Name represents the name of the scenario layer to inspect for its conditions in the ScenarioFramework.
	static void ConditionInspector(string inputLayerName = "")
	{
		DbgUI.Begin(string.Format("ScenarioFramework Condition Inspector %1", inputLayerName));
		{
			if (SCR_StringHelper.IsEmptyOrWhiteSpace(inputLayerName))
				DbgUI.InputText("Layer Name to inspect", inputLayerName);

			IEntity layerEntity = GetGame().GetWorld().FindEntityByName(inputLayerName);
			if (layerEntity)
			{
				SCR_ScenarioFrameworkLayerBase layerBase = SCR_ScenarioFrameworkLayerBase.Cast(layerEntity.FindComponent(SCR_ScenarioFrameworkLayerBase));
				if (layerBase)
				{
					PrepareConditionStrings(layerBase.m_aActivationConditions, layerEntity, "Condition");
					
					SCR_ScenarioFrameworkLayerTask layerTask = SCR_ScenarioFrameworkLayerTask.Cast(layerBase);
					if (layerTask)
						PrepareConditionStrings(layerTask.m_aFinishConditions, layerEntity, "Finish Condition");

					SCR_ScenarioFrameworkSlotTask slotTask = SCR_ScenarioFrameworkSlotTask.Cast(layerBase);
					if (slotTask)
						PrepareConditionStrings(slotTask.m_aFinishConditions, layerEntity, "Finish Condition");
				}
			}
		}
		DbgUI.End();
	}

	//------------------------------------------------------------------------------------------------
	//! Prepares condition strings for scenario activation conditions, displays them in list, allows user to select one and perform manipulations and display details
	//! \param[in] conditions
	//! \param[in] activationEntity ActivationEntity represents an entity in the game world that triggers scenario conditions.
	//! \param[in] categoryName CategoryName represents the name of the category for displaying condition strings in the debug UI hierarchy.
	static void PrepareConditionStrings(array<ref SCR_ScenarioFrameworkActivationConditionBase> conditions, IEntity activationEntity, string categoryName)
	{
		array<string> listElements = {};
		if (!conditions || conditions.IsEmpty())
			return;

		foreach (int index, SCR_ScenarioFrameworkActivationConditionBase condition : conditions)
		{
			string name = condition.ToString();
			name.Replace("SCR_ScenarioFrameworkActivationCondition", "");
			listElements.Insert(string.Format("%1 - %2", index, name));
		}

		if (listElements.IsEmpty())
			return;

		int selectedPluginIndex;
		DbgUI.Text(string.Format("%1 Hierarchy:", categoryName));
		DbgUI.List(string.Format("%1", categoryName), selectedPluginIndex, listElements);

		if (conditions.IsIndexValid(selectedPluginIndex))
			ProcessCondition(conditions[selectedPluginIndex], activationEntity);

		SCR_ScenarioFrameworkLayerBase layerBase = SCR_ScenarioFrameworkLayerBase.Cast(activationEntity.FindComponent(SCR_ScenarioFrameworkLayerBase));
		if (layerBase)
		{
			if (DbgUI.Button("Perform Conditon Check"))
				layerBase.InitActivationConditions();
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Debugs condition status and allows toggling debug mode for condition.
	//! \param[in] condition Target Condition.
	//! \param[in] layerEntity LayerEntity represents the entity in the scenario for which the condition is being processed.
	static void ProcessCondition(SCR_ScenarioFrameworkActivationConditionBase condition, IEntity layerEntity)
	{
		if (condition.m_bDebug)
		{
			DbgUI.Text("Debug Condition enabled");
			if (DbgUI.Button("Disable Condition debug"))
				condition.m_bDebug = false;
		}
		else
		{
			DbgUI.Text("Debug Condition disabled");
			if (DbgUI.Button("Enable Condition debug"))
				condition.m_bDebug = true;
		}

		bool conditionCheck = false;
		DbgUI.Check("Check condition status", conditionCheck);
		if (conditionCheck)
		{
			if (condition.Init(layerEntity))
				DbgUI.Text("Condition: True");
			else
				DbgUI.Text("Condition: False");
		}
	}
}
