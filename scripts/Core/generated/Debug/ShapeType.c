/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Debug
\{
*/

enum ShapeType
{
	//! When Shape is created and not initialized yet.
	NONE,
	//! Just box
	BBOX,
	//! One lines
	LINE,
	//! Not implemented
	BRUSH,
	//! 3D orientation visualizer
	PIVOT,
	//! Not implemented
	POINT,
	//! Triangle mesh. Just triangle list, no tri-fan/tri-strip.
	MESH,
	//! Polygon
	POLYGON,
	//! Triangle mesh. Just triangle list, no tri-fan/tri-strip.
	INDEXEDMESH,
	//! Sphere represented by triangle mesh
	SPHERE,
	//! Cylinder represented by triangle mesh
	CYLINDER,
	//! Eight faced pyramid. Defined by bound-box, where middle part is  equal to horizontal extents of box and top/bottom apogees lies on top/bottom side of box.
	DIAMOND,
	//! Four sided pyramid. Defined by bound-box, where base is equal to bottom side of box.
	PYRAMID,
	//! Arrow pointing down
	ARROW,
	//! List of multiple lines
	LINE_LIST,
	//! Multiple lines in a single winding
	LINE_STRIP,
}

/*!
\}
*/
