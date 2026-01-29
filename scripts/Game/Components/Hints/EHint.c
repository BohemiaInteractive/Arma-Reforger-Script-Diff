//! Hint types.
//! When a hint is shown, its type, when defined, is stored persistently.
//! After that the hint will not be shown anymore.
//!
//! Assign specific value to each entry!
//! Otherwise it may be shifted when new entries are added in front of it,
//! resulting in unintended type change in existing hints.
//!
//! This is because values are saved as numbers in SCR_HintSettings.
enum EHint
{
	UNDEFINED = 0, ///< Hint type will not be stored persistently if it has default value
	
	EDITOR_TOGGLE					= 100,
	
	EDITOR_MODE_EDIT				= 101,
	EDITOR_MODE_ADMIN				= 102,
	EDITOR_MODE_PHOTO_LIMITED		= 103,
	EDITOR_MODE_PHOTO_UNLIMITED		= 118,
	
	EDITOR_SELECT_ENTITY			= 104,
	EDITOR_SELECT_FACTION			= 105,
	EDITOR_SELECT_GROUP				= 106,
	
	EDITOR_PLACE					= 107,
	EDITOR_PLACE_CHARACTER			= 108,
	EDITOR_PLACE_VEHICLE			= 109,
	EDITOR_PLACE_WAYPOINT			= 123,
	EDITOR_PLACE_PLAYER				= 124,
	EDITOR_PLACE_TASK				= 110,
	
	EDITOR_ATTRIBUTES_GLOBAL		= 111,
	EDITOR_ATTRIBUTES_ENTITY		= 112,
	EDITOR_ATTRIBUTES_CAMERA		= 119,
	EDITOR_PHOTO_SCENE_PROPERTIES	= 126,
	
	EDITOR_CONTENT_BROWSER			= 113,
	EDITOR_MAP						= 114,
	EDITOR_TRANSFORM				= 115,
	EDITOR_EXTEND_SLOT				= 116,
	EDITOR_LAYER					= 117,
	EDITOR_CONTEXT_MENU				= 120,
	EDITOR_BUDGET					= 121,
	EDITOR_INTERACTION_VEHICLE		= 122,
	EDITOR_NOTIFICATION_EDITOR_ONLY	= 125,
	//								= 127
	
	MANUAL_CAMERA_MOVE				= 200,
	MANUAL_CAMERA_NIGHTMODE			= 201,
	MANUAL_CAMERA_ATTACH			= 202,
	MANUAL_CAMERA_FOCUS				= 203,
	MANUAL_CAMERA_SAVE				= 204,
	MANUAL_CAMERA_ZOOM				= 205,
	MANUAL_CAMERA_ADJUST_SPEED		= 206,
	MANUAL_CAMERA_ROLL				= 207,
	
	GAME_MODE_EDITOR_NO_GM			= 300,
	GAME_MODE_EDITOR_PLAYER_LIST	= 301,
	
	//~ Guide
	GUIDE_EDITOR_INTRO				= 400,
	GUIDE_EDITOR_ACTIONBAR			= 401,
	GUIDE_EDITOR_ENTITIES			= 402,
	GUIDE_EDITOR_EXITING			= 404,
	GUIDE_PHOTO_INTRO				= 405,
	
	CONFLICT_OVERVIEW				= 500,
	CONFLICT_SERVICE_DEPOTS			= 501,
	CONFLICT_BASE_SEIZED			= 502,
	CONFLICT_SUPPLY_RUNS			= 503,
	CONFLICT_TRANSPORT_REQUEST		= 504,
	CONFLICT_TRANSPORT_PICKUP		= 505,
	CONFLICT_VOLUNTEERING			= 506,
	CONFLICT_PRIMARY_OBJECTIVES		= 507,
	CONFLICT_RESPAWN				= 508,
	CONFLICT_MOBILE_HQ				= 509,
	CONFLICT_ELIMINATING_ENEMIES	= 510,
	CONFLICT_BASE_SEIZING			= 511,
	CONFLICT_TOWER_SEIZING			= 512,
	CONFLICT_BUILDING				= 513,
	CONFLICT_PROMOTIONS				= 514,
	CONFLICT_LOADOUTS				= 515,
	CONFLICT_SUPPLIES				= 516,
	CONFLICT_DEFENDING_BASES		= 517,
	CONFLICT_DRIVER					= 518,
	CONFLICT_VETERANCY				= 519,
	CONFLICT_LOSING					= 520,
	CONFLICT_NO_CONNECTION			= 521,
	CONFLICT_GROUP_ICONS			= 522,
	
	CONFLICT_COMMANDER_VOTING			= 524,
	CONFLICT_COMMANDER_ROLE				= 525,

	CONFLICT_COMMANDER_HQ_OPERATIONS	= 526,
	CONFLICT_COMMANDER_HQ_LOGISTICS		= 527,
	CONFLICT_COMMANDER_HQ_LOGISTICS_SUPPLY = 528,
	CONFLICT_COMMANDER_HQ_RADIO			= 529,

	CONFLICT_FOB_OBJECTIVE				= 530,
	CONFLICT_FOB_BUILDING				= 531,

	CONFLICT_GROUPS_INTRO				= 532,
	CONFLICT_GROUPS_ASSAULT				= 533,
	CONFLICT_GROUPS_MECHANIZED			= 534,
	CONFLICT_GROUPS_MACHINEGUN			= 535,
	CONFLICT_GROUPS_ANTITANK			= 536,

	CONFLICT_GROUPS_LOGISTIC			= 537,
	CONFLICT_GROUPS_TRANSPORT			= 538,
	CONFLICT_GROUPS_HELI_TRANSPORT		= 539,
	CONFLICT_GROUPS_ENGINEER			= 540,

	CONFLICT_GROUPS_SUPPORT				= 541,
	CONFLICT_GROUPS_RECON				= 542,
	CONFLICT_GROUPS_MORTAR				= 543,
	CONFLICT_GROUPS_MEDIC				= 544,
	CONFLICT_GROUPS_COMMANDER			= 545,
	
	CONFLICT_SUPPORT_REQUESTS_BASICS	= 546,
	CONFLICT_SUPPORT_REQUESTS_SQUAD_LEADER	= 547,
	CONFLICT_PICKUP_CREATION			= 548,
	CONFLICT_PICKUP_ASSIGNED			= 549,
	CONFLICT_REINFORCE_CREATION			= 550,
	CONFLICT_REINFORCE_ASSIGNMENT		= 551,
	CONFLICT_REARM_CREATION				= 552,
	CONFLICT_REARM_ASSIGNMENT			= 553,
	CONFLICT_REPAIR_CREATION			= 554,
	CONFLICT_REPAIR_ASSIGNMENT			= 555,
	CONFLICT_FIRE_SUPPORT_CREATION		= 556,
	CONFLICT_FIRE_SUPPORT_ASSIGNMENT	= 557,
	CONFLICT_SUPPORT_REQUESTS_ADVANCED2	= 558,
	CONFLICT_REGROUP_ASSIGNMENT			= 559,
	CONFLICT_RECON_REQUEST_CREATION		= 560,
	CONFLICT_BUILD_REQUEST_CREATION		= 561,
	CONFLICT_RESTOCK_ASSIGNMENT			= 562,
	CONFLICT_TASK_RELEVANCE				= 563,
	CONFLICT_SOURCE_BASE				= 564,
	CONFLICT_MSAR						= 565,
	CONFLICT_NIGHT_PLAY					= 566,
	
	CONFLICT_SQUAD_LEADER_BASIC			= 567,
	CONFLICT_SQUAD_LEADER_ADVANCED		= 568,
	CONFLICT_SQUAD_LEADER_COMMUNICATION	= 569,
	CONFLICT_SQUAD_LEADER_COHESION		= 570,
	CONFLICT_SQUAD_LEADER_RECON			= 571,
	CONFLICT_SQUAD_LEADER_SUPPLIES		= 572,
	CONFLICT_SQUAD_LEADER_LOGISTIC_FOB	= 573,
	CONFLICT_SQUAD_LEADER_SUPPORT		= 574,
	CONFLICT_OBJECTIVES_BASICS			= 575,
	CONFLICT_OBJECTIVES_ADVANCED		= 576,
	CONFLICT_OBJECTIVES_SQUAD_LEADER	= 577,
	CONFLICT_OBJECTIVES_ATTACK			= 578,
	CONFLICT_OBJECTIVES_SEIZE			= 579,
	CONFLICT_OBJECTIVES_HOLD			= 580,
	CONFLICT_OBJECTIVES_RECON			= 581,
	CONFLICT_OBJECTIVES_RESSUPLY		= 582,
	CONFLICT_OBJECTIVES_RALLY			= 583,
	CONFLICT_OBJECTIVES_CLEAR			= 584,
	CONFLICT_OBJECTIVES_ESTABLISH_BASE	= 585,
	CONFLICT_OBJECTIVES_DISMANTLE		= 586,
	                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    
	GAMEPLAY_RADIO_RESPAWN			= 600,
	GAMEPLAY_GROUPS					= 601,
	GAMEPLAY_WEAPON_INSPECTION		= 602,
	GAMEPLAY_VEHICLE_INVENTORY		= 603,
	
	GAMEPLAY_PERCEIVED_FACTION_CHANGED					= 604,
	GAMEPLAY_PERCEIVED_FACTION_CHANGED_PUNISH_WARCRIME	= 605,
	GAMEPLAY_PERCEIVED_FACTION_CHANGED_PUNISH_XP		= 606,
	
	GAMEPLAY_PERSONAL_BELONGINGS	= 607,
	
	FREE_ROAM_BUILDING_BASE_INTRO	= 700,
	FREE_ROAM_BUILDING_VEHICLE		= 701,
	FREE_ROAM_BUILDING_EXITING		= 702,
	
	EMOTES_INTRO					= 800,
}
