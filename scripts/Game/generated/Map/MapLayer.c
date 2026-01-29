/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Map
\{
*/

class MapLayer
{
	/*!
	\brief This layer index
	*/
	proto external int Index();
	proto external void SetFactionSize(int size);
	/*!
	\brief Get descriptor properties for certain type. Returns null if iFaction is illegal or out of range.
	*/
	proto external MapDescriptorProps GetPropsFor( int iFaction, EMapDescriptorType type );
	/*!
	\brief Returns grid properties of the map layer.
	*/
	proto external MapGridProps GetGridProps();
	/*!
	\brief Returns legend properties of the map layer.
	*/
	proto external MapLegendProps GetLegendProps();
	/*!
	\brief Returns contour properties of the map layer.
	*/
	proto external MapContourProps GetContourProps();
	/*!
	\brief Returns link properties of the map layer.
	*/
	proto external MapLinkProps GetMapLinkProps();
	/*!
	\brief Returns map selection properties of the layer.
	*/
	proto external MapSelectionProperties GetSelectionProps();
	/*!
	\brief Returns roads/ paths properties of the layer.
	*/
	proto external MapRoadProps GetRoadProps();
	/*!
	\brief Returns powerlines properties of the layer.
	*/
	proto external MapPowerLineProps GetPowerLineProps();
	/*!
	\brief Returns building properties of the layer.
	*/
	proto external MapBuildingProps GetBuildingProps();
	/*!
	\brief Returns water body properties of the layer.
	*/
	proto external MapWaterBodyProps GetWaterBodyProps();
	/*!
	\brief Returns hill properties of the layer.
	*/
	proto external MapHillProps GetHillProps();
	proto external MapAreaProperties GetAreaProps();
	/*!
	\brief Returns drawing properties of the map layer.
	*/
	proto external MapDrawingProps GetMapDrawingProps();
	/*!
	\brief Sets giving information to the giving descriptor type of the given faction. FACTION_ALL (-1) sets for the whole factions.
	*/
	proto void SetAll(  const EMapDescriptorType eType, const int iFactionIndex,  const int _fcolor, const int _bcolor, const int _ocolor, const int _tcolor,  const float _selfscale, const float _textscale,  const float selfScaleMin = 0.25, const float selfScaleMax = 4,  const float textScaleMin = 8, const float textScaleMax = 40,  vector offset = vector.Zero, ResourceName font = "" );
	/*!
	\brief Sets giving color to the whole colorized descriptors of the given faction.
	*/
	proto external void ModifyFaction( int iFactionIndex, int uColor );
	proto external void SetCeiling(float limit);
	proto external float GetCeiling();
}

/*!
\}
*/
