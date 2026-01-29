enum EPreviewEntityFlag
{
	HORIZONTAL			= 1 << 0, ///< Entity will remain horizontal and not be oriented to terrain
	ORIENT_CHILDREN		= 1 << 1, ///< Entity's children will be evaluated for orienting to terrain
	UNDERWATER			= 1 << 2, //< Entity is being edited under water surface. Valid only for root entity.
	IGNORE_TERRAIN		= 1 << 3, //< Initial terrain under entity will not be evaluated. Valid only for root entity.
	IGNORE_PREFAB		= 1 << 4, ///< Don't use pre-defined preview prefabs. Valid only for root entity.
	ONLY_EDITABLE		= 1 << 5, ///< Only editable entities will be processed
	REGISTER_EDITABLE	= 1 << 6, ///< Always register editable entity on the preview, even when it's a child of another preview
	GEOMETRY			= 1 << 7, ///< Relative height of entities is calculated to the nearest geometry below, not to terrain
	EDITABLE			= 1 << 8, ///< Preview represents an editable entity
};