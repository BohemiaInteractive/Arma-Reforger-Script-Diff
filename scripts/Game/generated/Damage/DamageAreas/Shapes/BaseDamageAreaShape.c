/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Damage_DamageAreas_Shapes
\{
*/

class BaseDamageAreaShape: ScriptAndConfig
{
	proto external vector GetWorldPosition();
	//! Returns the local transform of this shape
	proto external void GetTransform(out vector mat[]);
	//! Sets the local transform for this area damage.
	proto external void SetTransform(vector mat[]);
	//! Returns the damage area that is using this shape
	proto external DamageArea GetDamageArea();
	//! Returns the world transform of this shape.
	proto external void GetWorldTransform(out vector mat[]);

	// callbacks

	//! Returns true if the entity is in the area of influence of this shape (in contact)
	event bool ShouldAffectEntity(IEntity entityToAffect);
	//! Returns true if this position is contained inside of the shape
	event bool AffectsPosition(vector position);
	//! Returns the diameter (radius * 2) of a sphere that encapsulates the entire shape
	event float GetDiameter();
	//! Called when the area has to be drawn.
	//! \param color: The default color value to draw the area. White for disabled, red for enabled
	event void Diag_DrawArea(int color);
}

/*!
\}
*/
