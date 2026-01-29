//------------------------------------------------------------------------------------------------
class SCR_MapConstants
{
	const int CANVAS_COMMAND_VERTICES_LIMIT = 400; 		// hardcoded in ENF, should be increased on req
	const float MAX_PIX_PER_METER = 20;					// maximal possible zoom in the form of max allowed PixelPerUnit value
	
	const string MAP_FRAME_NAME = "MapFrame";			// name of the map.layout frame within mapmenu layout
	const string MAP_WIDGET_NAME = "MapWidget";			// name of the MapWidget within map layout
	const string DRAWING_WIDGET_NAME = "DrawingWidget";	// name of the CanvasWidget for drawing within map layout
	const string DRAWING_CONTAINER_WIDGET_NAME = "DrawingContainer"; //name of FrameWidget for creating drawn lines on map
	const string CFG_LAYERS_DEFAULT = "{3CAC09C3E89190F7}Configs/Map/MapLayersDefault.conf";
	const string CFG_PROPS_DEFAULT = "{AF65563F653FED68}Configs/Map/MapPropsDefault.conf";
	const string CFG_DESCTYPES_DEFAULT = "{FF6B20825D4A566C}Configs/Map/MapDescriptorDefaults.conf";
	const string CFG_DESCVIEW_DEFAULT = "{4FFDB559B60FC7BC}Configs/Map/MapDescriptorVisibilityDefault.conf";
};

//------------------------------------------------------------------------------------------------
//! Panning modes
enum EMapPanMode
{
	DRAG = 0,		// drag from center pan
	HORIZONTAL = 1,	// horizontal pan
	VERTICAL = 2,	// vertical pan
};

//------------------------------------------------------------------------------------------------
//! Mode of the map
enum EMapEntityMode
{
	FULLSCREEN = 1,
	EDITOR,
	SPAWNSCREEN,
	MINIMAP,
	PLAIN,
	TUTORIALFASTTRAVEL,
	COMBAT_SUPPORT_STATION,
	OPERATIONS_STATION,
	LOGISTICS_STATION,
};

//------------------------------------------------------------------------------------------------
//! Map cursor state
enum EMapCursorState
{
	CS_DISABLE 			= 0,
	CS_DEFAULT 			= 1,
	CS_MOVE 			= 1<<1,
	CS_PAN 				= 1<<2,
	CS_ZOOM 			= 1<<3,
	CS_HOVER 			= 1<<4,
	CS_SELECT 			= 1<<5,
	CS_MULTI_SELECTION 	= 1<<6,
	CS_DRAG 			= 1<<7,
	CS_DRAW 			= 1<<8,
	CS_CONTEXTUAL_MENU	= 1<<9,
	CS_MODIFIER			= 1<<10,
	CS_ROTATE			= 1<<11,
	CS_ASSIGN_WP		= 1<<12,
	CS_SUB_MENU			= 1<<13,
	CS_DIALOG			= 1<<14,
	CS_COMMAND_POSITIVE	= 1<<15,
	CS_COMMAND_NEGATIVE	= 1<<16,
	CS_LAST				= 1<<17
};

//------------------------------------------------------------------------------------------------
//! Flags for when cursor is placed on screen edges
enum EMapCursorEdgePos
{
	LEFT = 1,
	RIGHT = 2,
	TOP = 4,
	BOTTOM = 8
};

//------------------------------------------------------------------------------------------------
//! Map cursor multiselection type
enum EMapCursorSelectType
{
	RECTANGLE,
	CIRCLE
};

//------------------------------------------------------------------------------------------------
//! Map components which are not part of the component system
enum EMapOtherComponents
{
	NONE = 0,
	LEGEND_SCALE = 1,
	GRID = 2
};
