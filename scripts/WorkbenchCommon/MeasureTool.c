[WorkbenchToolAttribute(
	"Measure Tool",
	"Measure length along a 3D line\n" +
	"Line segment length is rendered at the center of segment.\nTotal length rendered next to the cursor.\n\n" +
	"LMB - Start measuring\nESC - Cancel measuring",
	awesomeFontCode: 0xF545)]
class MeasureTool : WorldEditorTool
{
	[Attribute(desc: "Measure against objects", category: "General")]
	protected bool m_bCheckAgainstEntities;

	protected ref DebugTextScreenSpace m_Text;
	protected ref DebugTextScreenSpace m_Crosshair;
	protected ref array<ref DebugTextWorldSpace> m_aSegmentLengths;
	protected ref Shape m_Polyline;

	protected vector m_vPrevious3DPoint;
	protected vector m_vCurrent3DPoint;
	protected float m_fDistancePrevious;
	protected float m_fDistanceCurrent;
	protected vector m_aLinePoints[1000];
	protected int m_iLinePointsCount;
	protected float m_fLastX;
	protected float m_fLastY;

	//------------------------------------------------------------------------------------------------
	override void OnMouseMoveEvent(float x, float y)
	{
		vector traceStart;
		vector traceEnd;
		vector traceDir;

		m_Crosshair.SetTextColor(ARGBF(1, 1.0, 1.0, 1.0));
		m_Text.SetTextColor(ARGBF(1, 1.0, 1.0, 1.0));
		m_Crosshair.SetPosition(x - 9, y - 16);
		m_Crosshair.SetText("+");
		m_Text.SetPosition(x + 15, y);

		TraceFlags traceFlags = TraceFlags.WORLD;
		if (m_bCheckAgainstEntities)
			traceFlags = TraceFlags.ENTS | TraceFlags.WORLD;

		if (m_API.TraceWorldPos(x, y, traceFlags, traceStart, traceEnd, traceDir))
		{
			m_vCurrent3DPoint = traceEnd;

			if (m_iLinePointsCount >= 1)
			{
				m_fDistanceCurrent = (m_vPrevious3DPoint - m_vCurrent3DPoint).Length();
				m_Text.SetText((m_fDistanceCurrent + m_fDistancePrevious).ToString() + " m");
				m_aLinePoints[m_iLinePointsCount] = traceEnd;
				m_Polyline = Shape.CreateLines(ARGB(255, 255, 255, 255), ShapeFlags.NOZBUFFER | ShapeFlags.TRANSP, m_aLinePoints, m_iLinePointsCount + 1);
				m_fLastX = x;
				m_fLastY = y;
			}
		}
		else
		{
			// line is being drawn and cursor is out of terrain bounds, stick cursor and length info to terrain's edge
			if (m_iLinePointsCount >= 1)
			{
				m_Crosshair.SetPosition(m_fLastX - 9, m_fLastY - 16);
				m_Text.SetPosition(m_fLastX + 15, m_fLastY);
			}
			else
			// line is not being drawn and cursor is out of terrain bounds
			{
				m_Crosshair.SetText("");
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnMousePressEvent(float x, float y, WETMouseButtonFlag buttons)
	{
		vector traceStart;
		vector traceEnd;
		vector traceDir;
		vector text_position;

		if (m_iLinePointsCount == 0)
		{
			m_vPrevious3DPoint = m_vCurrent3DPoint;
		}

		TraceFlags traceFlags = TraceFlags.WORLD;
		if (m_bCheckAgainstEntities)
			traceFlags = TraceFlags.ENTS | TraceFlags.WORLD;

		if (m_API.TraceWorldPos(x, y, traceFlags, traceStart, traceEnd, traceDir))
		{
			m_vCurrent3DPoint = traceEnd;
			m_fDistancePrevious += m_fDistanceCurrent;

			//Render line segments length
			if (m_iLinePointsCount != 0)
			{
				text_position = m_vPrevious3DPoint + ((m_vCurrent3DPoint - m_vPrevious3DPoint).Normalized() * (m_vPrevious3DPoint - m_vCurrent3DPoint).Length() * 0.5);
				m_aSegmentLengths.Insert(
					DebugTextWorldSpace.Create(
						m_API.GetWorld(),
						(m_vPrevious3DPoint - m_vCurrent3DPoint).Length().ToString() + " m",
						0,
						text_position[0],
						text_position[1],
						text_position[2],
						15,
						ARGBF(1, 1, 1, 1),
						0x00000000));
			}

			m_aLinePoints[m_iLinePointsCount] = m_vCurrent3DPoint;
			// Print(linePts[m_iLinePointsCount]);
			m_iLinePointsCount++;
			// Print(m_iLinePointsCount);
			m_vPrevious3DPoint = m_vCurrent3DPoint;
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnKeyPressEvent(KeyCode key, bool isAutoRepeat)
	{
		//Cancel measuring
		if (key == KeyCode.KC_ESCAPE && isAutoRepeat == false)
		{
			m_Text.SetText("");
			m_aSegmentLengths.Clear();
			m_fDistancePrevious = 0.0;
			m_fDistanceCurrent = 0.0;
			m_vPrevious3DPoint = "0 0 0";
			m_iLinePointsCount = 0;
			m_Polyline = Shape.CreateLines(ARGB(255, 255, 0, 0), ShapeFlags.NOZBUFFER | ShapeFlags.TRANSP, m_aLinePoints, m_iLinePointsCount + 1);
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnActivate()
	{
		m_Text = DebugTextScreenSpace.Create(m_API.GetWorld(), "", 0, 100, 100, 14, ARGBF(1, 1, 1, 1), 0x00000000);
		m_Crosshair = DebugTextScreenSpace.Create(m_API.GetWorld(), "", 0, 0, 0, 30, ARGBF(1, 1, 1, 1), 0x00000000);
		m_fDistancePrevious = 0.0;
		m_aSegmentLengths = {};
	}

	//------------------------------------------------------------------------------------------------
	override void OnDeActivate()
	{
		m_Text = null;
		m_Crosshair = null;
		m_aSegmentLengths.Clear();
		m_fDistancePrevious = 0.0;
		m_fDistanceCurrent = 0.0;
		m_vPrevious3DPoint = "0 0 0";
		m_iLinePointsCount = 0;
		m_Polyline = null;
	}
};
