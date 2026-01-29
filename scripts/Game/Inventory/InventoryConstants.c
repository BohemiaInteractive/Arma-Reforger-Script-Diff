enum ESlotSize
{
	SLOT_1x1 = 1,
	SLOT_2x1,
	SLOT_2x2,
	SLOT_3x3
};

enum SCR_EQuickSlotSize : ESlotSize
{
	DEFAULT = 0,
};

enum ESlotID
{
	SLOT_ANY 			= -1,
	SLOT_BACKPACK		= 0,
	SLOT_LBS_BUTTPACK,
	SLOT_VEST,
	SLOT_WEAPONS_STORAGE,
	SLOT_GADGETS_STORAGE,
	SLOT_LOADOUT_STORAGE,
	SLOT_SPECIALIST_STORAGE,
	SLOT_HAND,
	SLOT_LOOT_STORAGE = 9999
};

//@Note(Leo): Feel free to add as many types as you pleased
// *IMPORTANT* - If you tend to rename this enum please contact inventory programmers, since enum type name (ECommonItemType) is used for lookup in gamecode (refferenced in ItemAttributeCollectionClass.h)
enum ECommonItemType
{
	NONE = 0,
   	BANDAGE = 1,
	MORPHINE = 2,
	SALINE = 3,
	TOURNIQUET = 4,
   	AMMO = 10,
	MG_AMMO = 11,
	AMMO_MORTAR = 12,
	FOOD = 20,
	FLARE = 30,
	BINOCULARS = 50,
	COMPASS = 51,
	FLASHLIGHT = 52,
	RADIO = 53,
	WATCH = 54,
	BAYONET = 55,
	EXPLOSIVE_CHARGE = 56,
	MINE = 57,
	DETONATOR = 58,
	NIGHT_VISION = 59,
	GPS = 60,
	HAND_SLOT_ITEM = 61,
	IDENTITY_ITEM = 70,
};

//TODO: move this to the UI constants (if exists)
const float		BUTTON_OPACITY_DISABLED	= 0.2;
const float		BUTTON_OPACITY_ENABLED	= 1.0;