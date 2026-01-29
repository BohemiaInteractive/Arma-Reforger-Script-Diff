//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_BaseGroupCommandTitleField("m_sCommandName")]
class SCR_WaypointGroupCommand : SCR_BaseGroupCommand
{
	[Attribute(desc: "Prefab of the AI waypoint used in this command", params: "et class=AIWaypoint")]
	protected ResourceName m_sWaypointPrefab;
	
	[Attribute(defvalue: "1", desc: "Will the command override all previous commands?")]
	protected bool m_bOverridePreviousCommands;
	
	[Attribute(defvalue: "0", desc: "Will the command be automatically set to players position?")]
	protected bool m_bTargetSelf;
	
	[Attribute(defvalue: "0", desc: "Will the command override autonomous group behavior?")]
	protected bool m_bForceCommand;
	
	[Attribute(defvalue: "5", desc: "Completion radius of command")]
	protected float m_fCompletionRadius;
	
	[Attribute("0", UIWidgets.CheckBox, desc: "Optionally, execution of this command can set combat mode of the group.")]
	bool m_bSetCombatMode;
	
	[Attribute(typename.EnumToString(EAIGroupCombatMode, EAIGroupCombatMode.FIRE_AT_WILL), UIWidgets.ComboBox, desc: "New combat mode value", enums: ParamEnumArray.FromEnum(EAIGroupCombatMode))]
	protected EAIGroupCombatMode m_eCombatMode;
	
	[Attribute(desc: "Handler which is going to be called to manipualte the waypoint when it is created as well as when it is assigned to the group.")]
	protected ref SCR_BaseWaypointCommandHandler m_WaypointCommandHandler;

	//------------------------------------------------------------------------------------------------
	override bool Execute(IEntity cursorTarget, IEntity target, vector targetPosition, int playerID, bool isClient)
	{
		SCR_ChimeraCharacter user = SCR_ChimeraCharacter.Cast(GetGame().GetPlayerManager().GetPlayerControlledEntity(playerID));
		if (user && !CanBePerformed(user))
			return false;//verify that at the moment of execution player can actually do that
			
		if (isClient && playerID == SCR_PlayerController.GetLocalPlayerId())
		{
			SpawnWPVisualization(targetPosition, playerID);
			return true;
		}

		SpawnWPVisualization(targetPosition, playerID);
		if (!m_sWaypointPrefab || !target || !targetPosition)
			return false;

		bool waypointCreated = SetWaypointForAIGroup(target, targetPosition, playerID);
		
		if (!waypointCreated)
			return false;
		
		// Set combat mode
		if (m_bSetCombatMode)
			SetCombatMode(target, m_eCombatMode);
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	bool SetWaypointForAIGroup(IEntity target, vector targetPosition, int playerID)
	{
		SCR_AIGroup slaveGroup = SCR_AIGroup.Cast(target);
		if (!slaveGroup)
			return false;
		
		array<AIWaypoint> currentWaypoints = {};
		slaveGroup.GetWaypoints(currentWaypoints);
		foreach(AIWaypoint currentwp : currentWaypoints)
		{
			slaveGroup.RemoveWaypoint(currentwp);
		}
		
		Resource waypointPrefab = Resource.Load(GetWaypointPrefab());
		if (!waypointPrefab.IsValid())
		{
			//add custom error
			return false;
		}
		
		vector position;
		position = GetTargetPosition(playerID, targetPosition);
		
		EntitySpawnParams params = EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		params.Transform[3] = position; 
	
		CheckPreviousWaypoints(slaveGroup);
		
		SCR_AIWaypoint wp = SCR_AIWaypoint.Cast(GetGame().SpawnEntityPrefabLocal(waypointPrefab, null, params));
		if (!wp)
			return false;

		if (m_WaypointCommandHandler)
			m_WaypointCommandHandler.OnWaypointCreated(wp);

		if (m_bForceCommand)
			wp.SetPriorityLevel(SCR_AIActionBase.PRIORITY_LEVEL_PLAYER);
		
		wp.SetCompletionRadius(m_fCompletionRadius);
		slaveGroup.AddWaypoint(wp);
		if (m_WaypointCommandHandler)
			m_WaypointCommandHandler.OnWaypointIssued(wp, slaveGroup);

		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	void SpawnWPVisualization(vector targetPosition, int playerID)
	{
		PlayerController controller = GetGame().GetPlayerManager().GetPlayerController(playerID);
		if (!controller)
			return;
		
		SCR_PlayerControllerCommandingComponent commandingComp = SCR_PlayerControllerCommandingComponent.Cast(controller.FindComponent(SCR_PlayerControllerCommandingComponent));
		if (!commandingComp)
			return;
	
		commandingComp.DrawWaypointVisualization(targetPosition, m_fCompletionRadius);
	}
	
	//------------------------------------------------------------------------------------------------
	ResourceName GetWaypointPrefab()
	{
		return m_sWaypointPrefab;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetWaypointPrefab(ResourceName wpPrefab)
	{
		m_sWaypointPrefab = wpPrefab;
	}
	
	//------------------------------------------------------------------------------------------------
	bool GetOverridePreviousCommands()
	{
		return m_bOverridePreviousCommands;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetOverridePreviousCommands(bool overridePrevious)
	{
		m_bOverridePreviousCommands = overridePrevious;
	}
	
	//----------------------------------------------------------------------------------------------
	vector GetTargetPosition(int playerID, vector targetPosition)
	{
		if (m_bTargetSelf)
		{
			SCR_PlayerController orderingPlayerController = SCR_PlayerController.Cast(GetGame().GetPlayerManager().GetPlayerController(playerID));
			IEntity playerEntity = orderingPlayerController.GetControlledEntity();
			if (!playerEntity)
				return vector.Zero;
			return playerEntity.GetOrigin();
		}
		else
			return targetPosition;
	}
	
	//-------------------------------------------------------------------------------------------------
	void CheckPreviousWaypoints(SCR_AIGroup slaveGroup)
	{
		if (!m_bOverridePreviousCommands)
			return;
		
		array<AIWaypoint> currentWaypoints = {};
		slaveGroup.GetWaypoints(currentWaypoints);
		foreach(AIWaypoint currentwp : currentWaypoints)
		{
			slaveGroup.RemoveWaypoint(currentwp);
		}
		AIGroupMovementComponent slaveGroupMovementComp = AIGroupMovementComponent.Cast(slaveGroup.FindComponent(AIGroupMovementComponent));
		if (slaveGroupMovementComp)
			slaveGroupMovementComp.SetFormationDisplacement(0);
	}

	//------------------------------------------------------------------------------------------------
	override bool CanShowOnMap()
	{
		return CanBeShown();
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBeShown()
	{
		if (!CanBeShownInCurrentLifeState())
			return false;
		
		SCR_PlayerControllerGroupComponent groupController = SCR_PlayerControllerGroupComponent.GetLocalPlayerControllerGroupComponent();
		return groupController && CanRoleShow();
	}
	
	//------------------------------------------------------------------------------------------------
	void SetCombatMode(IEntity target, EAIGroupCombatMode combatMode)
	{
		SCR_AIGroup slaveGroup = SCR_AIGroup.Cast(target);
		if (!slaveGroup)
			return;
		
		SCR_AIGroupUtilityComponent groupUtility = SCR_AIGroupUtilityComponent.Cast(slaveGroup.FindComponent(SCR_AIGroupUtilityComponent));
		
		groupUtility.SetCombatMode(combatMode);
	}
}