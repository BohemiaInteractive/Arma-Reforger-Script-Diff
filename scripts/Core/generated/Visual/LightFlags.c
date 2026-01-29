/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Visual
\{
*/

enum LightFlags
{
	//! Cheap hint for explosion, muzzles etc.
	CHEAP,
	//! Casts shadows
	CASTSHADOW,
	//! Light does lookup to cubic texture
	LOOKUP,
	//! Light contributes to diffuse component only (has disabled specular component)
	DIFFUSE_ONLY,
	//! Special type of light from sky. It's used as directional+ambient light for ambient pass
	SKY,
	//! Is it dynamic light
	DYNAMIC,
}

/*!
\}
*/
