/*!
Bounding volume property. The layout must match BoundingVolumeClass in C++ because indices are used.
*/
[BaseContainerProps()]
class BoundingVolumeScr
{
	[Attribute("10 10 10", desc: "Dimensions of the bounding volume")]
	vector Size;
}
