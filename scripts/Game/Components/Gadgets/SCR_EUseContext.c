enum SCR_EUseContext
{
	NONE = 0,
	FROM_ACTION = 1,
	FROM_INVENTORY = 1 << 1,
	FROM_QUICKSLOT = 1 << 2,
	CUSTOM = 1 << 3,
}