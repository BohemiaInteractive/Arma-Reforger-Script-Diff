/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Damage_DamageAreas
\{
*/

class DamageArea: ScriptAndConfig
{
	/*!
	Returns parent entity of this DamageArea
	*/
	proto external IEntity GetParent();
	// ! Returns shape of this DamageArea
	proto external BaseDamageAreaShape GetShape();

	// callbacks

	/*!
	Callback when an entity enters this DamageArea this frame
	*/
	event void OnAreaEntered(notnull IEntity entity);
	/*!
	Callback when an entity was already inside of the area and it is still in the area this frame
	*/
	event void OnAreaPersisted(notnull IEntity entity);
	/*!
	Callback when an entity exits this DamageArea this frame
	*/
	event void OnAreaExit(IEntity entity);
	event void DisplayInfo();
}

/*!
\}
*/
