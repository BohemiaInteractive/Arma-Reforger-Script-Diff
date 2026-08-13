/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Debug
\{
*/

enum ShapeFlags
{
	DEFAULT,
	//! Do not compare z-buffer when render
	NOZBUFFER,
	//! Do not update z-buffer when render
	NOZWRITE,
	//! Render just wire-frame outline. No solid faces
	WIREFRAME,
	//! Is translucent
	TRANSP,
	//! Double-sided (do not cull back-faces)
	DOUBLESIDE,
	//! Rendered just once and then it's automatically destroyed. Do not keep pointer to these!!
	ONCE,
	//! Render just solid faces. No wire-frame outline.
	NOOUTLINE,
	//! Render just back faces
	BACKFACE,
	//! Do not cull shapes by view frustum
	NOCULL,
	//! Make it visible. Set by default
	VISIBLE,
	//! Additive blending (works with SS_TRANSP or SS_CONTRAST)
	ADDITIVE,
	//! dither according depth
	DEPTH_DITHER,
	//! More aggressive culling of far shapes with this flag
	CULL_FAR,
	//! Draw double sided, highlighting backfaces to visualize vertex winding order errors
	SHOW_ERRORS,
	/*!
	Use contrast rendering mode which darkens the shape color when rendered over a bright background or lightens the shape color over dark backgrounds
	when this flag is combined with SS_ADDITIVE. This flag should not be set explicitly, but rather via the SetContrast() function.
	*/
	CONTRAST,
	//! Do not insert it to shape-collection
	NOINSERT,
	//! Flat shading (no vertex normals are required)
	FLAT,
	// Vertex format flags
	SVF_NORMAL,
	SVF_COLOR,
}

/*!
\}
*/
