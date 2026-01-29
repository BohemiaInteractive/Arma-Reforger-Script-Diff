class SCR_SoundEvent
{
	// Communication protocol
	static const string SOUND_CP_NEGATIVEFEEDBACK_NEGATIVE = "SOUND_CP_NEGATIVEFEEDBACK_NEGATIVE";
	static const string SOUND_CP_TARGET = "SOUND_CP_TARGET";
	static const string SOUND_CP_MOVE_MID = "SOUND_CP_MOVE_MID";
	static const string SOUND_CP_MOVE_CLOSE = "SOUND_CP_MOVE_CLOSE";
	static const string SOUND_CP_MOVE_LONG = "SOUND_CP_MOVE_LONG";
	static const string SOUND_CP_RETURN = "SOUND_CP_RETURN";
	static const string SOUND_CP_MOUNT_BOARD = "SOUND_CP_MOUNT_BOARD";
	static const string SOUND_CP_MOUNT_ROLE = "SOUND_CP_MOUNT_ROLE";
	static const string SOUND_CP_MOUNT_GETOUT = "SOUND_CP_MOUNT_GETOUT";
	static const string SOUND_CP_MOUNT_NEAREST = "SOUND_CP_MOUNT_NEAREST_VEHICLE";
	static const string SOUND_CP_STOP = "SOUND_CP_STOP";
	static const string SOUND_CP_FLANK = "SOUND_CP_FLANK";
	static const string SOUND_CP_SPOTTED_CLOSE = "SOUND_CP_SPOTTED_CLOSE";
	static const string SOUND_CP_SPOTTED_MID = "SOUND_CP_SPOTTED_MID";
	static const string SOUND_CP_SPOTTED_LONG = "SOUND_CP_SPOTTED_LONG";
	static const string SOUND_REPORTS_STATUS_NOAMMO = "SOUND_REPORTS_STATUS_NOAMMO";
	static const string SOUND_REPORTS_STATUS_CLEAR = "SOUND_REPORTS_STATUS_CLEAR";
	static const string SOUND_REPORTS_STATUS_ENGAGING = "SOUND_REPORTS_STATUS_ENGAGING";
	static const string SOUND_REPORTS_STATUS_TARGETDOWN = "SOUND_REPORTS_STATUS_TARGETDOWN";
	static const string SOUND_REPORTS_ACTIONSTATUS_RELOAD = "SOUND_REPORTS_ACTIONSTATUS_RELOAD";
	static const string SOUND_REPORTS_ACTIONSTATUS_MOVE = "SOUND_REPORTS_ACTIONSTATUS_MOVE";
	static const string SOUND_REPORTS_ACTIONSTATUS_COVER = "SOUND_REPORTS_ACTIONSTATUS_COVER";
	static const string SOUND_CP_DEFEND_POSITION = "SOUND_CP_DEFEND_POSITION";
	static const string SOUND_CP_FOLLOW_LEADER = "SOUND_CP_FOLLOW_LEADER";
	static const string SOUND_CP_POSITIVEFEEDBACK = "SOUND_CP_POSITIVEFEEDBACK";
	static const string SOUND_CP_SEARCH_AND_DESTROY = "SOUND_CP_SEARCH_AND_DESTROY";
	static const string SOUND_CP_IDLE_SPEECH = "SOUND_CP_IDLE_SPEECH";
	static const string SOUND_CP_REPORTS_AREACLEAR = "SOUND_CP_REPORTS_AREACLEAR";
	static const string SOUND_CP_REPORTS_UNDER_FIRE = "SOUND_REPORTS_STATUS_UNDERFIRE";	

	//Communication protocol - Campaign
	static const string SOUND_HQ_MOB = "SOUND_HQ_MOB";
	static const string SOUND_HQ_FOB = "SOUND_HQ_FOB";
	static const string SOUND_HQ_COP = "SOUND_HQ_COP";
	static const string SOUND_HQ_REN = "SOUND_HQ_REN";
	static const string SOUND_HQ_DEM = "SOUND_HQ_DEM";
	static const string SOUND_HQ_POP = "SOUND_HQ_POP";
	static const string SOUND_HQ_POC = "SOUND_HQ_POC";
	static const string SOUND_HQ_POS = "SOUND_HQ_POS";
	static const string SOUND_HQ_POL = "SOUND_HQ_POL";
	static const string SOUND_HQ_PON = "SOUND_HQ_PON";
	static const string SOUND_HQ_POM = "SOUND_HQ_POM";
	static const string SOUND_HQ_RIF = "SOUND_HQ_RIF";
	static const string SOUND_HQ_VIC = "SOUND_HQ_VIC";
	static const string SOUND_HQ_DEF = "SOUND_HQ_DEF";
	static const string SOUND_SL_RRD = "SOUND_SL_RRD";
	static const string SOUND_SL_ERT = "SOUND_SL_ERT";
	static const string SOUND_SL_RRT = "SOUND_SL_RRT";
	static const string SOUND_SL_SDD = "SOUND_SL_SDD";
	static const string SOUND_SL_REI = "SOUND_SL_REI";
	static const string SOUND_SL_TRT = "SOUND_SL_TRT";
	static const string SOUND_SL_CSR = "SOUND_SL_CSR";
	static const string SOUND_SL_SRT = "SOUND_SL_SRT";
	static const string SOUND_HQ_RRU = "SOUND_HQ_RRU";
	static const string SOUND_HQ_TRU = "SOUND_HQ_TRU";
	static const string SOUND_HQ_ETU = "SOUND_HQ_ETU";
	static const string SOUND_HQ_RCR = "SOUND_HQ_RCR";
	static const string SOUND_SL_CHR = "SOUND_SL_CHR";
	static const string SOUND_HQ_BAL = "SOUND_HQ_BAL";
	static const string SOUND_HQ_CTL = "SOUND_HQ_CTL";
	static const string SOUND_HQ_BUA = "SOUND_HQ_BUA";
	static const string SOUND_HQ_BAA = "SOUND_HQ_BAA";
	static const string SOUND_HQ_BFA = "SOUND_HQ_BFA";
	static const string SOUND_HQ_BRA = "SOUND_HQ_BRA";
	static const string SOUND_HQ_BAD = "SOUND_HQ_BAD";
	static const string SOUND_HQ_BFD = "SOUND_HQ_BFD";
	static const string SOUND_HQ_BRD = "SOUND_HQ_BRD";
	static const string SOUND_HQ_BAR = "SOUND_HQ_BAR";
	static const string SOUND_HQ_BFR = "SOUND_HQ_BFR";
	static const string SOUND_HQ_BRR = "SOUND_HQ_BRR";
	static const string SOUND_HQ_ACB = "SOUND_HQ_ACB";
	static const string SOUND_HQ_BCB = "SOUND_HQ_BCB";
	static const string SOUND_HQ_SCB = "SOUND_HQ_SCB";
	static const string SOUND_HQ_LCB = "SOUND_HQ_LCB";
	static const string SOUND_HQ_VCB = "SOUND_HQ_VCB";
	static const string SOUND_HQ_PMV = "SOUND_HQ_PMV";
	static const string SOUND_HQ_PMD = "SOUND_HQ_PMD";
	static const string SOUND_HQ_PMC = "SOUND_HQ_PMC";
	static const string SOUND_HQ_PML = "SOUND_HQ_PML";
	static const string SOUND_HQ_HCB = "SOUND_HQ_HCB";
	static const string SOUND_HQ_RRL = "SOUND_HQ_RRL";
	static const string SOUND_HQ_BHA = "SOUND_HQ_BHA";

	// HQ Communication protocol - Campaign, new version
	static const string SOUND_HQC_M_RENEGADE_COMMANDER = "SOUND_HQC_M_RENEGADE_COMMANDER";
	static const string SOUND_HQC_M_DEMOTE_COMMANDER = "SOUND_HQC_M_DEMOTE_COMMANDER";
	static const string SOUND_HQC_M_PPRIVATE_COMMANDER = "SOUND_HQC_M_PPRIVATE_COMMANDER";
	static const string SOUND_HQC_M_PCORPORAL_COMMANDER = "SOUND_HQC_M_PCORPORAL_COMMANDER";
	static const string SOUND_HQC_M_PSERGEANT_COMMANDER = "SOUND_HQC_M_PSERGEANT_COMMANDER";
	static const string SOUND_HQC_M_PLIEUTENANT_COMMANDER = "SOUND_HQC_M_PLIEUTENANT_COMMANDER";
	static const string SOUND_HQC_M_PCAPTAIN_COMMANDER = "SOUND_HQC_M_PCAPTAIN_COMMANDER";
	static const string SOUND_HQC_M_PMAJOR_COMMANDER = "SOUND_HQC_M_PMAJOR_COMMANDER";
	static const string SOUND_HQC_M_BASEUNDERATTACK_COMMANDER = "SOUND_HQC_M_BASEUNDERATTACK_COMMANDER";
	static const string SOUND_HQC_M_VICTORY_COMMANDER = "SOUND_HQC_M_VICTORY_COMMANDER";
	static const string SOUND_HQC_M_DEFEAT_COMMANDER = "SOUND_HQC_M_DEFEAT_COMMANDER";
	static const string SOUND_HQC_M_WINNING_COMMANDER = "SOUND_HQC_M_WINNING_COMMANDER";
	static const string SOUND_HQC_M_LOSING_COMMANDER = "SOUND_HQC_M_LOSING_COMMANDER";

	// HQ COMMANDER
	static const string SOUND_PA_HQCOMMANDER_SUPPLIES_ARRIVED = "SOUND_PA_HQCOMMANDER_SUPPLIES_ARRIVED";
	static const string SOUND_HQC_ARRIVAL_HQCBATTALION = "SOUND_HQC_ARRIVAL_HQCBATTALION";
	static const string SOUND_HQC_RESIGNATION_HQCBATTALION = "SOUND_HQC_RESIGNATION_HQCBATTALION";
	static const string SOUND_HQC_DISRUPTION_HQCBATTALION = "SOUND_HQC_DISRUPTION_HQCBATTALION";

	//Combat Ops
	static const string	SOUND_CP_IPL_1 = "SOUND_CP_IPL_1";				//Insertion plan
	static const string	SOUND_CP_IPL_2 = "SOUND_CP_IPL_2";				//Insertion plan
	static const string	SOUND_CP_IPL_3 = "SOUND_CP_IPL_3";				//Insertion plan
	static const string	SOUND_CP_TD_PCL = "SOUND_CP_TD_PCL";			//Task Destroy
	static const string	SOUND_CP_TD_VCL = "SOUND_CP_TD_VCL";			//Task Destroy
	static const string	SOUND_CP_TD_SCL = "SOUND_CP_TD_SCL";			//Task Destroy
	static const string	SOUND_CP_TD_CCL = "SOUND_CP_TD_CCL";			//Task Destroy
	static const string	SOUND_CP_TD_PC = "SOUND_CP_TD_PC";				//Task Destroy
	static const string	SOUND_CP_TD_VC = "SOUND_CP_TD_VC";				//Task Destroy
	static const string	SOUND_CP_TD_SC = "SOUND_CP_TD_SC";				//Task Destroy
	static const string	SOUND_CP_TD_CC = "SOUND_CP_TD_CC";				//Task Destroy
	static const string	SOUND_CP_TIF = "SOUND_CP_TIF";					//Task Intel Found
	static const string	SOUND_CP_TACC = "SOUND_CP_TACC";				//Task Clear Area
	static const string	SOUND_CP_TETL = "SOUND_CP_TETL";				//Task Extraction
	static const string	SOUND_CP_TETW = "SOUND_CP_TETW";				//Task Extraction
	static const string	SOUND_CP_TETN = "SOUND_CP_TETN";				//Task Extraction
	static const string	SOUND_CP_TEC = "SOUND_CP_TEC";					//Task Extraction
	static const string	SOUND_CP_TEF_1 = "SOUND_CP_TEF_1";				//Task Extraction Failed
	static const string	SOUND_CP_TEF_2 = "SOUND_CP_TEF_2";				//Task Extraction Failed

	//UI sounds
	static const string SOUND_LOADSUPPLIES = "SOUND_LOADSUPPLIES";
	static const string SOUND_UNLOADSUPPLIES = "SOUND_UNLOADSUPPLIES";
	static const string SOUND_SIREN = "SOUND_SIREN";
	static const string SOUND_FE_ITEM_CHANGE = "SOUND_FE_ITEM_CHANGE";
	static const string SOUND_FE_BUTTON_HOVER = "SOUND_FE_BUTTON_HOVER";
	static const string SOUND_RESPAWN_COUNTDOWN = "SOUND_RESPAWN_COUNTDOWN";
	static const string SOUND_RESPAWN_COUNTDOWN_END = "SOUND_RESPAWN_COUNTDOWN_END";

	static const string FOCUS = "SOUND_FE_BUTTON_SELECT";
	static const string CLICK = "SOUND_FE_BUTTON_CONFIRM";
	static const string CLICK_CANCEL = "SOUND_FE_BUTTON_CANCEL";
	static const string CLICK_FAIL = "SOUND_FE_BUTTON_FAIL";
	static const string ERROR = "SOUND_FE_ERROR";
	static const string TAB_CLICK = "SOUND_FE_TAB_CONFIRM";
	static const string TAB_SWITCH = "SOUND_FE_TAB_SWITCH";
	static const string SOUND_FE_TEXT_ENTERED = "SOUND_FE_TEXT_ENTERED";
	static const string SOUND_FE_TEXT_LEFT = "SOUND_FE_TEXT_LEFT";

	static const string TURN_PAGE = "SOUND_FE_TURN_PAGE";

	static const string TOGGLE_ON = "SOUND_FE_BUTTON_FILTER_ON";
	static const string TOGGLE_OFF = "SOUND_FE_BUTTON_FILTER_OFF";

	static const string TASK_CREATED = "SOUND_HUD_TASK_CREATED";
	static const string TASK_SUCCEED = "SOUND_HUD_TASK_SUCCEEDED";
	static const string TASK_FAILED = "SOUND_HUD_TASK_FAILED";
	static const string TASK_CANCELED = "SOUND_HUD_TASK_CANCELED";
	static const string HINT = "SOUND_HUD_NOTIFICATION";
	static const string TASK_ACCEPT = "SOUND_HUD_TASK_ACCEPT";

	static const string ITEM_SELECTED = "SOUND_HUD_GADGET_SELECT";
	static const string ITEM_CONFIRMED = "SOUND_HUD_GADGET_CONFIRM";
	static const string ITEM_CANCELLED = "SOUND_HUD_GADGET_CANCEL";
	static const string MAP_PAN = "SOUND_HUD_MAP_MOVEMENT";

	static const string POINTS_ADDED = "SOUND_HUD_CAMPAIGN_POINTS_ADD";
	static const string POINTS_REMOVED ="SOUND_HUD_CAMPAIGN_POINTS_SUBSTRACT";

	static const string ACTION_FAILED = "SOUND_HUD_ACTION_CANTPERFORM";

	static const string SOUND_MAP_CLICK_POINT_ON = "SOUND_MAP_CLICK_POINT_ON";

	// UI sounds - HUD
	static const string SOUND_HUD_MAP_OPEN = "SOUND_HUD_MAP_OPEN";
	static const string SOUND_HUD_MAP_CLOSE = "SOUND_HUD_MAP_CLOSE";
	static const string SOUND_MAP_HOVER_BASE = "SOUND_MAP_HOVER_BASE";
	static const string SOUND_MAP_HOVER_ENEMY = "SOUND_MAP_HOVER_ENEMY";
	static const string SOUND_MAP_HOVER_TRANS_TOWER = "SOUND_MAP_HOVER_TRANS_TOWER";
	static const string SOUND_MAP_GADGET_SHOW = "SOUND_MAP_GADGET_SHOW";
	static const string SOUND_MAP_GADGET_HIDE  = "SOUND_MAP_GADGET_HIDE";
	static const string SOUND_MAP_GADGET_GRAB  = "SOUND_MAP_GADGET_GRAB";
	static const string SOUND_MAP_GADGET_RELEASE  = "SOUND_MAP_GADGET_RELEASE";
	static const string SOUND_MAP_GADGET_MARKER_DRAW_START  = "SOUND_MAP_GADGET_MARKER_DRAW_START";
	static const string SOUND_MAP_GADGET_MARKER_DRAW_STOP  = "SOUND_MAP_GADGET_MARKER_DRAW_STOP";
	static const string SOUND_MAP_GADGET_MARKER_REMOVE  = "SOUND_MAP_GADGET_MARKER_REMOVE";
	static const string SOUND_HUD_TASK_MENU_OPEN = "SOUND_HUD_TASK_MENU_OPEN";
	static const string SOUND_HUD_TASK_MENU_CLOSE = "SOUND_HUD_TASK_MENU_CLOSE";
	static const string SOUND_FE_HUD_PAUSE_MENU_OPEN = "SOUND_FE_HUD_PAUSE_MENU_OPEN";
	static const string SOUND_FE_HUD_PAUSE_MENU_CLOSE = "SOUND_FE_HUD_PAUSE_MENU_CLOSE";

	// UI sounds - Inventory
	static const string SOUND_INV_HOTKEY_CONFIRM = "SOUND_INV_HOTKEY_CONFIRM";
	static const string SOUND_INV_HOTKEY_SCROLL = "SOUND_INV_HOTKEY_SCROLL";
	static const string SOUND_INV_HOTKEY_OPEN = "SOUND_INV_HOTKEY_OPEN";
	static const string SOUND_INV_HOTKEY_CLOSE = "SOUND_INV_HOTKEY_CLOSE";
	static const string SOUND_INV_OPEN = "SOUND_INV_OPEN";
	static const string SOUND_INV_CLOSE = "SOUND_INV_CLOSE";
	static const string SOUND_INV_CONTAINER_DRAG = "SOUND_INV_CONTAINER_DRAG";
	static const string SOUND_INV_PICKUP_CLICK = "SOUND_INV_PICKUP_CLICK";
	static const string SOUND_INV_VICINITY_EQUIP_CLICK = "SOUND_INV_VICINITY_EQUIP_CLICK";
	static const string SOUND_INV_VICINITY_DROP_CLICK = "SOUND_INV_VICINITY_DROP_CLICK";
	static const string SOUND_INV_CONTAINER_DIFR_DROP = "SOUND_INV_CONTAINER_DIFR_DROP";
	static const string SOUND_INV_CONTAINER_SAME_DROP = "SOUND_INV_CONTAINER_SAME_DROP";
	static const string SOUND_INV_CONTAINER_CLOSE = "SOUND_INV_CONTAINER_CLOSE";
	static const string SOUND_INV_DROP_ERROR = "SOUND_INV_DROP_ERROR";
	static const string SOUND_INV_VICINITY_DRAG = "SOUND_INV_VICINITY_DRAG";
	static const string SOUND_INV_WEAPON_INSPECT = "SOUND_INV_INSPECT";
	static const string SOUND_INV_QUICKSLOT_ASSIGN = "SOUND_INV_CONTAINER_SAME_DROP";
	static const string SOUND_INV_QUICKSLOT_CLEAR = "SOUND_INV_CONTAINER_DRAG";

	//UI sounds - Editor
	static const string SOUND_E_MULTI_SELECT_START_KEYBOARD = "SOUND_E_MULTI_SELECT_START_KEYBOARD";
	static const string SOUND_E_MULTI_SELECT_START_GAMEPAD = "SOUND_E_MULTI_SELECT_START_GAMEPAD";
	static const string SOUND_FE_BUTTON_SELECT = "SOUND_FE_BUTTON_SELECT";
	static const string SOUND_E_TRAN_CANCEL = "SOUND_E_TRAN_CANCEL";
	static const string SOUND_E_LAYER_EDIT_START = "SOUND_E_LAYER_EDIT_START";
	static const string SOUND_E_LAYER_EDIT_END = "SOUND_E_LAYER_EDIT_END";
	static const string SOUND_E_LAYER_DEEPER = "SOUND_E_LAYER_DEEPER";
	static const string SOUND_E_LAYER_BACK = "SOUND_E_LAYER_BACK";

	//Vehicles
	static const string SOUND_VEHICLE_RAIN = "SOUND_VEHICLE_RAIN";
	static const string SOUND_ENGINE_START = "SOUND_ENGINE_START";
	static const string SOUND_ENGINE_STARTER_LP = "SOUND_ENGINE_STARTER_LP";
	static const string SOUND_ENGINE_START_FAILED = "SOUND_ENGINE_START_FAILED";
	static const string SOUND_ENGINE_STOP = "SOUND_ENGINE_STOP";
	static const string SOUND_TIRE_PUNCTURE = "SOUND_TIRE_PUNCTURE";
	static const string SOUND_VEHICLE_TRUNK_CLOSE = "SOUND_VEHICLE_TRUNK_CLOSE";
	static const string SOUND_VEHICLE_CLOSE_LIGHT_ON = "SOUND_VEHICLE_CLOSE_LIGHT_ON";
	static const string SOUND_VEHICLE_CLOSE_LIGHT_OFF = "SOUND_VEHICLE_CLOSE_LIGHT_OFF";
	static const string SOUND_VEHICLE_WATER_SMALL = "SOUND_VEHICLE_IMPACT_WATER_SMALL";
	static const string SOUND_VEHICLE_WATER_MEDIUM = "SOUND_VEHICLE_IMPACT_WATER_MEDIUM";
	static const string SOUND_VEHICLE_WATER_BIG = "SOUND_VEHICLE_IMPACT_WATER_BIG";
	
	//Supplies Sounds
	static const string SOUND_SUPPLIES_PARTIAL_LOAD = "SOUND_SUPPLIES_PARTIAL_LOAD";
	static const string SOUND_SUPPLIES_PARTIAL_UNLOAD = "SOUND_SUPPLIES_PARTIAL_UNLOAD";
	static const string SOUND_SUPPLIES_VEH_CONTAINER_APPEAR = "SOUND_SUPPLIES_VEH_CONTAINER_APPEAR";
	static const string SOUND_SUPPLIES_VEH_CONTAINER_DISAPPEAR = "SOUND_SUPPLIES_VEH_CONTAINER_DISAPPEAR";
	static const string SOUND_SUPPLIES_VEH_FULL = "SOUND_SUPPLIES_VEH_FULL";
	
	//Helicopters
	static const string SOUND_AUTOHOVER_ON = "SOUND_AUTOHOVER_ON";
	static const string SOUND_AUTOHOVER_OFF = "SOUND_AUTOHOVER_OFF";
	static const string SOUND_ROTOR_WASH_LP = "SOUND_ROTOR_WASH_LP";

	//Campaign
	static const string SOUND_RADIO_ESTABLISH_ACTION = "SOUND_RADIO_ESTABLISH_ACTION";
	static const string SOUND_RADIO_CHATTER_EV = "SOUND_RADIO_CHATTER_EV";
	static const string SOUND_RADIO_CHATTER_US = "SOUND_RADIO_CHATTER_US";
	static const string SOUND_RADIO_CHATTER_RU = "SOUND_RADIO_CHATTER_RU";
	static const string SOUND_RADIO_SIGNAL_GAIN = "SOUND_RADIO_SIGNAL_GAIN";
	static const string SOUND_RADIO_SIGNAL_LOST = "SOUND_RADIO_SIGNAL_LOST";
	static const string SOUND_BUILD = "SOUND_BUILD";
	static const string SOUND_CANCLELBUILDING = "SOUND_CANCLELBUILDING";
	static const string SOUND_DISASSEMBLY = "SOUND_DISASSEMBLY";
	static const string SOUND_STARTBUILDING = "SOUND_STARTBUILDING";

	//AmbientSounds
	static const string SOUND_LEAFYTREE_SMALL_LP = "SOUND_LEAFYTREE_SMALL_LP";
	static const string SOUND_LEAFYTREE_MEDIUM_LP = "SOUND_LEAFYTREE_MEDIUM_LP";
	static const string SOUND_LEAFYTREE_LARGE_LP = "SOUND_LEAFYTREE_LARGE_LP";
	static const string SOUND_LEAFYTREE_VERYLARGE_LP = "SOUND_LEAFYTREE_VERYLARGE_LP";
	static const string SOUND_BUSH_LP = "SOUND_BUSH_LP";
	static const string SOUND_BUSH_CRICKETS_LP = "SOUND_BUSH_CRICKETS_LP";

	// Gadgets
	static const string SOUND_FLASHLIGHT_ON = "SOUND_FLASHLIGHT_ON";
	static const string SOUND_FLASHLIGHT_OFF = "SOUND_FLASHLIGHT_OFF";
	static const string SOUND_ITEM_RADIO_TOGGLE_ON = "SOUND_ITEM_RADIO_TOGGLE_ON";
	static const string SOUND_ITEM_RADIO_TOGGLE_OFF = "SOUND_ITEM_RADIO_TOGGLE_OFF";
	static const string SOUND_ITEM_RADIO_TUNE_UP = "SOUND_ITEM_RADIO_TUNE_UP";
	static const string SOUND_ITEM_RADIO_TUNE_DOWN = "SOUND_ITEM_RADIO_TUNE_DOWN";
	static const string SOUND_ITEM_RADIO_TUNE_ERROR = "SOUND_ITEM_RADIO_TUNE_ERROR";
	static const string SOUND_DEPLOY = "SOUND_DEPLOY";
	static const string SOUND_UNDEPLOY = "SOUND_UNDEPLOY";
	static const string SOUND_DEPLOYED_RADIO_ENTER_ZONE = "SOUND_DEPLOYED_RADIO_ENTER_ZONE";
	static const string SOUND_DEPLOYED_RADIO_EXIT_ZONE = "SOUND_DEPLOYED_RADIO_EXIT_ZONE";
	static const string SOUND_DETONATOR_DETONATE_CHARGES = "SOUND_DETONATE";
	static const string SOUND_DETONATOR_DISCONNECT_WIRES = "SOUND_DISCONNECT";
	static const string SOUND_SHELL_CHARGE_RING_ADD = "SOUND_SHELL_CHARGE_RING_ADD";
	static const string SOUND_SHELL_CHARGE_RING_REMOVE = "SOUND_SHELL_CHARGE_RING_REMOVE";

	//BellSoundComponent
	static const string SOUND_BELL_END = "SOUND_BELL_END";
	static const string SOUND_BELL_A = "SOUND_BELL_A";
	static const string SOUND_BELL_B = "SOUND_BELL_B";

	//BuildingSoundComponent
	static const string SOUND_CREAK = "SOUND_CREAK";

	//Firing range
	static const string SOUND_RANGECP_STARTBUTTON = "SOUND_RANGECP_STARTBUTTON";
	static const string SOUND_RANGECP_ROUNDSTART = "SOUND_RANGECP_ROUNDSTART";
	static const string SOUND_RANGECP_ROUNDABORT = "SOUND_RANGECP_ROUNDABORT";
	static const string SOUND_RANGECP_CHANGEDISTANCE = "SOUND_RANGECP_CHANGEDISTANCE";
	static const string SOUND_RANGECP_CHANGETARGET = "SOUND_RANGECP_CHANGETARGET";
	static const string SOUND_TARGET_DOWN = "SOUND_TARGET_DOWN";
	static const string SOUND_TARGET_UP = "SOUND_TARGET_UP";

	// Character
	static const string SOUND_MELEE_IMPACT = "SOUND_MELEE_IMPACT";
	static const string SOUND_MELEE_IMPACT_BAYONET = "SOUND_MELEE_IMPACT_BAYONET";
	static const string SOUND_INJURED_PLAYERCHARACTER = "SOUND_INJURED_PLAYERCHARACTER";
	static const string SOUND_HIT = "SOUND_HIT";
	static const string SOUND_DEATH = "SOUND_DEATH";
	static const string SOUND_BODYFALL_TERMINAL = "SOUND_BODYFALL_TERMINAL";
	static const string SOUND_KNOCKOUT = "SOUND_BREATH_OUT";
	static const string SOUND_VOICE_PAIN_RELIEVE = "SOUND_VOICE_PAIN_RELIEVE";
	static const string SOUND_CHAR_MOVEMENT_WEAPON_SIGHT_TOGGLE = "SOUND_CHAR_MOVEMENT_WEAPON_SIGHT_TOGGLE";
	
	// Weapon
	static const string SOUND_SCOPE_ILLUM_ON = "SOUND_SCOPE_ILLUM_ON";
	static const string SOUND_SCOPE_ILLUM_OFF = "SOUND_SCOPE_ILLUM_OFF";
	static const string SOUND_SCOPE_ZOOM_IN = "SOUND_SCOPE_ZOOM_IN";
	static const string SOUND_SCOPE_ZOOM_OUT = "SOUND_SCOPE_ZOOM_OUT";
	static const string SOUND_PLACE_OBJECT = "SOUND_PLACEMENT";
	static const string SOUND_EXPLOSIVE_CONNECT_WIRES = "SOUND_CONNECT";
	static const string SOUND_EXPLOSIVE_DISARM = "SOUND_DISARM";
	static const string SOUND_EXPLOSIVE_ARM = "SOUND_ARM";
	static const string SOUND_EXPLOSIVE_ADJUST_TIMER = "SOUND_TIMER_ADJUST";
	static const string SOUND_LOAD = "SOUND_LOAD";
	static const string SOUND_SIGHT_ELEVATION = "SOUND_SIGHT_ELEVATION";

	// Actions
	static const string SOUND_EQUIP = "SOUND_EQUIP";
	static const string SOUND_UNEQUIP = "SOUND_UNEQUIP";
	static const string SOUND_PICK_UP = "SOUND_PICK_UP";
	static const string SOUND_DROP = "SOUND_DROP";
	static const string SOUND_TURN_ON = "SOUND_TURN_ON";
	static const string SOUND_TURN_OFF = "SOUND_TURN_OFF";
	static const string SOUND_CONTAINER_OPEN = "SOUND_CONTAINER_OPEN";
	static const string SOUND_CONTAINER_CLOSE = "SOUND_CONTAINER_CLOSE";
	static const string SOUND_TOILET = "SOUND_TOILET";
	static const string SOUND_PLAY_INSTRUMENT = "SOUND_PLAY_INSTRUMENT";
	static const string SOUND_RADIO_CHANGEFREQUENCY_ERROR = "SOUND_RADIO_CHANGEFREQUENCY_ERROR";
	static const string SOUND_RADIO_CHANGEFREQUENCY = "SOUND_RADIO_CHANGEFREQUENCY";
	static const string SOUND_RADIO_TURN_ON = "SOUND_RADIO_TURN_ON";
	static const string SOUND_RADIO_TURN_OFF = "SOUND_RADIO_TURN_OFF";
	static const string SOUND_RADIO_FREQUENCY_CYCLE = "SOUND_RADIO_FREQUENCY_CYCLE";
	static const string SOUND_MINEFLAG_PLACE = "SOUND_MINEFLAG_PLACE";

	//Destruction
	static const string SOUND_BUILDING_CRACK = "SOUND_BUILDING_CRACK";
	static const string SOUND_HIT_GROUND = "SOUND_HIT_GROUND";
	static const string SOUND_BREAK = "SOUND_BREAK";
	static const string SOUND_MPD_ = "SOUND_MPD_"; 					//Material type is being added at the end of the string

	// Music
	static const string SOUND_ONENTERINGENEMYBASE = "SOUND_ONENTERINGENEMYBASE";
	static const string SOUND_RESPAWNMENU = "SOUND_RESPAWNMENU";
	static const string SOUND_ONSPAWN = "SOUND_ONSPAWN";
	static const string SOUND_ONDEATH = "SOUND_ONDEATH";
	static const string SOUND_ONBASECAPTURE = "SOUND_ONBASECAPTURE";
	
	// Music Instruments
	static const string SOUND_STOP_PLAYING = "SOUND_STOP_PLAYING";
	
	static const string SOUND_MHQ_DEPLOY = "SOUND_MHQ_DEPLOY";
	static const string SOUND_MHQ_DISMANTLE = "SOUND_MHQ_DISMANTLE";
}
