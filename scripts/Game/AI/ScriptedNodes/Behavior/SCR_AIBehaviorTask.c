class SCR_AIActionTask : AITaskScripted
{
	// This is parent class not to be used / visible in palette of AI nodes
	
	static const string ENTITY_PORT = "Entity";
	static const string POSITION_PORT = "Position";
	static const string STANCE_PORT = "Stance";
	static const string ROLEINVEHICLE_PORT = "RoleInVehicle";
	static const string REPORTER_PORT = "ReportingAgent";
	static const string USE_VEHICLES_PORT = "UseVehicles";
	static const string DESIREDDISTANCE_PORT = "DesiredDistance";
	static const string TARGET_PORT = "Target";
	static const string TARGETINFO_PORT = "TargetInfo";
	static const string TARGETPOSITION_PORT = "TargetPosition";
	static const string NEXTCOVERPOSITION_PORT = "NextCoverPosition";
	static const string COMPARTMENT_PORT = "Compartment";
	static const string SMARTACTION_PORT = "SmartActionComponent";
	static const string WEAPON_TYPE_PORT = "WeaponType";
	static const string WEAPON_COMP_PORT = "WeaponComponent";
	static const string MAGAZINE_COMP_PORT = "MagazineComponent";
	static const string MAGAZINE_WELL_PORT = "MagazineWell";
	static const string PRIORITY_LEVEL_PORT = "PriorityLevel";
	static const string MUZZLE_ID = "MuzzleID";
	static const string WAYPOINT_PORT = "Waypoint";
	static const string BOARDING_PARAMS_PORT = "BoardingParams";
	static const string DEFEND_DIRECTION_PORT = "DefendDirection";
	static const string DEFEND_LOCATION_PORT = "DefendLocation";
	static const string ANGULAR_RANGE_PORT = "AngularRange";
	static const string WAYPOINT_RELATED_PORT = "WaypointRelated";
	static const string IS_DANGEROUS_PORT = "isDangerous";
	static const string RESET_TIMER_PORT = "ResetTimer";
	static const string TIMEOUT_PORT = "Timeout";
	static const string DELAY_PORT = "Delay";
	static const string REINIT_PORT = "Reinit";
	static const string RADIUS_PORT = "Radius";
	static const string AGENT_SCRIPT_PORT = "AgentScript";
	static const string RELATED_INVOKER_PORT = "RelatedInvoker";
	
	protected SCR_AIBaseUtilityComponent m_UtilityComp;
	
	//------------------------------------------------------------------------------------------------
	override void OnEnter(AIAgent owner)
	{
		m_UtilityComp = SCR_AIBaseUtilityComponent.Cast(owner.FindComponent(SCR_AIBaseUtilityComponent));
		if (!m_UtilityComp)
		{
			NodeError(this, owner, "Can't find base utility component.");
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns current action from the utility component
	protected SCR_AIActionBase GetExecutedAction()
	{
		if (!m_UtilityComp)
			return null;
		
		return SCR_AIActionBase.Cast(m_UtilityComp.GetExecutedAction());
	}
};