/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Entities
\{
*/

class GeneratorBaseEntityClass: GenericEntityClass
{
}

class GeneratorBaseEntity: GenericEntity
{
	/*!
	Specific implementation for given generator which handles what should happen when shape is initialized for the first time
	Do NOT call Workbench API functions from here.
	Called from `OnShapeInit`. Should not be called directly.
	*/
	event protected void OnShapeInitInternal(IEntitySource shapeEntitySrc, ShapeEntity shapeEntity);
	/*!
	Implementation of what should happen before transform (coords, angles, scale) of shape changes
	Exists because some generators do not care whether their parent shape moved while others do (transform change does not invoke init).
	Called from `BeforeShapeTransform`. Should not be called directly.
	*/
	event protected void BeforeShapeTransformInternal(IEntitySource shapeEntitySrc, ShapeEntity shapeEntity, inout vector oldTransform[4]);
	/*!
	Implementation of what should happen when transform (coords, angles, scale) of shape changes
	Exists because some generators do not care whether their parent shape moved while others do (transform change does not invoke init).
	Called from `OnShapeTransform`. Should not be called directly.
	*/
	event protected void OnShapeTransformInternal(IEntitySource shapeEntitySrc, ShapeEntity shapeEntity, array<vector> mins, array<vector> maxes);
	/*!
	Specific implementation for given generator which handles what should happen when shape changes
	Workbench API functions may be called from here, do NOT wrap them in begin/end entity edit.
	Called from `OnShapeChanged`. Should not be called directly.

	Bounding boxes (in world space) of areas where was the parent shape potentially changed are provided in mins and maxes arrays.
	For each i mins[i] and maxes[i] form a bounding box.

	\param shapeEntitySrc Parent of this generator
	\param shapeEntity Parent of this generator
	\param mins Minimum values of bounding boxes
	\param maxes Maximum values of bounding boxes
	*/
	event protected void OnShapeChangedInternal(IEntitySource shapeEntitySrc, ShapeEntity shapeEntity, array<vector> mins, array<vector> maxes);
	/*!
	Specific implementation for given generator which can directly react to changes in point data.
	Workbench API functions may be called from here, do NOT wrap them in begin/end entity edit.
	Called from `OnPointDataChanged`. Should not be called directly.

	\param situation Enum int, 2 = point data were added, 4 = before point data is changed, 5 = after point data is changed.
	\param propertyID Name of the property that was changed
	\param pointIndices Indices of points on which the properties were changed.
	*/
	event protected void OnPointDataChangedInternal(IEntitySource shapeEntitySrc, ShapeEntity shapeEntity, PointChangedSituation situation, string propertyID, array<int> pointIndices);
	/*!
	Specific implementation for given generator which can directly react to changes in points.
	Workbench API functions may be called from here, do NOT wrap them in begin/end entity edit.
	Called from `OnPointChanged`. Should not be called directly.

	\param situation Enum int, 0 = moved_from position, 1 = moved_to position, 2 = added point at position, 3 = removed point from position.
	\param pointIndex Index of the point on which the property was changed.
	\param position vector in LS at which the situation happened.
	*/
	event protected void OnPointChangedInternal(IEntitySource shapeEntitySrc, ShapeEntity shapeEntity, PointChangedSituation situation, int pointIndex, vector position);
	/*!
	Implementation of what should happen when anchor snapping happens
	Called from `OnShapeChanged`. Should not be called directly.
	\param shapeEntitySrc Parent of this generator
	\param parentAnchor Anchor on parent which snapped
	\param other Other shape taking part in snapping
	\param otherAnchor Anchor index on the other entity
	\param isReciever True if parent of this generator did not move during snapping - something else snapped to it, false otherwise
	*/
	event protected void OnAnchorSnappedInternal(IEntitySource shapeEntitySrc, int parentAnchor, IEntitySource other, int otherAnchor, bool isReciever);
	/*!
	Implementation of whether this generator allows snapping of given two anchors

	\see OnAnchorSnappedInternal
	\return False if `parentAnchor` from `shapeEntitySrc` and `otherAnchor` from `other` should not be snapped, true otherwise
	*/
	event protected bool CanAnchorSnapInternal(IEntitySource shapeEntitySrc, int parentAnchor, IEntitySource other, int otherAnchor, bool isReciever);
	/*!
	Implementation for event emitted when a part of shape which is inside the generator's parent shape bounding box changes.
	The scope of which parts of the other shape changed (which also intersect or stopped intersecting our shape's bbox) is given by bounding boxes
	(in world space) provided in `mins` and `maxes` arrays.
	For each i: mins[i] and maxes[i] form a bounding box.
	The intersection is tested only in the XZ axis (when looking from above),
	e.g. `other` may be high above `shapeEntitySrc` and this event will still be triggered.

	\param shapeEntitySrc Parent of this generator
	\param other The intersecting shape
	\param mins Minimum values of bounding boxes
	\param maxes Maximum values of bounding boxes
	*/
	event protected void OnIntersectingShapeChangedXZInternal(IEntitySource shapeEntitySrc, IEntitySource other, array<vector> mins, array<vector> maxes);

	/*!
	Called when parent shape is first initialized - called only once
	Handles calling either script or c++ implementation of OnShapeInitInternal
	*/
	proto external void OnShapeInit(IEntitySource shapeEntitySrc, ShapeEntity shapeEntity);
	/*!
	Called when parent shape's transform (coords, angles, scale) changes
	Handles calling either script or c++ implementation of BeforeShapeTransformInternal
	*/
	proto external void BeforeShapeTransform(IEntitySource shapeEntitySrc, ShapeEntity shapeEntity, inout vector oldTransform[4]);
	/*!
	Called when parent shape's transform (coords, angles, scale) changes
	Handles calling either script or c++ implementation of OnShapeTransformInternal
	*/
	proto external void OnShapeTransform(IEntitySource shapeEntitySrc, ShapeEntity shapeEntity, array<vector> mins, array<vector> maxes);
	/*!
	Called when parent shape is changed in any way (except for cases handled by `OnShapeTransform` and `OnShapeInit`)
	Handles calling either script or c++ implementation of OnShapeWBInitInternal
	*/
	proto external void OnShapeChanged(IEntitySource shapeEntitySrc, ShapeEntity shapeEntity, array<vector> mins, array<vector> maxes);
	/*!
	Called when PointData on one of the points has been changed. OnShapeChanged is also called in that case.
	*/
	proto external void OnPointDataChanged(IEntitySource src, ShapeEntity shapeEntity, PointChangedSituation situation, string propertyID, array<int> pointIndices);
	/*!
	Called when a point was changed. OnShapeChanged is also called in that case.
	*/
	proto external void OnPointChanged(IEntitySource src, ShapeEntity shapeEntity, PointChangedSituation situation, int pointIndex, vector position);
	/*!
	Called when anchor snapping happend, either a parent's anchor snapped to something or something to a parent's anchor
	Handles calling either script or c++ implementation of OnAnchorSnappedInternal
	*/
	proto external void OnAnchorSnapped(IEntitySource shapeEntitySrc, int parentAnchor, IEntitySource other, int otherAnchor, bool isReciever);
	/*!
	Called when anchor snapping is attempted
	Handles calling either script or c++ implementation of CanAnchorSnapInternal
	*/
	proto external bool CanAnchorSnap(IEntitySource shapeEntitySrc, int parentAnchor, IEntitySource other, int otherAnchor, bool isReciever);
	/*!
	Called when a shape intersecting our bbox changed or moved out
	Handles calling either script or c++ implementation of OnIntersectingShapeChangedXZInternal
	*/
	proto external void OnIntersectingShapeChangedXZ(IEntitySource shapeEntitySrc, IEntitySource other, array<vector> mins, array<vector> maxes);
}

/*!
\}
*/
