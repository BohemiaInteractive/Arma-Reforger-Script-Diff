/*!
Notification IDs used by SCR_NotificationsComponent.
*/
enum ENotification
{
	UNKNOWN = 0, //!< Default value to make sure this value is set.
	
	//PLAYER
	PLAYER_JOINED = 100, //!< A player joined - (param1 = PlayerID)
	PLAYER_LEFT = 101, //!< A player left - (param1 = PlayerID)
	PLAYER_KICKED = 102, //!< A player was kicked - (param1 = PlayerID, param2 = kickCauseGroup, param3 = kickCauseReason)
	PLAYER_BANNED = 103, //!< A player was banned - (param1 = PlayerID, param2 = kickCauseGroup, param3 = kickCauseReason, param4 = Duration)
	PLAYER_JOINED_FACTION = 104, ///A player joined a faction - (param1 = PlayerID, param2 = factionIndex)
	PLAYER_DIED = 105, //!< Player died - (param1 = PlayerID)
	PLAYER_KILLED_PLAYER = 106, //!< Player killed a player - (param1 = (killer)PlayerID, param2 = (Killed Player)PlayerID)
	AI_KILLED_PLAYER = 107, //!< AI killed a Player - (param1 = (killer)EditibleEntityID, param2 = (killed player)PlayerID)
	PLAYER_LOADOUT_SAVED = 108, //!< Player changed their loadout
	POSSESSED_AI_DIED = 110, ///> Shown to GM only when possessed NPC died
	POSSESSED_AI_KILLED_PLAYER = 111, ///> Shown to GM only when possessed NPC killed a player
	PLAYER_KILLED_POSSESSED_AI = 112, ///> Shown to GM only when player killed a possessed NPC 
	AI_KILLED_POSSESSED_AI = 113, ///> Shown to GM only when player killed a possessed NPC 
	POSSESSED_AI_KILLED_POSSESSED_AI = 114, ///> Shown to GM only when possessed NPC killed a player
	PLAYER_BANNED_NO_DURATION = 115, //!< A player was banned for the session - (param1 = PlayerID, param2 = kickCauseGroup, param3 = kickCauseReason)
	PLAYER_LOADOUT_NOT_SAVED = 116, //!< Player tried to save their loadout but it failed (Prob because loadout did not change)
	PLAYER_TELEPORTED_SELF = 117, //!< Player teleported to new location (param1 = PlayerID, param2 = distance)
	PLAYER_LOADOUT_ITEM_FAILED_NOT_FACTION = 118, //!< Player failed to save loadout because given item is not part of the player's faction (param1 = inventoryItemRplId)
	PLAYER_LOADOUT_ITEM_FAILED_NOT_IN_ARSENAL = 119, //!< Player failed to save loadout because given item is not found in the given arsenal inventory (param1 = inventoryItemRplId)
	PLAYER_LOADOUT_NOT_SAVED_INVALID_ITEMS = 120, //!< Loadout was not saved as one or more items were Invalid
	PLAYER_LOADOUT_NOT_SAVED_UNCHANGED = 121, //!< Player loadout was not saved as the loadout is unchanged
	PLAYER_LOADOUT_ITEM_FAILED_ITEM_BLACKLISTED = 122, //!< Player failed to save loadout because given item is blacklisted and cannot be saved (param1 = inventoryItemRplId)
	PLAYER_LOADOUT_CLEARED_BY_EDITOR = 123, //!< Player loadout was cleared as GM changed the blacklisted items
	PLAYER_LOADOUT_CLEARED = 124, //!< Player loadout was cleared without GM control
	PLAYER_LOADOUT_SAVED_SUPPLY_COST = 125, //!< Player changed their loadout and show the supply cost for that loadout in notification (param1 = SupplyCost)
	
	//~ Killed by editor
	PLAYER_KILLED_BY_EDITOR = 126, //!< Player killed by GM but no GM ID known (param1 = (Killed)PlayerID)
	PLAYER_KILLED_BY_EDITOR_PLAYER = 127, //!< Player killed by GM. GM ID is known - (param1 = (Killed)PlayerID, param2 = (killer)PlayerID)
	POSSESSED_AI_KILLED_BY_EDITOR = 128, //!< Possessed AI killed by GM but no GM ID known (param1 = (Killed)PlayerID)
	POSSESSED_AI_KILLED_BY_EDITOR_PLAYER = 129, //!< Possessed AI killed by GM. GM ID is known - (param1 = (Killed)PlayerID, param2 = (killer)PlayerID)
	
	//~ In AI in group notifications
	AI_IN_GROUP_DIED = 130, //!< AI in group has died. Unknown Killer - (param1 = AI RPL ID)
	PLAYER_KILLED_AI_IN_GROUP = 131, //!< AI in group has died. Unknown Killer - (param1 = (killer)PlayerID, param2 = (killed AI)AI RPL ID)
	AI_KILLED_AI_IN_GROUP = 132, //!< AI in group has died. Unknown Killer - (param1 = (killer)AI RPL ID, param2 = (killed AI)AI RPL ID)
	POSSESSED_AI_KILLED_AI_IN_GROUP = 133, //!< AI in group has died. Unknown Killer - (param1 = (killer)AI RPL ID, param2 = (killed AI)AI RPL ID)
	PLAYER_ON_BLOCKLIST_JOINED = 134, //!< Player who is on blocklist of local player has joined the server
	
	//PLAYER
	PLAYER_LOADOUT_NOT_SAVED_NOT_ENOUGH_MSA = 150, //!< Player loadout was not saved due to player not having enough Military Supply Allocation for loadout
	PLAYER_LOADOUT_NOT_SAVED_INSUFFICIENT_RANK = 151, //!< Player loadout was not saved as player has insufficient rank for one or more weapons
	
	//RIGHTS
	EDITOR_EDITOR_RIGHTS_ASSIGNED = 200, //!< Player got rights assigned - (param1 = GameMasterID, param2 = TargetPlayerID))
	EDITOR_EDITOR_RIGHTS_REMOVED = 201, //!< Player got rights revoked - (param1 = GameMasterID, param2 = TargetPlayerID)
	EDITOR_ADMIN_RIGHTS_ASSIGNED = 202, //!< Player got rights assigned - (param1 = GameMasterID, param2 = TargetPlayerID)
	EDITOR_ADMIN_RIGHTS_REMOVED = 203, //!< Player got rights revoked - (param1 = GameMasterID, param2 = TargetPlayerID)
	EDITOR_PHOTOMODE_RIGHTS_ASSIGNED = 204, //!< Player got rights assigned - (param1 = GameMasterID, param2 = TargetPlayerID)
	EDITOR_PHOTOMODE_RIGHTS_REMOVED = 205, //!< Player got rights revoked - (param1 = GameMasterID, param2 = TargetPlayerID)
	EDITOR_ENABLE_PROP_BUDGET = 206,
	EDITOR_DISABLE_PROP_BUDGET = 207,
	EDITOR_ENABLE_AI_BUDGET = 208,
	EDITOR_DISABLE_AI_BUDGET = 209,
	EDITOR_ENABLE_VEHICLE_BUDGET = 210,
	EDITOR_DISABLE_VEHICLE_BUDGET = 211,
	EDITOR_ENABLE_SYSTEM_BUDGET = 212,
	EDITOR_DISABLE_SYSTEM_BUDGET = 213,
	EDITOR_BUILDING_RIGHTS_ASSIGNED = 214,
	EDITOR_BUILDING_RIGHTS_REMOVED = 215,
	
	//PING
	EDITOR_PING_GM = 400, //!< GM pings - (param1 = GameMasterID, param2 = -1, param3 positionX, param4 positionY, param4 positionZ)
	EDITOR_PING_GM_TARGET_ENTITY = 401, //!< GM pings at target entity - (param1 = GameMasterID, param2 = EditableEntityID)
	EDITOR_PING_GM_TARGET_PLAYER = 402, //!< GM pings at target player - (param1 = GameMasterID, param2 = TargetPlayerID)
	EDITOR_PING_PLAYER = 403, //!< Player pings - (param1 = PlayerID, param2 = -1, param3 positionX, param4 positionY, param4 positionZ)
	EDITOR_PING_PLAYER_TARGET_ENTITY = 404, //!< Player pings at target entity - (param1 = PlayerID, param2 = EditableEntityID)
	EDITOR_PING_PLAYER_TARGET_PLAYER = 405, //!< Player pings at target player - (param1 = PlayerID, param2 = TargetPlayerID)
	EDITOR_PING_NO_GM_TO_PING = 406, //!< No GM to ping to notification	
	EDITOR_GM_ONLY_PING = 407, //!< GM only ping - (param1 = GameMasterID, param2 = -1, param3 positionX, param4 positionY, param4 positionZ)
	EDITOR_GM_ONLY_PING_TARGET_ENTITY = 408, //!< GM only ping at target entity - (param1 = GameMasterID, param2 = EditableEntityID)
	EDITOR_GM_ONLY_PING_TARGET_PLAYER = 409, //!< GM only ping at target player - (param1 = GameMasterID, param2 = TargetPlayerID)
	EDITOR_GM_ONLY_PING_LIMITED_RIGHTS = 410, //!< Tries to ping GM only ping but has limited editor rights
	
	//EDITOR NOTIFICATIONS
	EDITOR_CANNOT_OPEN = 500, //!< Opening the editor is disabled or no modes are available
	EDITOR_CANNOT_CLOSE = 501, //!< Closing the editor is disabled
	EDITOR_TRANSFORMING_LOST_ACCESS = 502, //!< Transformed entity is no longer available - (param1 = EditableEntityID)
	EDITOR_TRANSFORMING_INCORRECT_POSITION = 503, //!< Attempting to move entity to incorrect position
	EDITOR_TRANSFORMING_INCORRECT_TARGET = 504, //!< Attempting to move entity inside incompatible entity
	EDITOR_PLACING_BUDGET_MAX = 505,//!< Max budget reached
	EDITOR_PLACING_CANNOT_AS_PLAYER = 506, //!< Attempting to place non-character entity as a player
	EDITOR_TRANSFORMING_FAIL = 507, //!< General error when server fails to transform the entity
	EDITOR_FACTION_NO_SPAWNS = 508, //!< Called if all spawnpoints are removed
	//EDITOR_FACTION_NO_TASKS = 509, //!< Called if all objectives are removed
	EDITOR_SESSION_SAVE_SUCCESS = 510, //!< Editor session saved
	EDITOR_SESSION_SAVE_FAIL = 511, //!< Editor session save was not successful for whatever reason
	EDITOR_SESSION_LOAD_SUCCESS = 512, //!< Editor session loaded
	EDITOR_SESSION_LOAD_FAIL = 513, //!< Editor session load was not successful for whatever reason
	EDITOR_SAVED_CAMERA_POSITION = 514, //!< When GM saved camera position (param1 = camera int)
	EDITOR_LOADED_CAMERA_POSITION = 515, //!< When GM loaded camera position (param1 = camera int)
	EDITOR_TASK_PLACED = 516,  //!< Objective was placed (param1 = Task Replication ID, FactionIndex)
	EDITOR_TASK_COMPLETED = 517, //!< Objective was Completed (param1 = Task Replication ID, FactionIndex)
	EDITOR_TASK_FAILED = 518, //!< Objective was Failed (param1 = Task Replication ID, FactionIndex)
	EDITOR_TASK_DELETED = 519, //!< Objective was Deleted by GM (param1 = Task Replication ID, FactionIndex)
	EDITOR_TASK_CANCELED = 520, //!< Objective was Canceled (param1 = Task Replication ID, FactionIndex)
	EDITOR_GM_TELEPORTED_PLAYER = 521, //!< When the player gets teleported a notification will be send to that player (param1 = GM teleported the player, player that was teleported (for GM to go to position))
	EDITOR_PLACING_BUDGET_MAX_FOR_VEHICLE_OCCUPANTS = 522, //!< Max budget reached when trying to place occupants in vehicle
	EDITOR_PLACING_OUT_OF_CAMPAIGN_BUILDING_ZONE = 523, //!< Trying to place a composition outside of building perimeter
	EDITOR_PLACING_NO_MORE_INSTANCES = 524, //!< Trying to place a composition (service) which already exists in target base.
	EDITOR_PLACING_NO_ENOUGH_SUPPLIES = 525, //!< Not enough supplies to build a composition.
	EDITOR_PLACING_BLOCKED = 526, //!< Area is blocked by another entity.
	EDITOR_PLACING_RANK_CORPORAL_NEEDED = 527, //!< Rank corporal is needed to place this composition.
	EDITOR_PLACING_RANK_SERGEANT_NEEDED = 528, //!< Rank sergeant is needed to place this composition.
	EDITOR_SERVICE_BUILD = 529, //!< Shows the name of the player, type of service and base where the service was built.
	EDITOR_SERVICE_DISASSEMBLED = 530, //!< Shows the name of the player, type of service and base where the service was disassembled.
	EDITOR_PREVIEWING_IN_NIGHTMODE = 531, //!< Local player is previewing datetime or weather in night mode. This means night mode is disabled until previewing is done
	EDITOR_COMPOSITION_UNDER_CONSTRUCTION = 532, //!< Composition cannot be moved or deleted because the construction works has begun.
	EDITOR_PLACING_NO_MORE_COMPOSITIONS_AT_BASE = 533, //!< Player cannot place more compositions at the base
	EDITOR_COOLDOWN = 534, //!< Cooldown for requesting another entity is running.
	EDITOR_AILIMIT = 535, //!< Max limit of spawned AI at base (Free Roam Building) has been reached.
	EDITOR_ENEMY_IN_AREA = 536, //!< The enemy was detected in Free Roam Building mode area, mode is terminated.
	EDITOR_PLACING_RANK_TOO_LOW = 537, //!< Rank is too low to be able to place this composition.
	EDITOR_PLACING_BUDGET_ESTABLISH_BASE = 538, //!< New Command post can only be built within range of the establish base task
	EDITOR_PLACING_ESTABLISH_BASE_LIMIT_REACHED = 539, //!< The faction reached a limit of Established bases and can not build a new Command post
	EDITOR_PHOTOMODE_IS_LOCKED = 540, //!< Player in multiplayer is not allowed to use Armavision

	//AI
	//EDITOR_AI_GROUP_ELIMINATED = 600, //!< An AI group was eliminated - (param1 = EditableEntityID)
	
	//GLOBAL ATTRIBUTES
	EDITOR_ATTRIBUTES_FACTION_LOADOUT_CHANGED = 700, //!< GM changes faction loadout - (For now disabled as misses specific NotificationDisplay class)
	EDITOR_ATTRIBUTES_RESPAWN_ENABLED = 701, //!< GM enabled respawn - (param1 = GameMasterID)
	EDITOR_ATTRIBUTES_RESPAWN_DISABLED = 702, //!< GM disabled respawn - (param1 = GameMasterID)
	EDITOR_ATTRIBUTES_DATE_CHANGED = 703, //!< GM change the game date - (param1 = GM, param2 = day, param3 = month, param4 = year)
	EDITOR_ATTRIBUTES_TIME_CHANGED = 704, //!< GM change the game time  (param1 = GM, param2 = hours1:, param3 = hours2, param4 = minute1, param5 = minute2
	EDITOR_ATTRIBUTES_WEATHER_CHANGED = 705, //!< GM change the game weather
	EDITOR_ATTRIBUTES_GM_BUDGET_CHANGED = 706, //!< GM change other GM editor budget - (param1 = GameMasterID)
	EDITOR_ATTRIBUTES_FACTION_CHANGED = 707, //!< A GM enabling/disabled a faction - (param1: GM)
	EDITOR_ATTRIBUTES_WIND_CHANGED = 709, //!< GM changed wind Speed and/or Direction - (param1: GM)
	EDITOR_ATTRIBUTES_WIND_DEFAULT = 710, //!< GM set wind speed to be default instead of overridden - (param1: GM)
	EDITOR_ATTRIBUTES_WEATHER_AUTO = 711, //!< Weather set to auto - (param1: GM)
	EDITOR_ATTRIBUTES_DAY_DURATION_CHANGED = 712, //!< GM changed day duration - (param1 = GM, param2 = NewDuration)
	EDITOR_ATTRIBUTES_RESPAWN_TIME_CHANGED = 713, //!< GM changed respawn time - (param1 = GM id, param2 = NewTime)
	EDITOR_ATTRIBUTES_DAY_ADVANCE_ENABLED = 714, //!< If GM enables time advancement - (param1: GM)
	EDITOR_ATTRIBUTES_DAY_ADVANCE_DISABLED = 715,//!< If GM Disables time advancement - (param1: GM)
	EDITOR_ATTRIBUTES_ENABLE_RESPAWN_ON_PLAYER = 719, //!< Called when GM enables spawning on Radio operators (param1 = GM)
	EDITOR_ATTRIBUTES_DISABLE_RESPAWN_ON_PLAYER = 720, //!< Called when GM disables spawning on Radio operators (param1 = GM)
	EDITOR_ATTRIBUTES_FACTION_CHANGED_NO_GM = 721, //!< Called when GM places a spawnpoint but the faction was not enabled
	EDITOR_ATTRIBUTES_ENABLED_AMBIENT_MUSIC = 724, //!< When the GM enables server wide ambient music (param1 = GM that enables it)
	EDITOR_ATTRIBUTES_DISABLED_AMBIENT_MUSIC = 725, //!< When the GM disables server wide ambient music (param1 = GM that disables it)	EDITOR_GM_ENABLED_AMBIENT_MUSIC = 522, //!< When the GM enables server wide ambient music (param1 = GM that enables it)
	//NOT IN GAME \/
	EDITOR_ATTRIBUTES_SERVER_NAME_CHANGED = 716, //!< GM change the server name
	EDITOR_ATTRIBUTES_SERVER_PASSWORD_CHANGED = 717, //!< GM change the server password
	EDITOR_ATTRIBUTES_SERVER_PLAYER_COUNT_CHANGED = 718, //!< GM change the server max player count - (param1 = newPlayerCount)
	EDITOR_ATTRIBUTES_UNMUTE_MUSIC = 726, //!< When the GM enables server wide music (param1 = GM that enables it) (Not in game)
	EDITOR_ATTRIBUTES_MUTE_MUSIC = 727, //!< When the GM disables server wide music (param1 = GM that disables it) (Not in game)
	//NOT IN GAME /\
	EDITOR_CHANGED_KILLFEED_TYPE = 728, //!< Changed the killfeed type (param1 = GM who changed setting, param2 = The new KillfeedType)
	EDITOR_CHANGED_KILLFEED_RECEIVE_TYPE = 729, //!< Changed killfeed receive type param1 = GM who changed setting, param2 = The new KillfeedReceiveType)
	EDITOR_ATTRIBUTES_UNCONSCIOUSNESS_ENABLED = 730, //!< GM enabled unconsciousness - (param1 = GameMasterID)
	EDITOR_ATTRIBUTES_UNCONSCIOUSNESS_DISABLED = 731, //!< GM disabled unconsciousness - (param1 = GameMasterID)
	EDITOR_FACTION_SET_FRIENDLY_TO = 735, //!< GM set faction relations to friendly - (param1 = GameMasterID, param2 = FactionA, param3 = FactionB)
	EDITOR_FACTION_SET_HOSTILE_TO = 736, //!< GM set faction relations to hostile - (param1 = GameMasterID, param2 = FactionA, param3 = FactionB)
	EDITOR_FACTION_SET_FRIENDLY_TO_SELF = 737, //!< GM set faction relations to friendly to itself - (param1 = GameMasterID, param2 = Faction)
	EDITOR_FACTION_SET_HOSTILE_TO_SELF = 738, //!< GM set faction relations to hostile to itself - (param1 = GameMasterID, param2 = Faction)
	EDITOR_CHANGED_FUEL_CONSUMPTION_SCALE = 739, //!< GM changed the fuel consumption scale of vehicles (param1 = GM, param2 = new Fuel Consumption Scale)
	EDITOR_GLOBAL_NIGHTMODE_ENABLED = 740, //!< GM changed the nightmode to true (param1 = GM)
	EDITOR_GLOBAL_NIGHTMODE_DISABLED = 741, //!< GM changed the nightmode to false (param1 = GM)
	EDITOR_CHANGED_LOADOUT_SAVE_BLACKLIST = 742, //!< GM changed which items players can save in their loadout (param1 = GM)
	
	EDITOR_ATTRIBUTES_ENABLE_RESPAWN_ON_RADIO_VEHICLE = 743, //!< Called when GM enables spawning on Radio vehicles (param1 = GM)
	EDITOR_ATTRIBUTES_DISABLE_RESPAWN_ON_RADIO_VEHICLE = 744, //!< Called when GM disables spawning on Radio vehicles (param1 = GM)
	EDITOR_ATTRIBUTES_ENABLE_DEPLOYABLE_RADIO_SPAWNPOINT = 745, //!< Called when GM enables deploying radio back pack spawn points for players (param1 = GM)
	EDITOR_ATTRIBUTES_DISABLE_DEPLOYABLE_RADIO_SPAWNPOINT = 746, //!< Called when GM disables deploying radio back pack spawn points for players (param1 = GM)
	
	EDITOR_ATTRIBUTES_ENABLE_GLOBAL_SUPPLY_USAGE = 747, //!< Called when GM enables Global supplies usage (param1 = GM)
	EDITOR_ATTRIBUTES_DISABLE_GLOBAL_SUPPLY_USAGE = 748, //!< Called when GM disables Global supplies usage (param1 = GM)
	EDITOR_ATTRIBUTES_CHANGED_SPAWN_SUPPLYCOST_MULTIPLIER = 749, //!< Called when GM changes the supply cost multiplier for loadouts (param1 = GM, param2 = SupplyCost multiplier)
	EDITOR_ATTRIBUTES_CHANGED_ARSENAL_GAMEMODE_TYPE = 750, //!< Called when GM changes the Arsenal game mode type (param1 = GM, param2 = new gamemode type enum)
	EDITOR_ATTRIBUTES_CHANGED_ARSENAL_TYPE_ENABLED = 751, //!< Called when GM changes the Arsenal enabled type (param1 = GM)
	
	EDITOR_ATTRIBUTES_CHANGED_DEPLOYABLE_RADIO_SPAWNPOINT_BUDGET_NONE = 752, //!< Called when GM changes deployable radio budget type to NONE (param1 = GM)
	EDITOR_ATTRIBUTES_CHANGED_DEPLOYABLE_RADIO_SPAWNPOINT_BUDGET_SUPPLIES = 753, //!< Called when GM changes deployable radio budget type to SUPPLIES (param1 = GM)
	EDITOR_ATTRIBUTES_CHANGED_DEPLOYABLE_RADIO_SPAWNPOINT_BUDGET_TICKETS = 754, //!< Called when GM changes deployable radio budget type to TICKETS (param1 = GM)
	EDITOR_ATTRIBUTES_CHANGED_DEPLOYABLE_RADIO_SPAWNPOINT_TICKET_AMOUNT = 755, //!< Called when GM changes amount of respawn tickets available to deployed radios (param1 = GM, param2 = ticket amount)
	
	EDITOR_REFUND_ENTITY_AT_DEPOTS_ENABLED = 756, //!< Called when the GM enables vehicle refunding (param1 = GM)
	EDITOR_REFUND_ENTITY_AT_DEPOTS_DISABLED = 757, //!< Called when the GM disables vehicle refunding (param1 = GM)
	
	EDITOR_TEAMKILL_PUNISHMENT_ENABLED = 758, //!< Called when the GM enables teamkill punishment (param1 = GM)
	EDITOR_TEAMKILL_PUNISHMENT_DISABLED = 759, //!< Called when the GM disables teamkill punishment(param1 = GM)
	
	EDITOR_CHANGED_FRIENDLY_FIRE_KILLFEED_TYPE = 760, //!< Changed friendly killfeed receive type param1 = GM who changed setting, param2 = The new FriendlyFireKillfeedReceiveType)
	
	EDITOR_PERCEIVED_FACTION_PUNISHMENT_KILLING_SET = 761, //!< Called when the GM changes the punishment when killing a hostile and the players perceived faction is not the players default faction (param1 = GM)
	
	EDITOR_PERCEIVED_FACTION_TYPE_DISABLED = 762, //!< Called when the perceived faction logic is disabled (param1 = GM)
	EDITOR_PERCEIVED_FACTION_TYPE_HIGHEST_VALUE = 763, //!< Called when the perceived faction logic is set to Highest value type (param1 = GM)
	EDITOR_PERCEIVED_FACTION_TYPE_FULL_OUTFIT = 764, //!< Called when the perceived faction logic is set to full outfit type (param1 = GM)
	
	EDITOR_NAMETAG_RELATION_FILTER_CHANGED = 765, //!< (param1 = GM) Called when nametag relation filters change
	EDITOR_NAMETAG_CONTROLLER_FILTER_CHANGED = 766, //!< (param1 = GM) Called when nametag controller filters change
	
	EDITOR_VEHICLE_SALVAGE_ENABLED = 767, //!< Called when the GM enables vehicle salvage
	EDITOR_VEHICLE_SALVAGE_DISABLED = 768, //!< Called when the GM disables vehicle salvage

	EDITOR_MORTAR_BALLISTIC_DATA_VISIBILITY_ENABLED = 769, //!< Called when the GM enables the in-scope ui element of mrotars that shows ballistic data for the shells
	EDITOR_MORTAR_BALLISTIC_DATA_VISIBILITY_DISABLED = 770, //!< Called when the GM disables the in-scope ui element of mrotars that shows ballistic data for the shells
	
	//GM
	EDITOR_PLAYER_BECAME_GM = 800, //!<Player become GM - (param1 = PlayerID)
	EDITOR_GM_LEFT = 801, //!<Player become GM - (param1 = PlayerID)
	EDITOR_PLAYER_NO_LONGER_GM = 802, //!<Player no longer GM - (param1 = PlayerID)
	
	//CAMPAIGN
	BUILD_COST_MULTI_CHANGED = 900, //!< Cost multiplier to build compositions by players (not editor) 
	BUILD_REFUND_MULTI_CHANGED = 901,//!< Refund multiplier for compositions by players (not editor)
	SUPPLY_TRUCK_LOADING_PLAYER = 902, //!< Shows to truck occupants when vehicle is being loaded - (param1 = player id, param2 = amount)
	SUPPLY_TRUCK_UNLOADING_PLAYER = 903, //!< Shows to truck occupants when vehicle is being unloaded - (param1 = player id, param2 = amount)
	SUPPLY_TRUCK_LOADING_PLAYER_STOPPED = 904, //!< Shows to truck occupants when unloading player cancels interaction - (param1 = player id)
	SUPPLY_TRUCK_UNLOADING_PLAYER_STOPPED = 905, //!< Shows to truck occupants when unloading player cancels interaction  - (param1 = player id)
	SUPPLY_TRUCK_LOADING_PLAYER_FINISHED = 906, //!< Shows to truck occupants when loading player finishes interaction - (param1 = player id)
	SUPPLY_TRUCK_UNLOADING_PLAYER_FINISHED = 907, //!< Shows to truck occupants when unloading player finishes interaction - (param1 = player id)
	CAMPAIGN_TRANSPORT_GROUP_LOST = 908, //!< Shows to Commander when transport group is deleted (param1 = Transport group replication ID)

	//VOTING
	VOTING_EDITOR_IN_START = 1000, //!< Voting started to become GM (param1 = player id)
	VOTING_EDITOR_IN_SUCCEED = 1001, //!< Voting succeeded to become GM (param1 = player id)
	VOTING_EDITOR_IN_FAIL = 1002, //!< Voting failed to become GM (param1 = player id)
	VOTING_EDITOR_IN_VOTE_CASTED = 1003, //!< Voting failed to become GM (param1 = player id)
	VOTING_EDITOR_IN_VOTE_ABSTAINED = 1004, //!< Voting cast locally by player to make other GM (param1 = player id)
	VOTING_EDITOR_OUT_START = 1010,//!< Voting abstained locally by player to make other GM (param1 = player id)
	VOTING_EDITOR_OUT_SUCCEED = 1011, //!< Voting succeeded to remove GM (param1 = player id)
	VOTING_EDITOR_OUT_FAIL = 1012, //!< Voting failed to remove GM (param1 = player id)
	VOTING_EDITOR_OUT_VOTE_CASTED = 1013, //!< Voting cast locally by player to remove other GM (param1 = player id)
	VOTING_EDITOR_OUT_VOTE_ABSTAINED = 1014, //!< Voting abstained locally by player to remove other GM (param1 = player id)
	VOTING_KICK_START = 1020, //!< Voting started to kick player (param1 = player id)
	VOTING_KICK_SUCCEED = 1021, //!< Voting succeeded to kick player (param1 = player id)
	VOTING_KICK_FAIL = 1022, //!< Voting failed to kick player (param1 = player id)
	VOTING_KICK_VOTE_CASTED = 1023, //!< Voting Cast locally to kick player (param1 = player id)
	VOTING_KICK_VOTE_ABSTAINED = 1024, //!< Voting Abstained localy to kick player (param1 = player id)
	VOTING_AUTO_KICK_START = 1025, //!< Voting started to Auto kick player (param1 = player id)
	VOTING_AUTO_BAN_LIGHT_START = 1030, //!< Voting started to auto light ban player (param1 = player id)
	VOTING_AUTO_BAN_HEAVY_START = 1031, //!< Voting started to auto heavy ban player (param1 = player id)
	VOTING_BAN_SUCCEED = 1032, //!< Voting succeeded to auto light/heavy ban player (param1 = player id)
	VOTING_BAN_FAIL = 1033, //!< Voting failed to auto light/heavy ban player (param1 = player id)
	VOTING_BAN_VOTE_CASTED = 1034, //!< Voting Cast locally to auto light/heavy ban player (param1 = player id)
	VOTING_BAN_VOTE_ABSTAINED = 1035, //!< Voting Abstained localy to auto light/heavy ban player (param1 = player id)
	VOTING_KICK_SUCCEED_SUBJECT_LEFT = 1036, //!< Voting notification shown when vote to kick ends when kicked player left the server
	VOTING_COMMANDER_DONE = 1037, ///< Voting succeeded for a new faction commander (param1 = player id)
	VOTING_COMMANDER_WITHDRAW = 1038, ///< Faction commander left their role (param1 = player id)
	
	//GROUPS 
	GROUPS_PLAYER_JOINED = 1101, //!< Player joined the group (param1 = player id)
	GROUPS_PLAYER_LEFT = 1102, //!< Player has left the group (param1 = player id)
	GROUPS_PLAYER_PROMOTED_LEADER = 1103, //!< Player has been promoted to group leader (param1 = player id)
	GROUPS_AI_JOINED = 1104, //!< AI joined the group (param1 = entityID)
	GROUPS_AI_LEFT = 1105, //!< AI has left the group (param1 = entityID)
	GROUPS_PLAYER_IS_NOT_LEADER = 1106, //!< Player cant choose group flag, if he is not the leader
	GROUPS_NO_FLAGS = 1107, //!< There are no groupflags in groups manager
	GROUPS_REQUEST_JOIN_PRIVATE_GROUP = 1108, //!< Player want to join the private group
	GROUPS_REQUEST_ACCEPTED = 1109, //!< player was accepted to private group
	GROUPS_REQUEST_DENIED = 1110, //!< player was accepted to private group
	GROUPS_REQUEST_CANCELLED = 1111, //!< players join request was canceled by group leader change
	GROUPS_INVITE_CANCELLED_INSUFFICIENT_RANK = 1112, //!< Player invite was not sent due to invitee's insufficient rank to join group
	
	//OTHERS
	ACTION_ON_COOLDOWN = 1200, //!< Action is on cooldown and cannot be executed
	AREA_SEIZING_DONE_FRIENDLIES = 1201, //!< Area captured by friendlies
	AREA_SEIZING_DONE_ENEMIES = 1202, //!< Area captured by enemies
	BASE_SEIZING_DONE_FRIENDLIES = 1203, //!< Base captured by friendlies
	BASE_SEIZING_DONE_ENEMIES = 1204, //!< Base captured by enemies
	TELEPORTED_PLAYER_BLOCKING_SPAWNER = 1205, //!< Player was teleported as it was blocking a spawner
	GROUPS_REQUEST_SENT = 1206, //!< player sent join request (param1 = group id)
	GROUP_RADIO_DISMANTLED_BY_FRIENDLY = 1207, //!< friendly player dismantled a radio beacon
	
	//COMMANDING
	COMMANDING_NO_RIGHTS = 1300,
	
	SUPPORTSTATION_HEALED_BY_OTHER_UPDATE = 1400, //!< Player is being healed by another Character (param1 = Healer entity RplID, param2: hitzoneGroupId, param2: health scaled)
	SUPPORTSTATION_HEALED_BY_OTHER_DONE_NOT_FULL = 1401,//!< Player is being healed by another Character, healing is done but support station cannot heal more (param1 = Healer entity RplID, param2: hitzoneGroupId, param2: health scaled)
	SUPPORTSTATION_HEALED_BY_OTHER_DONE = 1458, //!< Player has fully healed an hitzone group of a Character (param1 = Healer entity RplID, param2: hitzoneGroupId)

	SUPPORTSTATION_FIRE_EXTINGUISHED_VEHICLE_BY_OTHER_UPDATE = 1402, //!< Character is extinguising a fire on a vehicle you are in. The fire is still there (param1 = Repairer entity RplID)
	SUPPORTSTATION_FIRE_EXTINGUISHED_VEHICLE_BY_OTHER_DONE = 1403, //!< Character is extinguising a fire on a vehicle you are in. The fire is gone (param1 = Repairer entity RplID)
	
	SUPPORTSTATION_REPAIRED_BY_OTHER_UPDATE = 1404, //!< Vehicle player is in is being repaired by another Character. (param1 = Healer entity RplID, param2: hitzoneGroupId, param2: health scaled)
	SUPPORTSTATION_REPAIRED_BY_OTHER_DONE_NOT_FULL = 1405, //!< Vehicle player is in is being repaired by another Character. Repair of hitzone group is done but hitzone group is not fully repaired (param1 = Repairer entity RplID, param2: hitzoneGroupId)
	SUPPORTSTATION_REPAIRED_BY_OTHER_DONE = 1406, //!< Vehicle player is in is being repaired by another Character. Repair of hitzone group is done (param1 = Repairer entity RplID, param2: hitzoneGroupId)
	
	SUPPORTSTATION_REFUELED_BY_OTHER_UPDATE = 1407, //!< Vehicle player is in is being refueled by another Character. (param1 = Healer entity RplID, param2: fuelpercentage)
	SUPPORTSTATION_REFUELED_BY_OTHER_TANK_FULL = 1408,  //!< Vehicle player is in is being refueled by another Character. The fuel tank that was being refueled is full but not all fuel tanks are full (param1 = Healer entity RplID)
	SUPPORTSTATION_REFUELED_BY_OTHER_DONE = 1409, //!< Vehicle player is in is being refueled by another Character. All fuel tanks are full (param1 = Healer entity RplID)
	
	SUPPORTSTATION_RESUPPLIED_BY_OTHER_AMMO = 1410, //!< Player held weapon ammo is being resupplied by another character (param1 = Resupplier entity RplID)
	SUPPORTSTATION_RESUPPLIED_BY_OTHER_UGL = 1411, //!< Player held weapon UGL ammo is being resupplied by another character (param1 = Resupplier entity RplID)
	SUPPORTSTATION_RESUPPLIED_BY_OTHER_GRENADE = 1412, //!< Player grenades are being resupplied by another character (param1 = Resupplier entity RplID)
	SUPPORTSTATION_RESUPPLIED_BY_OTHER_BANDAGE = 1413, //!< Player bandages are being resupplied by another character (param1 = Resupplier entity RplID)
	SUPPORTSTATION_RESUPPLIED_BY_OTHER_MORPHINE = 1414, //!< Player morphine are being resupplied by another character (param1 = Resupplier entity RplID)
	
	CHECK_FUEL = 1415, //!< Checks fuel of vehicle (param1 = Fuel percentage)
	
	//FAST TRAVEL
	FASTTRAVEL_AVAILABLE = 1500,
	FASTTRAVEL_UNAVAILABLE = 1501,
	FASTTRAVEL_PLAYER_LOCATION_WRONG = 1502,
	FASTTRAVEL_PLAYER_LOCATION_CANCELLED = 1503,
	FASTTRAVEL_DONE = 1504,
	FASTTRAVEL_ENEMIES_NEARBY = 1505,

	//DEPLOYABLE INVENTORY ITEMS
	DEPLOYABLE_SPAWNPOINTS_NEARBY_SPAWNPOINT = 1600,
	DEPLOYABLE_SPAWNPOINTS_NEARBY_PLAYERS = 1601,
	DEPLOYABLE_SPAWNPOINTS_DEPLOYED_SPAWNPOINT_LIMIT = 1602,
	DEPLOYABLE_SPAWNPOINTS_DISPLAY_RESPAWN_COUNT = 1603,
	DEPLOYABLE_SPAWNPOINTS_DISPLAY_GROUP = 1604,
	DEPLOYABLE_SPAWNPOINTS_NEARBY_BASE = 1605,
	DEPLOYABLE_SPAWNPOINTS_NEARBY_HQ = 1606,
	DEPLOYABLE_SPAWNPOINTS_ZONE_ENTERED = 1607,
	DEPLOYABLE_SPAWNPOINTS_ZONE_EXITED = 1608,
	DEPLOYABLE_SPAWNPOINTS_LOADOUTS_ALLOWED = 1609,
	DEPLOYABLE_SPAWNPOINTS_LOADOUTS_BANNED = 1610,
	
	// GM SAVES
	EDITOR_SAVE_PUBLISH_SUCCESS = 1700,
	EDITOR_SAVE_PUBLISH_FAIL = 1701,

	//PLACEABLE INVENTORY ITEMS
	PLACEABLE_ITEM_CANT_PLACE_GENERIC = 1800,
	PLACEABLE_ITEM_CANT_PLACE_SURFACE_NO_LONGER_THERE = 1801,
	PLACEABLE_ITEM_CANT_PLACE_DIFFERENT_SURFACE = 1802,
	PLACEABLE_ITEM_CANT_PLACE_DISTANCE = 1803,
	PLACEABLE_ITEM_CANT_PLACE_TOO_STEEP = 1804,
	PLACEABLE_ITEM_CANT_PLACE_NOT_ENOUGH_SPACE = 1805,

	//INVENTORY ITEMS
	PLAYER_HAND_ITEM_DROPPED = 1900, //!< Player was forced to drop hand slot item
	PLAYER_HAND_ITEM_PUT_IN_INVENTORY = 1901, //!< Player was forced to put hand item in inventory

	//GROUP TASK
	GROUP_TASK_GROUP_ASSIGNED_TO_SAME_TASK = 2200,
	GROUP_TASK_GROUP_ASSIGNED_TO_SAME_REQUEST = 2201,
	GROUP_TASK_GROUP_ASSIGNED_TO_YOUR_REQUEST = 2202,
	GROUP_TASK_COMPLETED = 2203,
	GROUP_TASK_CANCELED = 2204,
	GROUP_TASK_FAILED = 2205,
	GROUP_TASK_REQUEST_COMPLETED = 2206,
	GROUP_TASK_REQUEST_CANCELED = 2207,
	GROUP_TASK_REQUEST_FAILED = 2208,
	GROUP_TASK_YOUR_REQUEST_COMPLETED = 2209,
	GROUP_TASK_YOUR_REQUEST_CANCELED = 2210,
	GROUP_TASK_YOUR_REQUEST_FAILED = 2211,
	GROUP_TASK_CREATED = 2212,
	GROUP_TASK_REINFORCE_ENTERED_AREA = 2213,
	GROUP_TASK_REPAIR_ENTERED_AREA = 2214,
	GROUP_TASK_REPAIR_LEAVED_AREA = 2215,
	GROUP_TASK_REPAIR_TRUCK_IS_NOT_IN_RANGE = 2216,
	GROUP_TASK_ESTABLISH_BASE_INVALID_AREA = 2217,
	GROUP_TASK_ESTABLISH_BASE_ENABLED = 2218,
	GROUP_TASK_ESTABLISH_BASE_DISABLED = 2219,
	GROUP_TASK_ESTABLISH_BASE_OUTSIDE_RADIO_RANGE = 2220,
	GROUP_TASK_ESTABLISH_BASE_TOO_CLOSE_ANOTHER_BASE = 2221,
	GROUP_TASK_ESTABLISH_BASE_TOO_CLOSE_ANOTHER_OBJECTIVE = 2222,
	GROUP_TASK_ESTABLISH_BASE_LIMIT_REACHED = 2223,
};
