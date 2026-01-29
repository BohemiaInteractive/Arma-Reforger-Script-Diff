//! Types of armor and their corresponding protection level. Protection level is int value used for damage reduction.
//! @ingroup ArmorLevels
enum SCR_EArmorLevels
{
	NONE = 0,			//!< No protection value
	CUSTOM_01 = 10,		//!< Arbitrary protection values
	CUSTOM_02 = 15,
	CUSTOM_03 = 20,
	CUSTOM_04 = 25,
	TYPE_I = 5,		//!< Balanced to conform to NIJ standard protection values
	TYPE_IIA = 7,
	TYPE_II = 8,
	TYPE_IIIA = 11,
	TYPE_III = 17,
	CLASS_1 = 6,		//!< Balanced to conform to GOST standard protection values
	CLASS_2 = 9,
	CLASS_2A = 10,
	CLASS_3 = 13,
	CLASS_4 = 18,
}
