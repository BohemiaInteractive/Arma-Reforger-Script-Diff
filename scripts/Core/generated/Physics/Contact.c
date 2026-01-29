/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Physics
\{
*/

/*!
Output structure for reporting collisions.
*/
sealed class Contact
{
	Physics Physics1;
	Physics Physics2;
	//! Surface properties of Object1
	SurfaceProperties	Material1;
	//! Surface properties of Object2
	SurfaceProperties	Material2;
	//! Impulse applied to resolve the collision
	float	Impulse;
	//! Index of collider on Object1
	int ShapeIndex1
	//! Index of collider on Object2
	int ShapeIndex2
	//! Collision axis at the contact point
	vector	Normal;
	//! Position of the contact point (world space)
	vector	Position;
	//! Velocity of Object1 before collision (world space)
	vector	VelocityBefore1;
	//! Velocity of Object2 before collision (world space)
	vector	VelocityBefore2;
	//! Velocity of Object1 after collision (world space)
	vector	VelocityAfter1;
	//! Velocity of Object2 after collision (world space)
	vector	VelocityAfter2;

	//! Relative speed of objects before collision (along the collision normal)
	proto external float GetRelativeNormalVelocityBefore();
	//! Relative speed of objects after collision (along the collision normal)
	proto external float GetRelativeNormalVelocityAfter();
}

/*!
\}
*/
