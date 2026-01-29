/** @ingroup Editable_Entities
*/ 
/*!
Unique entity keys.
For editor users to edit or even see an entity, at least one of their editor keys must match the entity's keys.
For example, if the entity has KEY_1 and KEY_2, while the editor has KEY_2 and KEY_8, the entity will be available, since both have KEY_2.
*/
enum EEditableEntityAccessKey
{
	DEFAULT = 1 << 0, ///< Unless changed manually, all entities have this key set by default.
	KEY_1 = 1 << 1, ///< Custom key 1
	KEY_2 = 1 << 2, ///< Custom key 2
	KEY_3 = 1 << 3, ///< Custom key 3
	KEY_4 = 1 << 4, ///< Custom key 4
	KEY_5 = 1 << 5, ///< Custom key 5
	KEY_6 = 1 << 6, ///< Custom key 6
	KEY_7 = 1 << 7, ///< Custom key 7
	KEY_8 = 1 << 8, ///< Custom key 8
	KEY_9 = 1 << 9, ///< Custom key 9
	KEY_10 = 1 << 10, ///< Custom key 10
	KEY_11 = 1 << 11, ///< Custom key 11
	KEY_12 = 1 << 12, ///< Custom key 12
	KEY_13 = 1 << 13, ///< Custom key 13
	KEY_14 = 1 << 14, ///< Custom key 14
	KEY_15 = 1 << 15, ///< Custom key 15
	KEY_16 = 1 << 16, ///< Custom key 16
	KEY_17 = 1 << 17, ///< Custom key 17
	KEY_18 = 1 << 18, ///< Custom key 18
	KEY_19 = 1 << 19, ///< Custom key 19
	KEY_20 = 1 << 20, ///< Custom key 20
	KEY_21 = 1 << 21, ///< Custom key 21
	KEY_22 = 1 << 22, ///< Custom key 22
	KEY_23 = 1 << 23, ///< Custom key 23
	KEY_24 = 1 << 24, ///< Custom key 24
	KEY_25 = 1 << 25, ///< Custom key 25
	KEY_26 = 1 << 26, ///< Custom key 26
	KEY_27 = 1 << 27, ///< Custom key 27
	KEY_28 = 1 << 28, ///< Custom key 28
	KEY_29 = 1 << 29, ///< Custom key 29
	KEY_30 = 1 << 30 ///< Custom key 30
};