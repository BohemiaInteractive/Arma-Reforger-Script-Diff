/*!
\defgroup WidgetAPI Widget UI system
\{
*/

typedef TypeID WidgetType;
typedef TypeID EventType;

typedef func WidgetEventCallback;
bool WidgetEventCallback(Widget w = null);

class WidgetType
{
	static const WidgetType TextWidgetTypeID;
	static const WidgetType MultilineEditBoxWidgetTypeID;
	static const WidgetType RichTextWidgetTypeID;
	static const WidgetType RenderTargetWidgetTypeID;
	static const WidgetType ImageWidgetTypeID;
	static const WidgetType ConsoleWidgetTypeID;
	static const WidgetType VideoWidgetTypeID;
	static const WidgetType RTTextureWidgetTypeID;
	static const WidgetType FrameWidgetTypeID;
	static const WidgetType ButtonWidgetTypeID;
	static const WidgetType PanelWidgetTypeID;
	static const WidgetType CheckBoxWidgetTypeID;
	static const WidgetType WindowWidgetTypeID;
	static const WidgetType ComboBoxWidgetTypeID;
	static const WidgetType ProgressBarWidgetTypeID;
	static const WidgetType SliderWidgetTypeID;
	static const WidgetType BaseListboxWidgetTypeID;
	static const WidgetType TextListboxWidgetTypeID;
	static const WidgetType GenericListboxWidgetTypeID;
	static const WidgetType EditBoxWidgetTypeID;
	static const WidgetType WorkspaceWidgetTypeID;
	static const WidgetType CanvasWidgetOldTypeID;
	static const WidgetType CanvasWidgetTypeID;
	static const WidgetType GridLayoutWidgetTypeID;
	static const WidgetType HorizontalLayoutWidgetTypeID;
	static const WidgetType VerticalLayoutWidgetTypeID;
	static const WidgetType OverlayWidgetTypeID;
	static const WidgetType ScrollLayoutWidgetTypeID;
	static const WidgetType SizeLayoutWidgetTypeID;
	static const WidgetType UniformGridLayoutWidgetTypeID;
	static const WidgetType BlurWidgetTypeID;
	static const WidgetType ScaleWidgetTypeID;
}

//-----------------------------------------------------------------------
enum WidgetNavigationDirection
{
	LEFT,
	RIGHT,
	UP,
	DOWN
}

enum HorizontalFillOrigin
{
	LEFT,
	CENTER,
	RIGHT,
}

enum VerticalFillOrigin
{
	TOP,
	CENTER,
	BOTTOM,
}

//-----------------------------------------------------------------------
// Canvas widget commands
//-----------------------------------------------------------------------
class CanvasWidgetCommand : Managed
{
}

class TextDrawCommand : CanvasWidgetCommand
{
	int m_iColor = 0xff000000;
	vector m_Position; //!< Only in XY, Z is ignored
	string m_sText;
	int m_iFontPropertiesId;
	float m_fSize = 24;
	vector m_Pivot; //!< Only in XY, Z is ignored
	float m_fRotation;
}

class LineDrawCommand : CanvasWidgetCommand
{
	int m_iColor = 0xff000000;
	ref array<float> m_Vertices; //!< 2D vertices such as [x0, y0, x1, y1, ... xn, yn]
	float m_fWidth;
	float m_fOutlineWidth;
	int m_iOutlineColor;
	ref SharedItemRef m_pTexture; //!< If null only filled line is drawn
	vector m_UVScale; //!< Only in XY, Z is ignored
	bool m_bShouldEnclose;
}

class PolygonDrawCommand : CanvasWidgetCommand
{
	int m_iColor = 0xff000000;
	ref array<float> m_Vertices; //!< 2D vertices such as [x0, y0, x1, y1, ... xn, yn]
	float m_fUVScale;
	ref SharedItemRef m_pTexture; //!< If null, only filled polygon is drawn
}

class TriMeshDrawCommand : CanvasWidgetCommand
{
	int m_iColor = 0xff000000;
	ref array<float> m_Vertices; //!< 2D vertices such as [x0, y0, x1, y1, ... xn, yn]
	ref array<int> m_Indices; //!< Indices to the m_Vertices array forming triangles
	float m_fUVScale;
	ref SharedItemRef m_pTexture; //!< If null, only filled mesh is drawn
}

class ImageDrawCommand : CanvasWidgetCommand
{
	int m_iColor = 0xffffffff;
	float m_fRotation;
	float m_fUV[4] = {0, 0, 1, 1}; //!< TopLeft.U, TopLeft.V, BottomRight.U, BottomRight.V
	vector m_Pivot;
	ref SharedItemRef m_pTexture;
	vector m_Position; //!< Only in XY, Z is ignored
	vector m_Size; //!< Only in XY, Z is ignored
	WidgetFlags m_iFlags = WidgetFlags.STRETCH; //!< 0, STRETCH, NOWRAP
}

//! Draws all commands from internal array in order
class CompositeDrawCommand : CanvasWidgetCommand
{
	ref array<ref CanvasWidgetCommand> m_Commands;
}

/*!
\}
*/
