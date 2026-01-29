enum SCR_EWeaponGroupFireMode
{
	//! Salvo mode, all weapons in the group fire simultanously
	SALVO = 1 << 0,
	//! Ripple mode, shoot "ripple quantity" weapons
	RIPPLE = 1 << 1,
	//! Sequentially go through all weapons in the group, firing them one after another
	SEQUENTIAL = 1 << 2
}