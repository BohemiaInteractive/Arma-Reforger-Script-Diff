//#ifdef WORKBENCH
//[EntityEditorProps(category: "GameScripted/Channels", description: "Channel Manager", color: "0 0 255 255")]
//class SCR_TaskCommandChatChannel : SCR_ChatChannel
//{
//	[Attribute("task_create")]
//	protected string m_sCommandTaskCreate;
//	
//	[Attribute("task_delete")]
//	protected string m_sCommandTaskDelete;
//	
//	[Attribute("task_setState")]
//	protected string m_sCommandTaskSetState;
//	
//	[Attribute("task_setOwnership")]
//	protected string m_sCommandTaskSetOwnership;
//	
//	[Attribute("task_setVisibility")]
//	protected string m_sCommandTaskSetVisibility;
//	
//	[Attribute("task_setUIVisibility")]
//	protected string m_sCommandTaskSetUIVisibility;
//	
//	[Attribute("task_addFaction")]
//	protected string m_sCommandTaskAddFaction;
//	
//	[Attribute("task_removeFaction")]
//	protected string m_sCommandTaskRemoveFaction;
//	
//	[Attribute("task_addGroup")]
//	protected string m_sCommandTaskAddGroup;
//	
//	[Attribute("task_removeGroup")]
//	protected string m_sCommandTaskRemoveGroup;
//	
//	[Attribute("task_addExecutor")]
//	protected string m_sCommandTaskAddExecutor;
//	
//	[Attribute("task_removeExecutor")]
//	protected string m_sCommandTaskRemoveExecutor;
//	
//	[Attribute("task_assign")]
//	protected string m_sCommandTaskAssign;
//	
//	[Attribute("task_unassign")]
//	protected string m_sCommandTaskUnassign;
//	
//	[Attribute("task_move")]
//	protected string m_sCommandTaskMove;
//	
//	[Attribute("task_parent")]
//	protected string m_sCommandTaskParent;
//	
//	[Attribute("task_unparent")]
//	protected string m_sCommandTaskUnparent;
//	
//	[Attribute("task_showProgress")]
//	protected string m_sCommandTaskShowProgress;
//	
//	[Attribute("task_addProgress")]
//	protected string m_sCommandTaskAddProgress;
//	
//	[Attribute("task_removeProgress")]
//	protected string m_sCommandTaskRemoveProgress;
//	
//	[Attribute("task_list")]
//	protected string m_sCommandTaskDisplay;
//	
//	[Attribute("help")]
//	protected string m_sCommandHelp;
//	
//	[Attribute("{CF6A2A13DF7B1018}Prefabs/Tasks/ExtendedTask.et")]
//	protected ResourceName m_sTaskResource;
//	
//	protected const string COMMAND_KEYWORD_THIS = "this";
//	protected const string COMMAND_KEYWORD_NONE = "none";
//	
//	protected const string COMMAND_KEYWORD_TRUE = "true";
//	protected const string COMMAND_KEYWORD_FALSE = "false";
//	
//	protected const string COMMAND_KEYWORD_STATE_CREATED = "created";
//	protected const string COMMAND_KEYWORD_STATE_ASSIGNED = "assigned";
//	protected const string COMMAND_KEYWORD_STATE_PROGRESSED = "progressed";
//	protected const string COMMAND_KEYWORD_STATE_COMPLETED = "completed";
//	protected const string COMMAND_KEYWORD_STATE_FAILED = "failed";
//	
//	protected const string COMMAND_KEYWORD_OWNERSHIP_NONE = "none";
//	protected const string COMMAND_KEYWORD_OWNERSHIP_ENTITY = "entity";
//	protected const string COMMAND_KEYWORD_OWNERSHIP_GROUP = "group";
//	protected const string COMMAND_KEYWORD_OWNERSHIP_FACTION = "faction";
//	protected const string COMMAND_KEYWORD_OWNERSHIP_EVERYONE = "everyone";
//	
//	protected const string COMMAND_KEYWORD_VISIBILITY_NONE = "none";
//	protected const string COMMAND_KEYWORD_VISIBILITY_ENTITY = "entity";
//	protected const string COMMAND_KEYWORD_VISIBILITY_ASSIGNEES = "assignees";
//	protected const string COMMAND_KEYWORD_VISIBILITY_GROUP = "group";
//	protected const string COMMAND_KEYWORD_VISIBILITY_FACTION = "faction";
//	protected const string COMMAND_KEYWORD_VISIBILITY_EVERYONE = "everyone";
//	
//	protected const string COMMAND_KEYWORD_UIVISIBILITY_NONE = "none";
//	protected const string COMMAND_KEYWORD_UIVISIBILITY_MAP = "map";
//	protected const string COMMAND_KEYWORD_UIVISIBILITY_LIST = "list";
//	protected const string COMMAND_KEYWORD_UIVISIBILITY_ALL = "all";
//	
//	protected const string COMMAND_KEYWORD_TASKLIST_ALL = "all";
//	
//	//------------------------------------------------------------------------------------------------
//	override bool ProcessMessage(BaseChatComponent sender, string message, bool isAuthority)
//	{	
//		if (message.Contains(m_sCommandTaskCreate))
//		{
//			ProcessCommand_TaskCreate(message);
//			return false;
//		}
//		
//		if (message.Contains(m_sCommandTaskDelete))
//		{
//			ProcessCommand_TaskDelete(message);
//			return false;
//		}
//		
//		if (message.Contains(m_sCommandTaskSetState))
//		{
//			ProcessCommand_TaskSetState(message);
//			return false;
//		}
//		
//		if (message.Contains(m_sCommandTaskSetOwnership))
//		{
//			ProcessCommand_TaskSetOwnership(message);
//			return false;
//		}
//		
//		if (message.Contains(m_sCommandTaskSetVisibility))
//		{
//			ProcessCommand_TaskSetVisibility(message);
//			return false;
//		}
//		
//		if (message.Contains(m_sCommandTaskSetUIVisibility))
//		{
//			ProcessCommand_TaskSetUIVisibility(message);
//			return false;
//		}
//		
//		if (message.Contains(m_sCommandTaskAddFaction))
//		{
//			ProcessCommand_TaskAddFaction(message);
//			return false;
//		}
//		
//		if (message.Contains(m_sCommandTaskRemoveFaction))
//		{
//			ProcessCommand_TaskRemoveFaction(message);
//			return false;
//		}
//		
//		if (message.Contains(m_sCommandTaskAddGroup))
//		{
//			ProcessCommand_TaskAddGroup(message);
//			return false;
//		}
//		
//		if (message.Contains(m_sCommandTaskRemoveGroup))
//		{
//			ProcessCommand_TaskRemoveGroup(message);
//			return false;
//		}
//		
//		if (message.Contains(m_sCommandTaskAddExecutor))
//		{
//			ProcessCommand_TaskAddExecutor(message);
//			return false;
//		}
//		
//		if (message.Contains(m_sCommandTaskRemoveExecutor))
//		{
//			ProcessCommand_TaskRemoveExecutor(message);
//			return false;
//		}
//		
//		if (message.Contains(m_sCommandTaskAssign))
//		{
//			ProcessCommand_TaskAssign(message);
//			return false;
//		}
//		
//		if (message.Contains(m_sCommandTaskUnassign))
//		{
//			ProcessCommand_TaskUnassign(message);
//			return false;
//		}
//		
//		if (message.Contains(m_sCommandTaskMove))
//		{
//			ProcessCommand_TaskMove(message);
//			return false;
//		}
//		
//		if (message.Contains(m_sCommandTaskParent))
//		{
//			ProcessCommand_TaskParent(message);
//			return false;
//		}
//		
//		if (message.Contains(m_sCommandTaskUnparent))
//		{
//			ProcessCommand_TaskUnparent(message);
//			return false;
//		}
//		
//		if (message.Contains(m_sCommandTaskShowProgress))
//		{
//			ProcessCommand_TaskShowProgress(message);
//			return false;
//		}
//		
//		if (message.Contains(m_sCommandTaskAddProgress))
//		{
//			ProcessCommand_TaskAddProgress(message);
//			return false;
//		}
//		
//		if (message.Contains(m_sCommandTaskRemoveProgress))
//		{
//			ProcessCommand_TaskRemoveProgress(message);
//			return false;
//		}
//		
//		if (message.Contains(m_sCommandTaskDisplay))
//		{
//			ProcessCommand_ShowTasks(message);
//			return false;
//		}
//		
//		if (message.Contains(m_sCommandHelp))
//		{
//			ProcessCommand_Help(message);
//			return false;
//		}
//		
//		Print("Console > Unknown command: " + message + " Type 'help' for a list of commands", LogLevel.ERROR);
//		return false;
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	protected void ProcessCommand_TaskCreate(string command)
//	{
//		command.Replace(m_sCommandTaskCreate, "");
//		
//		array<string> params = {};
//		command.Split(" ", params, true);
//		
//		if (params.Count() < 6)
//		{
//			Print("Console > Invalid syntax for " + m_sCommandTaskCreate + ": <taskID> <task_Name> <task_Desc> <x> <y> <z>", LogLevel.ERROR);
//			return;
//		}
//		
//		foreach (string param : params)
//		{
//			if (param.IsEmpty())
//			{
//				Print("Console > Invalid syntax for " + m_sCommandTaskCreate + ": <taskID> <task_Name> <task_Desc> <x> <y> <z>", LogLevel.ERROR);
//				return;
//			}
//		}
//		
//		string taskID = params[0];
//		
//		string taskName = params[1];
//		taskName.Replace("_", " ");
//		string taskDesc = params[2];
//		taskDesc.Replace("_", " ");
//		
//		CameraBase currentCamera = GetGame().GetCameraManager().CurrentCamera();
//		
//		float taskPosX;
//		if (currentCamera && params[3] == COMMAND_KEYWORD_THIS)
//			taskPosX = currentCamera.GetOrigin()[0];
//		else
//			taskPosX = params[3].ToFloat();
//		
//		float taskPosY;
//		if (currentCamera && params[4] == COMMAND_KEYWORD_THIS)
//			taskPosY = currentCamera.GetOrigin()[1];
//		else
//			taskPosY = params[4].ToFloat();
//		
//		float taskPosZ;
//		if (currentCamera && params[5] == COMMAND_KEYWORD_THIS)
//			taskPosZ = currentCamera.GetOrigin()[2];
//		else
//			taskPosZ = params[5].ToFloat();
//		
//		vector taskPos = {taskPosX, taskPosY, taskPosZ};
//		
//		SCR_TaskSystem taskSystem = SCR_TaskSystem.GetInstance();
//		if (!taskSystem)
//			return;
//		
//		taskSystem.CreateTask(m_sTaskResource, taskID, taskName, taskDesc, taskPos);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	protected void ProcessCommand_TaskDelete(string command)
//	{
//		command.Replace(m_sCommandTaskDelete, "");
//		
//		array<string> params = {};
//		command.Split(" ", params, true);
//		
//		if (params.Count() < 1 || params[0].IsEmpty())
//		{
//			Print("Console > Invalid syntax for " + m_sCommandTaskDelete + ": <taskID>", LogLevel.ERROR);
//			return;
//		}
//		
//		string taskID = params[0];
//		
//		SCR_TaskSystem taskSystem = SCR_TaskSystem.GetInstance();
//		if (!taskSystem)
//			return;
//		
//		taskSystem.DeleteTask(taskSystem.GetTaskFromTaskID(taskID));
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	protected void ProcessCommand_TaskSetState(string command)
//	{
//		command.Replace(m_sCommandTaskSetState, "");
//		
//		array<string> params = {};
//		command.Split(" ", params, true);
//		
//		if (params.Count() < 2)
//		{
//			Print("Console > Invalid syntax for " + m_sCommandTaskSetState + ": <taskID> <state>", LogLevel.ERROR);
//			return;
//		}
//		
//		foreach (string param : params)
//		{
//			if (param.IsEmpty())
//			{
//				Print("Console > Invalid syntax for " + m_sCommandTaskSetState + ": <taskID> <state>", LogLevel.ERROR);
//				return;
//			}
//		}
//		
//		string taskID = params[0];
//		
//		SCR_ETaskState taskState;		
//		switch (params[1])
//		{
//			case COMMAND_KEYWORD_STATE_CREATED:
//				taskState = SCR_ETaskState.CREATED;
//				break;
//			
//			case COMMAND_KEYWORD_STATE_ASSIGNED:
//				taskState = SCR_ETaskState.ASSIGNED;
//				break;
//			
//			case COMMAND_KEYWORD_STATE_PROGRESSED:
//				taskState = SCR_ETaskState.PROGRESSED;
//				break;
//			
//			case COMMAND_KEYWORD_STATE_COMPLETED:
//				taskState = SCR_ETaskState.COMPLETED;
//				break;
//			
//			case COMMAND_KEYWORD_STATE_FAILED:
//				taskState = SCR_ETaskState.FAILED;
//				break;
//			
//			default:
//				Print("Console > Specified task state is invalid: <created, assigned, progressed, completed, failed>", LogLevel.ERROR);
//				return;
//		}
//		
//		SCR_TaskSystem taskSystem = SCR_TaskSystem.GetInstance();
//		if (!taskSystem)
//			return;
//		
//		taskSystem.SetTaskState(taskSystem.GetTaskFromTaskID(taskID), taskState);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	protected void ProcessCommand_TaskSetOwnership(string command)
//	{
//		command.Replace(m_sCommandTaskSetOwnership, "");
//		
//		array<string> params = {};
//		command.Split(" ", params, true);
//		
//		if (params.Count() < 2)
//		{
//			Print("Console > Invalid syntax for " + m_sCommandTaskSetOwnership + ": <taskID> <ownership>", LogLevel.ERROR);
//			return;
//		}
//		
//		foreach (string param : params)
//		{
//			if (param.IsEmpty())
//			{
//				Print("Console > Invalid syntax for " + m_sCommandTaskSetOwnership + ": <taskID> <ownership>", LogLevel.ERROR);
//				return;
//			}
//		}	
//		
//		string taskID = params[0];
//		
//		SCR_ETaskOwnership taskOwnership;
//		switch (params[1])
//		{
//			case COMMAND_KEYWORD_OWNERSHIP_NONE:
//				taskOwnership = SCR_ETaskOwnership.NONE;
//				break;
//			
//			case COMMAND_KEYWORD_OWNERSHIP_ENTITY:
//				taskOwnership = SCR_ETaskOwnership.EXECUTOR;
//				break;
//			
//			case COMMAND_KEYWORD_OWNERSHIP_GROUP:
//				taskOwnership = SCR_ETaskOwnership.GROUP;
//				break;
//			
//			case COMMAND_KEYWORD_OWNERSHIP_FACTION:
//				taskOwnership = SCR_ETaskOwnership.FACTION;
//				break;
//			
//			case COMMAND_KEYWORD_OWNERSHIP_EVERYONE:
//				taskOwnership = SCR_ETaskOwnership.EVERYONE;
//				break;
//			
//			default:
//				Print("Console > Specified task ownership is invalid: <none, group, faction, everyone>", LogLevel.ERROR);
//				return;
//		}
//		
//		SCR_TaskSystem taskSystem = SCR_TaskSystem.GetInstance();
//		if (!taskSystem)
//			return;
//		
//		taskSystem.SetTaskOwnership(taskSystem.GetTaskFromTaskID(taskID), taskOwnership);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	protected void ProcessCommand_TaskSetVisibility(string command)
//	{
//		command.Replace(m_sCommandTaskSetVisibility, "");
//		
//		array<string> params = {};
//		command.Split(" ", params, true);
//		
//		if (params.Count() < 2)
//		{
//			Print("Console > Invalid syntax for " + m_sCommandTaskSetVisibility + ": <taskID> <visibility>", LogLevel.ERROR);
//			return;
//		}
//		
//		foreach (string param : params)
//		{
//			if (param.IsEmpty())
//			{
//				Print("Console > Invalid syntax for " + m_sCommandTaskSetVisibility + ": <taskID> <visibility>", LogLevel.ERROR);
//				return;
//			}
//		}	
//		
//		string taskID = params[0];
//		
//		SCR_ETaskVisibility taskVisibility;
//		switch (params[1])
//		{
//			case COMMAND_KEYWORD_VISIBILITY_NONE:
//				taskVisibility = SCR_ETaskVisibility.NONE;
//				break;
//			
//			case COMMAND_KEYWORD_VISIBILITY_ENTITY:
//				taskVisibility = SCR_ETaskVisibility.EXECUTOR;
//				break;
//			
//			case COMMAND_KEYWORD_VISIBILITY_ASSIGNEES:
//				taskVisibility = SCR_ETaskVisibility.ASSIGNEES;
//				break;
//			
//			case COMMAND_KEYWORD_VISIBILITY_GROUP:
//				taskVisibility = SCR_ETaskVisibility.GROUP;
//				break;
//			
//			case COMMAND_KEYWORD_VISIBILITY_FACTION:
//				taskVisibility = SCR_ETaskVisibility.FACTION;
//				break;
//			
//			case COMMAND_KEYWORD_VISIBILITY_EVERYONE:
//				taskVisibility = SCR_ETaskVisibility.EVERYONE;
//				break;
//			
//			default:
//				Print("Console > Specified task visibility is invalid: <none, assignee, group, faction, everyone>", LogLevel.ERROR);
//				return;
//		}
//		
//		SCR_TaskSystem taskSystem = SCR_TaskSystem.GetInstance();
//		if (!taskSystem)
//			return;
//		
//		taskSystem.SetTaskVisibility(taskSystem.GetTaskFromTaskID(taskID), taskVisibility);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	protected void ProcessCommand_TaskSetUIVisibility(string command)
//	{
//		command.Replace(m_sCommandTaskSetUIVisibility, "");
//		
//		array<string> params = {};
//		command.Split(" ", params, true);
//		
//		if (params.Count() < 2)
//		{
//			Print("Console > Invalid syntax for " + m_sCommandTaskSetUIVisibility + ": <taskID> <visibility>", LogLevel.ERROR);
//			return;
//		}
//		
//		foreach (string param : params)
//		{
//			if (param.IsEmpty())
//			{
//				Print("Console > Invalid syntax for " + m_sCommandTaskSetUIVisibility + ": <taskID> <visibility>", LogLevel.ERROR);
//				return;
//			}
//		}	
//		
//		string taskID = params[0];
//		
//		SCR_ETaskUIVisibility taskVisibility;
//		switch (params[1])
//		{
//			case COMMAND_KEYWORD_UIVISIBILITY_NONE:
//				taskVisibility = SCR_ETaskUIVisibility.NONE;
//				break;
//			
//			case COMMAND_KEYWORD_UIVISIBILITY_MAP:
//				taskVisibility = SCR_ETaskUIVisibility.MAP_ONLY;
//				break;
//			
//			case COMMAND_KEYWORD_UIVISIBILITY_LIST:
//				taskVisibility = SCR_ETaskUIVisibility.LIST_ONLY;
//				break;
//			
//			case COMMAND_KEYWORD_UIVISIBILITY_ALL:
//				taskVisibility = SCR_ETaskUIVisibility.ALL;
//				break;
//			
//			default:
//				Print("Console > Specified task UI visibility is invalid: <none, map, list, all>", LogLevel.ERROR);
//				return;
//		}
//		
//		SCR_TaskSystem taskSystem = SCR_TaskSystem.GetInstance();
//		if (!taskSystem)
//			return;
//		
//		taskSystem.SetTaskUIVisibility(taskSystem.GetTaskFromTaskID(taskID), taskVisibility);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	protected void ProcessCommand_TaskAddFaction(string command)
//	{
//		command.Replace(m_sCommandTaskAddFaction, "");
//		
//		array<string> params = {};
//		command.Split(" ", params, true);
//		
//		if (params.Count() < 2)
//		{
//			Print("Console > Invalid syntax for " + m_sCommandTaskAddFaction + ": <taskID> <factionKey>", LogLevel.ERROR);
//			return;
//		}
//		
//		foreach (string param : params)
//		{
//			if (param.IsEmpty())
//			{
//				Print("Console > Invalid syntax for " + m_sCommandTaskAddFaction + ": <taskID> <factionKey>", LogLevel.ERROR);
//				return;
//			}
//		}	
//		
//		string taskID = params[0];
//		string taskFactionKey = params[1];
//		switch (taskFactionKey)
//		{
//			case COMMAND_KEYWORD_THIS:
//				FactionAffiliationComponent factionAffiliation = FactionAffiliationComponent.Cast(GetGame().GetPlayerController().GetControlledEntity().FindComponent(FactionAffiliationComponent));
//				if (!factionAffiliation)
//				{
//					taskFactionKey = string.Empty;
//					break;
//				}
//			
//				taskFactionKey = factionAffiliation.GetAffiliatedFactionKey();
//				break;
//		}
//		
//		SCR_TaskSystem taskSystem = SCR_TaskSystem.GetInstance();
//		if (!taskSystem)
//			return;
//		
//		taskSystem.AddTaskFaction(taskSystem.GetTaskFromTaskID(taskID), taskFactionKey);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	protected void ProcessCommand_TaskRemoveFaction(string command)
//	{
//		command.Replace(m_sCommandTaskRemoveFaction, "");
//		
//		array<string> params = {};
//		command.Split(" ", params, true);
//		
//		if (params.Count() < 2)
//		{
//			Print("Console > Invalid syntax for " + m_sCommandTaskRemoveFaction + ": <taskID> <factionKey>", LogLevel.ERROR);
//			return;
//		}
//		
//		foreach (string param : params)
//		{
//			if (param.IsEmpty())
//			{
//				Print("Console > Invalid syntax for " + m_sCommandTaskRemoveFaction + ": <taskID> <factionKey>", LogLevel.ERROR);
//				return;
//			}
//		}	
//		
//		string taskID = params[0];
//		string taskFactionKey = params[1];
//		switch (taskFactionKey)
//		{
//			case COMMAND_KEYWORD_THIS:
//				FactionAffiliationComponent factionAffiliation = FactionAffiliationComponent.Cast(GetGame().GetPlayerController().GetControlledEntity().FindComponent(FactionAffiliationComponent));
//				if (!factionAffiliation)
//				{
//					taskFactionKey = string.Empty;
//					break;
//				}
//			
//				taskFactionKey = factionAffiliation.GetAffiliatedFactionKey();
//				break;
//		}
//		
//		SCR_TaskSystem taskSystem = SCR_TaskSystem.GetInstance();
//		if (!taskSystem)
//			return;
//		
//		taskSystem.RemoveTaskFaction(taskSystem.GetTaskFromTaskID(taskID), taskFactionKey);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	protected void ProcessCommand_TaskAddGroup(string command)
//	{
//		command.Replace(m_sCommandTaskAddGroup, "");
//		
//		array<string> params = {};
//		command.Split(" ", params, true);
//		
//		if (params.Count() < 2)
//		{
//			Print("Console > Invalid syntax for " + m_sCommandTaskAddGroup + ": <taskID> <groupID>", LogLevel.ERROR);
//			return;
//		}
//		
//		foreach (string param : params)
//		{
//			if (param.IsEmpty())
//			{
//				Print("Console > Invalid syntax for " + m_sCommandTaskAddGroup + ": <taskID> <groupID>", LogLevel.ERROR);
//				return;
//			}
//		}	
//		
//		string taskID = params[0];
//		string taskGroupID = params[1];
//		switch (taskGroupID)
//		{
//			case COMMAND_KEYWORD_THIS:
//				SCR_PlayerControllerGroupComponent groupComponent = SCR_PlayerControllerGroupComponent.Cast(GetGame().GetPlayerController().FindComponent(SCR_PlayerControllerGroupComponent));
//				if (!groupComponent)
//				{
//					taskGroupID = "-1";
//					break;
//				}
//			
//				taskGroupID = groupComponent.GetGroupID().ToString();
//				break;
//		}
//		
//		SCR_TaskSystem taskSystem = SCR_TaskSystem.GetInstance();
//		if (!taskSystem)
//			return;
//		
//		taskSystem.AddTaskGroup(taskSystem.GetTaskFromTaskID(taskID), taskGroupID.ToInt());
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	protected void ProcessCommand_TaskRemoveGroup(string command)
//	{
//		command.Replace(m_sCommandTaskRemoveGroup, "");
//		
//		array<string> params = {};
//		command.Split(" ", params, true);
//		
//		if (params.Count() < 2)
//		{
//			Print("Console > Invalid syntax for " + m_sCommandTaskRemoveGroup + ": <taskID> <groupID>", LogLevel.ERROR);
//			return;
//		}
//		
//		foreach (string param : params)
//		{
//			if (param.IsEmpty())
//			{
//				Print("Console > Invalid syntax for " + m_sCommandTaskRemoveGroup + ": <taskID> <groupID>", LogLevel.ERROR);
//				return;
//			}
//		}	
//		
//		string taskID = params[0];
//		string taskGroupID = params[1];
//		switch (taskGroupID)
//		{
//			case COMMAND_KEYWORD_THIS:
//				SCR_PlayerControllerGroupComponent groupComponent = SCR_PlayerControllerGroupComponent.Cast(GetGame().GetPlayerController().FindComponent(SCR_PlayerControllerGroupComponent));
//				if (!groupComponent)
//				{
//					taskGroupID = "-1";
//					break;
//				}
//			
//				taskGroupID = groupComponent.GetGroupID().ToString();
//				break;
//		}
//		
//		SCR_TaskSystem taskSystem = SCR_TaskSystem.GetInstance();
//		if (!taskSystem)
//			return;
//		
//		taskSystem.RemoveTaskGroup(taskSystem.GetTaskFromTaskID(taskID), taskGroupID.ToInt());
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	protected void ProcessCommand_TaskAddExecutor(string command)
//	{
//		command.Replace(m_sCommandTaskAddExecutor, "");
//		
//		array<string> params = {};
//		command.Split(" ", params, true);
//		
//		if (params.Count() < 2)
//		{
//			Print("Console > Invalid syntax for " + m_sCommandTaskAddExecutor + ": <taskID> <playerID/entityID/groupID>", LogLevel.ERROR);
//			return;
//		}
//		
//		foreach (string param : params)
//		{
//			if (param.IsEmpty())
//			{
//				Print("Console > Invalid syntax for " + m_sCommandTaskAddExecutor + ": <taskID> <playerID/entityID/groupID>", LogLevel.ERROR);
//				return;
//			}
//		}	
//		
//		string taskID = params[0];
//		
//		SCR_TaskExecutor executor;
//		if (params[1] == COMMAND_KEYWORD_THIS)
//		{
//			int playerID = GetGame().GetPlayerController().GetPlayerId();
//			executor = SCR_TaskExecutor.FromPlayerID(playerID);
//		}
//		else if (params[1].ToInt() > 0)
//		{
//			int playerID = params[1].ToInt();
//			executor = SCR_TaskExecutor.FromPlayerID(playerID);
//		}
//		else if (params[1].Contains("g:") && params[1].Substring(2, params[1].Length() - 2).ToInt() > -1)
//		{
//			int groupID = params[1].Substring(2, params[1].Length() - 2).ToInt();
//			executor = SCR_TaskExecutor.FromGroup(groupID);
//		}
//		else
//		{
//			IEntity ent = GetGame().FindEntity(params[1]);
//			executor = SCR_TaskExecutor.FromEntity(ent);
//		}
//		
//		if (!executor)
//		{
//			Print("Console > Could not find entity with name: " + params[1], LogLevel.ERROR);
//			return;
//		}
//		
//		SCR_TaskSystem taskSystem = SCR_TaskSystem.GetInstance();
//		if (!taskSystem)
//			return;
//		
//		taskSystem.AddTaskExecutor(taskSystem.GetTaskFromTaskID(taskID), executor);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	protected void ProcessCommand_TaskRemoveExecutor(string command)
//	{
//		command.Replace(m_sCommandTaskRemoveExecutor, "");
//		
//		array<string> params = {};
//		command.Split(" ", params, true);
//		
//		if (params.Count() < 2)
//		{
//			Print("Console > Invalid syntax for " + m_sCommandTaskRemoveExecutor + ": <taskID> <playerID/entityID/groupID>", LogLevel.ERROR);
//			return;
//		}
//		
//		foreach (string param : params)
//		{
//			if (param.IsEmpty())
//			{
//				Print("Console > Invalid syntax for " + m_sCommandTaskRemoveExecutor + ": <taskID> <playerID/entityID/groupID>", LogLevel.ERROR);
//				return;
//			}
//		}	
//		
//		string taskID = params[0];
//		
//		SCR_TaskExecutor executor;
//		if (params[1] == COMMAND_KEYWORD_THIS)
//		{
//			int playerID = GetGame().GetPlayerController().GetPlayerId();
//			executor = SCR_TaskExecutor.FromPlayerID(playerID);
//		}
//		else if (params[1].ToInt() > 0)
//		{
//			int playerID = params[1].ToInt();
//			executor = SCR_TaskExecutor.FromPlayerID(playerID);
//		}
//		else if (params[1].Contains("g:") && params[1].Substring(2, params[1].Length() - 2).ToInt() > -1)
//		{
//			int groupID = params[1].Substring(2, params[1].Length() - 2).ToInt();
//			executor = SCR_TaskExecutor.FromGroup(groupID);
//		}
//		else
//		{
//			IEntity ent = GetGame().FindEntity(params[1]);
//			executor = SCR_TaskExecutor.FromEntity(ent);
//		}
//		
//		if (!executor)
//		{
//			Print("Console > Could not find entity with name: " + params[1], LogLevel.ERROR);
//			return;
//		}
//		
//		SCR_TaskSystem taskSystem = SCR_TaskSystem.GetInstance();
//		if (!taskSystem)
//			return;
//		
//		taskSystem.RemoveTaskExecutor(taskSystem.GetTaskFromTaskID(taskID), executor);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	protected void ProcessCommand_TaskAssign(string command)
//	{
//		command.Replace(m_sCommandTaskAssign, "");
//		
//		array<string> params = {};
//		command.Split(" ", params, true);
//		
//		if (params.Count() < 2)
//		{
//			Print("Console > Invalid syntax for " + m_sCommandTaskAssign + ": <taskID> <playerID/entityID/groupID> <force>", LogLevel.ERROR);
//			return;
//		}
//		
//		foreach (string param : params)
//		{
//			if (param.IsEmpty())
//			{
//				Print("Console > Invalid syntax for " + m_sCommandTaskAssign + ": <taskID> <playerID/entityID/groupID> <force>", LogLevel.ERROR);
//				return;
//			}
//		}	
//		
//		string taskID = params[0];
//		
//		SCR_TaskExecutor assignee;
//		if (params[1] == COMMAND_KEYWORD_THIS)
//		{
//			int playerID = GetGame().GetPlayerController().GetPlayerId();
//			assignee = SCR_TaskExecutor.FromPlayerID(playerID);
//		}
//		else if (params[1].ToInt() > 0)
//		{
//			int playerID = params[1].ToInt();
//			assignee = SCR_TaskExecutor.FromPlayerID(playerID);
//		}
//		else if (params[1].Contains("g:") && params[1].Substring(2, params[1].Length() - 2).ToInt() > -1)
//		{
//			int groupID = params[1].Substring(2, params[1].Length() - 2).ToInt();
//			assignee = SCR_TaskExecutor.FromGroup(groupID);
//		}
//		else
//		{
//			IEntity ent = GetGame().FindEntity(params[1]);
//			assignee = SCR_TaskExecutor.FromEntity(ent);
//		}
//		
//		if (!assignee)
//		{
//			Print("Console > Could not find entity with name: " + params[1], LogLevel.ERROR);
//			return;
//		}
//		
//		bool force;
//		if (params.Count() >= 3)
//		{
//			if (params[2] == COMMAND_KEYWORD_TRUE)
//			{
//				force = true;
//			}
//			else if (params[2] == COMMAND_KEYWORD_FALSE)
//			{
//				force = false;
//			}
//			else
//			{
//				Print("Console > Specified keyword is invalid: <true, false>", LogLevel.ERROR);
//				return;
//			}
//		}
//		
//		SCR_TaskSystem taskSystem = SCR_TaskSystem.GetInstance();
//		if (!taskSystem)
//			return;
//		
//		taskSystem.AssignTask(taskSystem.GetTaskFromTaskID(taskID), assignee, force);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	protected void ProcessCommand_TaskUnassign(string command)
//	{
//		command.Replace(m_sCommandTaskUnassign, "");
//		
//		array<string> params = {};
//		command.Split(" ", params, true);
//		
//		if (params.Count() < 2)
//		{
//			Print("Console > Invalid syntax for " + m_sCommandTaskUnassign + ": <taskID> <playerID/entityID/groupID>", LogLevel.ERROR);
//			return;
//		}
//		
//		foreach (string param : params)
//		{
//			if (param.IsEmpty())
//			{
//				Print("Console > Invalid syntax for " + m_sCommandTaskUnassign + ": <taskID> <playerID/entityID/groupID>", LogLevel.ERROR);
//				return;
//			}
//		}	
//		
//		string taskID = params[0];
//		
//		SCR_TaskExecutor assignee;
//		if (params[1] == COMMAND_KEYWORD_THIS)
//		{
//			int playerID = GetGame().GetPlayerController().GetPlayerId();
//			assignee = SCR_TaskExecutor.FromPlayerID(playerID);
//		}
//		else if (params[1].ToInt() > 0)
//		{
//			int playerID = params[1].ToInt();
//			assignee = SCR_TaskExecutor.FromPlayerID(playerID);
//		}
//		else if (params[1].Contains("g:") && params[1].Substring(2, params[1].Length() - 2).ToInt() > -1)
//		{
//			int groupID = params[1].Substring(2, params[1].Length() - 2).ToInt();
//			assignee = SCR_TaskExecutor.FromGroup(groupID);
//		}
//		else
//		{
//			IEntity ent = GetGame().FindEntity(params[1]);
//			assignee = SCR_TaskExecutor.FromEntity(ent);
//		}
//		
//		if (!assignee)
//		{
//			Print("Console > Could not find entity with name: " + params[1], LogLevel.ERROR);
//			return;
//		}
//		
//		SCR_TaskSystem taskSystem = SCR_TaskSystem.GetInstance();
//		if (!taskSystem)
//			return;
//		
//		taskSystem.UnassignTask(taskSystem.GetTaskFromTaskID(taskID), assignee);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	protected void ProcessCommand_TaskMove(string command)
//	{
//		command.Replace(m_sCommandTaskMove, "");
//		
//		array<string> params = {};
//		command.Split(" ", params, true);
//		
//		if (params.Count() < 4)
//		{
//			Print("Console > Invalid syntax for " + m_sCommandTaskMove + ": <taskID> <x> <y> <z>", LogLevel.ERROR);
//			return;
//		}
//		
//		foreach (string param : params)
//		{
//			if (param.IsEmpty())
//			{
//				Print("Console > Invalid syntax for " + m_sCommandTaskMove + ": <taskID> <x> <y> <z>", LogLevel.ERROR);
//				return;
//			}
//		}	
//		
//		string taskID = params[0];
//		
//		CameraBase currentCamera = GetGame().GetCameraManager().CurrentCamera();
//		
//		float taskPosX;
//		if (currentCamera && params[1] == COMMAND_KEYWORD_THIS)
//			taskPosX = currentCamera.GetOrigin()[0];
//		else
//			taskPosX = params[1].ToFloat();
//		
//		float taskPosY;
//		if (currentCamera && params[2] == COMMAND_KEYWORD_THIS)
//			taskPosY = currentCamera.GetOrigin()[1];
//		else
//			taskPosY = params[2].ToFloat();
//		
//		float taskPosZ;
//		if (currentCamera && params[3] == COMMAND_KEYWORD_THIS)
//			taskPosZ = currentCamera.GetOrigin()[2];
//		else
//			taskPosZ = params[3].ToFloat();
//		
//		vector taskPos = {taskPosX, taskPosY, taskPosZ};
//		
//		SCR_TaskSystem taskSystem = SCR_TaskSystem.GetInstance();
//		if (!taskSystem)
//			return;
//		
//		taskSystem.MoveTask(taskSystem.GetTaskFromTaskID(taskID), taskPos);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	protected void ProcessCommand_TaskParent(string command)
//	{
//		command.Replace(m_sCommandTaskParent, "");
//		
//		array<string> params = {};
//		command.Split(" ", params, true);
//		
//		if (params.Count() < 2)
//		{
//			Print("Console > Invalid syntax for " + m_sCommandTaskParent + ": <childTaskID> <taskID>", LogLevel.ERROR);
//			return;
//		}
//		
//		foreach (string param : params)
//		{
//			if (param.IsEmpty())
//			{
//				Print("Console > Invalid syntax for " + m_sCommandTaskParent + ": <childTaskID> <taskID>", LogLevel.ERROR);
//				return;
//			}
//		}	
//		
//		string childTaskID = params[0];
//		string taskID = params[1];
//		
//		SCR_TaskSystem taskSystem = SCR_TaskSystem.GetInstance();
//		if (!taskSystem)
//			return;
//		
//		taskSystem.AddChildTaskTo(taskSystem.GetTaskFromTaskID(taskID), taskSystem.GetTaskFromTaskID(childTaskID));
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	protected void ProcessCommand_TaskUnparent(string command)
//	{
//		command.Replace(m_sCommandTaskUnparent, "");
//		
//		array<string> params = {};
//		command.Split(" ", params, true);
//		
//		if (params.Count() < 2)
//		{
//			Print("Console > Invalid syntax for " + m_sCommandTaskUnparent + ": <childTaskID> <taskID>", LogLevel.ERROR);
//			return;
//		}
//		
//		foreach (string param : params)
//		{
//			if (param.IsEmpty())
//			{
//				Print("Console > Invalid syntax for " + m_sCommandTaskUnparent + ": <childTaskID> <taskID>", LogLevel.ERROR);
//				return;
//			}
//		}	
//		
//		string childTaskID = params[0];
//		string taskID = params[1];
//		
//		SCR_TaskSystem taskSystem = SCR_TaskSystem.GetInstance();
//		if (!taskSystem)
//			return;
//		
//		taskSystem.RemoveChildTaskFrom(taskSystem.GetTaskFromTaskID(taskID), taskSystem.GetTaskFromTaskID(childTaskID));
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	protected void ProcessCommand_TaskShowProgress(string command)
//	{
//		command.Replace(m_sCommandTaskShowProgress, "");
//		
//		array<string> params = {};
//		command.Split(" ", params, true);
//		
//		if (params.Count() < 2)
//		{
//			Print("Console > Invalid syntax for " + m_sCommandTaskShowProgress + ": <taskID> <enable>", LogLevel.ERROR);
//			return;
//		}
//		
//		foreach (string param : params)
//		{
//			if (param.IsEmpty())
//			{
//				Print("Console > Invalid syntax for " + m_sCommandTaskShowProgress + ": <taskID> <enable>", LogLevel.ERROR);
//				return;
//			}
//		}	
//		
//		string taskID = params[0];
//		
//		bool enable;
//		if (params[1] == COMMAND_KEYWORD_TRUE)
//		{
//			enable = true;
//		}
//		else if (params[1] == COMMAND_KEYWORD_FALSE)
//		{
//			enable = false;
//		}
//		else
//		{
//			Print("Console > Specified keyword is invalid: <true, false>", LogLevel.ERROR);
//			return;
//		}
//		
//		SCR_TaskSystem taskSystem = SCR_TaskSystem.GetInstance();
//		if (!taskSystem)
//			return;
//		
//		taskSystem.ShowProgressForTask(taskSystem.GetTaskFromTaskID(taskID), enable);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	protected void ProcessCommand_TaskAddProgress(string command)
//	{
//		command.Replace(m_sCommandTaskAddProgress, "");
//		
//		array<string> params = {};
//		command.Split(" ", params, true);
//		
//		if (params.Count() < 2)
//		{
//			Print("Console > Invalid syntax for " + m_sCommandTaskAddProgress + ": <taskID> <progress>", LogLevel.ERROR);
//			return;
//		}
//		
//		foreach (string param : params)
//		{
//			if (param.IsEmpty())
//			{
//				Print("Console > Invalid syntax for " + m_sCommandTaskAddProgress + ": <taskID> <progress>", LogLevel.ERROR);
//				return;
//			}
//		}	
//		
//		string taskID = params[0];
//		string progress = params[1];
//		
//		SCR_TaskSystem taskSystem = SCR_TaskSystem.GetInstance();
//		if (!taskSystem)
//			return;
//		
//		taskSystem.AddTaskProgress(taskSystem.GetTaskFromTaskID(taskID), progress.ToFloat());
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	protected void ProcessCommand_TaskRemoveProgress(string command)
//	{
//		command.Replace(m_sCommandTaskRemoveProgress, "");
//		
//		array<string> params = {};
//		command.Split(" ", params, true);
//		
//		if (params.Count() < 2)
//		{
//			Print("Console > Invalid syntax for " + m_sCommandTaskRemoveProgress + ": <taskID> <progress>", LogLevel.ERROR);
//			return;
//		}
//		
//		foreach (string param : params)
//		{
//			if (param.IsEmpty())
//			{
//				Print("Console > Invalid syntax for " + m_sCommandTaskRemoveProgress + ": <taskID> <progress>", LogLevel.ERROR);
//				return;
//			}
//		}	
//		
//		string taskID = params[0];
//		string progress = params[1];
//		
//		SCR_TaskSystem taskSystem = SCR_TaskSystem.GetInstance();
//		if (!taskSystem)
//			return;
//		
//		taskSystem.RemoveTaskProgress(taskSystem.GetTaskFromTaskID(taskID), progress.ToFloat());
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	protected void ProcessCommand_ShowTasks(string command)
//	{
//		command.Replace(m_sCommandTaskDisplay, "");
//		
//		array<string> params = {};
//		command.Split(" ", params, true);
//		
//		if (params.Count() < 1)
//		{
//			Print("Console > Invalid syntax for " + m_sCommandTaskDisplay + ": <playerID/entityID>", LogLevel.ERROR);
//			return;
//		}
//
//		if (params[0].IsEmpty())
//		{
//			Print("Console > Invalid syntax for " + m_sCommandTaskDisplay + ": <playerID/entityID>", LogLevel.ERROR);
//			return;
//		}
//		
//		SCR_TaskSystem taskSystem = SCR_TaskSystem.GetInstance();
//		if (!taskSystem)
//			return;
//		
//		if (params[0] == COMMAND_KEYWORD_TASKLIST_ALL)
//		{
//			taskSystem.PrintTasks();
//			return;
//		}
//		
//		SCR_TaskExecutor executor;
//		if (params[0] == COMMAND_KEYWORD_THIS)
//		{
//			int playerID = GetGame().GetPlayerController().GetPlayerId();
//			executor = SCR_TaskExecutor.FromPlayerID(playerID);
//		}
//		else if (params[0].ToInt() > 0)
//		{
//			int playerID = params[0].ToInt();
//			executor = SCR_TaskExecutor.FromPlayerID(playerID);
//		}
//		else
//		{
//			IEntity ent = GetGame().FindEntity(params[0]);
//			executor = SCR_TaskExecutor.FromEntity(ent);
//		}
//		
//		if (!executor)
//		{
//			Print("Console > Could not find entity with name: " + params[0], LogLevel.ERROR);
//			return;
//		}
//		
//		taskSystem.PrintTasks(executor);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	protected void ProcessCommand_Help(string command)
//	{
//		Print("\n", LogLevel.NORMAL);
//		
//		Print("Console > Syntax for " + m_sCommandTaskCreate + ": <taskID> <task_Name> <task_Desc> <x> <y> <z>", LogLevel.NORMAL);
//		Print("Example: task_create move0 Move move_to_location this 0 this", LogLevel.DEBUG);
//		Print("\n", LogLevel.NORMAL);
//		
//		Print("Console > Syntax for " + m_sCommandTaskDelete + ": <taskID>", LogLevel.NORMAL);
//		Print("Example: task_delete move0", LogLevel.DEBUG);
//		Print("\n", LogLevel.NORMAL);
//		
//		Print("Console > Syntax for " + m_sCommandTaskSetState + ": <taskID> <state>", LogLevel.NORMAL);
//		Print("Example: task_setState move0 completed", LogLevel.DEBUG);
//		Print("\n", LogLevel.NORMAL);
//		
//		Print("Console > Syntax for " + m_sCommandTaskSetOwnership + ": <taskID> <ownership>", LogLevel.NORMAL);
//		Print("Example: task_setOwnership move0 group", LogLevel.DEBUG);
//		Print("\n", LogLevel.NORMAL);
//		
//		Print("Console > Syntax for " + m_sCommandTaskSetVisibility + ": <taskID> <visibility>", LogLevel.NORMAL);
//		Print("Example: task_setVisibility move0 group", LogLevel.DEBUG);
//		Print("\n", LogLevel.NORMAL);
//		
//		Print("Console > Syntax for " + m_sCommandTaskSetUIVisibility + ": <taskID> <visibility>", LogLevel.NORMAL);
//		Print("Example: task_setUIVisibility move0 map", LogLevel.DEBUG);
//		Print("\n", LogLevel.NORMAL);
//		
//		Print("Console > Syntax for " + m_sCommandTaskAddFaction + ": <taskID> <factionKey>", LogLevel.NORMAL);
//		Print("Example: task_addFaction move0 USSR", LogLevel.DEBUG);
//		Print("\n", LogLevel.NORMAL);
//		
//		Print("Console > Syntax for " + m_sCommandTaskRemoveFaction + ": <taskID> <factionKey>", LogLevel.NORMAL);
//		Print("Example: task_removeFaction move0 USSR", LogLevel.DEBUG);
//		Print("\n", LogLevel.NORMAL);
//		
//		Print("Console > Syntax for " + m_sCommandTaskAddGroup + ": <taskID> <groupID>", LogLevel.NORMAL);
//		Print("Example: task_addGroup move0 this", LogLevel.DEBUG);
//		Print("\n", LogLevel.NORMAL);
//		
//		Print("Console > Syntax for " + m_sCommandTaskRemoveGroup + ": <taskID> <groupID>", LogLevel.NORMAL);
//		Print("Example: task_removeGroup move0 this", LogLevel.DEBUG);
//		Print("\n", LogLevel.NORMAL);
//		
//		Print("Console > Syntax for " + m_sCommandTaskAddExecutor + ": <taskID> <playerID/entityID/groupID>", LogLevel.NORMAL);
//		Print("Example: task_addExecutor move0 this", LogLevel.DEBUG);
//		Print("\n", LogLevel.NORMAL);
//		
//		Print("Console > Syntax for " + m_sCommandTaskRemoveExecutor + ": <taskID> <playerID/entityID/groupID>", LogLevel.NORMAL);
//		Print("Example: task_removeExecutor move0 this", LogLevel.DEBUG);
//		Print("\n", LogLevel.NORMAL);
//		
//		Print("Console > Syntax for " + m_sCommandTaskAssign + ": <taskID> <playerID/entityID/groupID> <force>", LogLevel.NORMAL);
//		Print("Example: task_assign move0 this true", LogLevel.DEBUG);
//		Print("\n", LogLevel.NORMAL);
//		
//		Print("Console > Syntax for " + m_sCommandTaskUnassign + ": <taskID> <playerID/entityID/groupID>", LogLevel.NORMAL);
//		Print("Example: task_unassign move0 this", LogLevel.DEBUG);
//		Print("\n", LogLevel.NORMAL);
//		
//		Print("Console > Syntax for " + m_sCommandTaskMove + ": <taskID> <x> <y> <z>", LogLevel.NORMAL);
//		Print("Example: task_move move0 100 this 20.5", LogLevel.DEBUG);
//		Print("\n", LogLevel.NORMAL);
//		
//		Print("Console > Syntax for " + m_sCommandTaskParent + ": <childTaskID> <taskID>", LogLevel.NORMAL);
//		Print("Example: task_parent move1 move2", LogLevel.DEBUG);
//		Print("\n", LogLevel.NORMAL);
//		
//		Print("Console > Syntax for " + m_sCommandTaskUnparent + ": <childTaskID> <taskID>", LogLevel.NORMAL);
//		Print("Example: task_unparent move1 move2", LogLevel.DEBUG);
//		Print("\n", LogLevel.NORMAL);
//		
//		Print("Console > Syntax for " + m_sCommandTaskShowProgress + ": <taskID> <enable>", LogLevel.NORMAL);
//		Print("Example: task_showProgress move0 true", LogLevel.DEBUG);
//		Print("\n", LogLevel.NORMAL);
//		
//		Print("Console > Syntax for " + m_sCommandTaskAddProgress + ": <taskID> <progress>", LogLevel.NORMAL);
//		Print("Example: task_addProgress move0 50", LogLevel.DEBUG);
//		Print("\n", LogLevel.NORMAL);
//		
//		Print("Console > Syntax for " + m_sCommandTaskRemoveProgress + ": <taskID> <progress>", LogLevel.NORMAL);
//		Print("Example: task_removeProgress move0 50", LogLevel.DEBUG);
//		Print("\n", LogLevel.NORMAL);
//		
//		Print("Console > Syntax for " + m_sCommandTaskDisplay + ": <playerID/entityID>", LogLevel.NORMAL);
//		Print("Example: task_list all", LogLevel.DEBUG);
//		Print("\n", LogLevel.NORMAL);
//		
//		Print("Console > Keyword 'this': Points to relevant data related to the player executing the command", LogLevel.NORMAL);
//		Print("Console > Keyword 'none': Sets data to null; Can only be used with <factionKey/groupID> parameter", LogLevel.NORMAL);
//		
//		Print("Console > Keyword 'created/assigned/progressed/completed/failed': Sets task state to CREATED/ASSIGNED/PROGRESSED/COMPLETED/FAILED; Can only be used with <state> parameter", LogLevel.NORMAL);
//		Print("Console > Keyword 'none/entity/group/faction/everyone': Sets task ownership to NONE/ENTITY/GROUP/FACTION/EVERYONE; Can only be used with <ownership> parameter", LogLevel.NORMAL);	
//		Print("Console > Keyword 'none/entity/assignees/group/faction/everyone': Sets task visibility to NONE/ENTITY/ASSIGNEES/GROUP/FACTION/EVERYONE; Can only be used with <visibility> parameter", LogLevel.NORMAL);
//		Print("Console > Keyword 'none/map/list/all': Sets task UI visibility to NONE/MAP_ONLY/LIST_ONLY/ALL; Can only be used with <visibility> parameter", LogLevel.NORMAL);
//		
//		Print("Console > Keyword 'all': Displays all available tasks in the scene; Can only be used with command: " + m_sCommandTaskDisplay, LogLevel.NORMAL);
//		Print("Console > Keyword 'true/false': Forces assignment to task; Can only be used with <force/enable> parameter", LogLevel.NORMAL);
//	}
//}
//#endif