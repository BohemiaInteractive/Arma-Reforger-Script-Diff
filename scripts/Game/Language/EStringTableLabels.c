/*!
Localization key labels.

Assign specific value to each entry!
Otherwise it may be shifted when new entries are added in front of it,
resulting in unintended label change in existing keys.
*/
enum EStringTableLabels
{
	UI,
	CHARACTERS,
	WEAPONS,
	VEHICLES,
	DIALOGUE,
	
	//Releasables
	PATCH_0_9_6 = 96, ///< Features intended for 0.9.6 patch
	PATCH_1_2_1 = 97,
	
	EDITOR = 200, ///< In-game editor
	EDITOR_MODE_EDIT = 201, ///< Edit mode
	EDITOR_MODE_STRATEGY = 202, ///< Strategy mode
	EDITOR_MODE_PHOTO = 203, ///< Photo mode
	EDITOR_MODE_ADMIN = 204, ///< Admin mode
	EDITOR_MODE_SPECTATE = 205, ///< Spectate mode
	EDITOR_ATTRIBUTE = 206, ///< Attributes
	EDITOR_CONTEXT_ACTION = 207, ///< Context actions
	EDITOR_OVERLAY = 208, ///< GUI overlay
	EDITOR_FILTER = 209, ///< Photo mode post-process filter
	EDITOR_TOOLBAR = 210, ///< Editor Toolbar
	EDITOR_TOOLTIP_DETAIL = 211, ///< Editor Tooltip detail
	EDITOR_CONTENT_BROWSER = 212, ///< Editor Content Browser
	EDITOR_CONTENT_BROWSER_FILTER = 213, ///< Editor Content Browser Filter
	
	EDITABLE_ENTITY = 300, ///< Editable Entities
	
	CONTROLS_HINTS = 400, // Available actions display aka controls hints
	USER_ACTIONS = 401, // Interaction handler, action menu
	QUICK_MENU = 402, // Quick menu - gadget & VON selection
	VON = 403, // Onscreen visualization for outgoing & incomming VON transmission
	CHAT = 404, // Chat tags, labels, names	
	NOTIFICATION = 405, //Notification
	FIELD_MANUAL = 406, ///< Field manual entries
	HINT = 407, ///< Hints
	VOTING = 408, ///< Voting
	KEYBIND = 409, ///< ActionManager keybinds

	RANKS = 500, // Military ranks
	
	CAMPAIGN = 600,	// Campaign-specific
	CAMPAIGN_TASKS = 601,	// Tasks (objectives)
	
	ROLES = 700, // Soldier roles
	GROUPS = 800, // Military groups
	
	ITEMS = 900, // Iventory items
	AMMUNITION = 1000, // All kinds of ammunition
	MAGAZINES = 1100, // Magazines, clips, belts - anything that can be loaded into weapon
	AMMOBOXES = 1150, //Ammoboxes
	GEAR = 1200, // Personal gear - uniforms, backpacks, helmets, vests, load-bearing systems and their individual pouches

	INVENTORY = 1300, // Inventory texts - UI and interactions
	
	//Units and generic
	COMPASS = 1400, ///< Compass Directions
	COMPASS_SHORT = 1401, ///< Compass Directions (Abbreviated)
	DATE = 1402, ///< Date values
	VALUE_UNIT = 1403, ///< Units for Values (Eg: %, Seconds, Meters)
	VALUE_UNIT_SHORT = 1404, ///< Units for Values short (Eg: s (Seconds), m/s (Meters per second))
	TIME_OF_DAY = 1405, ///< Dawn, Dusk, Midnight SetCursor
	WEATHER = 1406, ///< Clear, Cloudy, Rainy etc
	BLOODTYPE = 1407, ///<Blood type A+ AB- ect

	//Map
	MAP_SETTLEMENTS = 2000, // Settlement names on map (Cities, town, villages)
	MAP_HYDRONYMS = 2001,
	MAP_AGRONYMS = 2002,
	MAP_ORONYMS = 2003,
	MAP_DRYMONYMS = 2004,
	MAP_URBANONYMS = 2005,
	MAP_TOPONYMS = 2006,

	//Dialog
	DIALOGUE_PROTOCOL = 2100,
	DIALOGUE_CONFLICT = 2101,
	
	END_SCREEN = 2200,
	
	// Main menu 
	MAIN_MENU = 2300,
	SERVER_BROWSER = 2301,
	WORKSHOP = 2302,
	SERVER_HOSTING = 2303,
};