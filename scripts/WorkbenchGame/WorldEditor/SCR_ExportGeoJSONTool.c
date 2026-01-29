#ifdef WORKBENCH
class SCR_GeoPointData
{
	vector m_vPosition;
	vector m_vInTangent;
	vector m_vOutTangent;

	//------------------------------------------------------------------------------------------------
	// constructor
	void SCR_GeoPointData(vector position, vector inTangent = "0 0 0", vector outTangent = "0 0 0")
	{
		m_vPosition = position;
		m_vInTangent = inTangent;
		m_vOutTangent = outTangent;
	}
}

enum SCR_EGeoExportType
{
	GeoJSON,
	SVG,
}

class SCR_GeoProperty
{
	string m_sName;
	string m_sValue;

	//------------------------------------------------------------------------------------------------
	// constructor
	void SCR_GeoProperty(string name, string value)
	{
		m_sName = name;
		m_sValue = value;
	}
}

class SCR_GeoExporter
{
	protected string m_sOutput;
	protected float m_fXShift;
	protected float m_fYShift;

	protected ref FileHandle m_File;

	static const string ROAD_GENERATOR_ENTITY = "RoadGeneratorEntity";
	static const string PREFAB_GENERATOR_ENTITY = "PrefabGeneratorEntity";

	static const string POLYLINE_SHAPE_ENTITY = "PolylineShapeEntity";
	static const string SPLINE_SHAPE_ENTITY = "SplineShapeEntity";

	static const string IS_CLOSED_PROPERTY = "IsClosed";
	static const string LINE_COLOR_PROPERTY = "LineColor";
	static const string ROAD_WIDTH_PROPERTY = "RoadWidth";

	//------------------------------------------------------------------------------------------------
	// constructor
	void SCR_GeoExporter(string path, int xShift, int yShift)
	{
		m_sOutput = path;
		m_fXShift = xShift;
		m_fYShift = yShift;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \return true on file opening success, false otherwise
	bool Init()
	{
		m_File = FileIO.OpenFile(m_sOutput, FileMode.WRITE);
		if (!m_File)
		{
			Print("Unable to open file to write " + m_sOutput, LogLevel.ERROR);
			return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//!
	void Close()
	{
		if (m_File)
			m_File.Close();
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	string GetPath()
	{
		return m_sOutput;
	}

	//------------------------------------------------------------------------------------------------
	protected void Export(string line)
	{
		m_File.WriteLine(line);
	}

	// methods to override

	//------------------------------------------------------------------------------------------------
	//!
	void BeginExport();

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] name
	//! \param[in] shapeType
	//! \param[in] points
	//! \param[in] props
	void FeatureExport(string name, GeoShapeType shapeType, array<ref SCR_GeoPointData> points, array<ref SCR_GeoProperty> props);

	//------------------------------------------------------------------------------------------------
	//!
	void EndExport();
}

class SCR_GeoSVGExporter : SCR_GeoExporter
{
	float m_fXMax = 20000;
	float m_fYMax = 20000;

	//------------------------------------------------------------------------------------------------
	override void BeginExport()
	{
		WorldEditor we = Workbench.GetModule(WorldEditor);

		vector min, max;
		we.GetTerrainBounds(min, max);
		m_fXMax = max[0];
		m_fYMax = max[2];

		Export("<?xml version=\"1.0\" encoding=\"utf-8\"?>");
		Export("<!-- Generator: Enfusion, SVG Export Plug-In  -->");

		Export("<svg version=\"1.1\" id=\"WE\" xmlns=\"http:/" + "/www.w3.org/2000/svg\" xmlns:xlink=\"http:/" + "/www.w3.org/1999/xlink\" x=\"0px\" y=\"0px\" width=\""+ (int)m_fXMax + "\" height=\"" + (int)m_fYMax + "\" viewBox=\"0 0 " + (int)m_fXMax + " " + (int)m_fYMax + "\">");
	}

	//------------------------------------------------------------------------------------------------
	override void FeatureExport(string name, GeoShapeType shapeType, array<ref SCR_GeoPointData> points, array<ref SCR_GeoProperty> props)
	{
		if (shapeType == GeoShapeType.POINT)
		{
			vector pos;
			foreach (SCR_GeoPointData point : points)
			{
				pos = point.m_vPosition;
				Export(string.Format("<path d=\"M%1,%2\"/>", pos[0] + m_fXShift, pos[2] + m_fYShift));
			}
		}
		else if (shapeType == GeoShapeType.POLYLINE)
		{
			int width = 1;

			string polyline = "<polyline fill=\"none\" stroke=\"#000000\" stroke-width=\"" + width + "\" stroke-miterlimit=\"10\" ";
			string pointsAttr = "points=\"";
			vector pos;

			foreach (int i, SCR_GeoPointData point : points)
			{
				if (i > 0)
					pointsAttr += " ";

				pos = point.m_vPosition;
				pointsAttr += string.Format("%1,%2", pos[0] + m_fXShift, pos[2] + m_fYShift);
			}

			polyline += pointsAttr;
			polyline += "\"/>";

			Export(polyline);
		}
		else if (shapeType == GeoShapeType.POLYGON)
		{
			string polyline = "<polygon ";
			string pointsAttr = "points=\"";
			vector pos;

			foreach (int i, SCR_GeoPointData point : points)
			{
				if (i > 0)
					pointsAttr += " ";

				pos = point.m_vPosition;
				pointsAttr += string.Format("%1,%2", pos[0] + m_fXShift, pos[2] + m_fYShift);
			}

			polyline += pointsAttr;
			polyline += "\"/>";

			Export(polyline);
		}
		else if (shapeType == GeoShapeType.UNKNOWN) // Unknown is Spline for us atm
		{
			int width = 1;
			foreach (SCR_GeoProperty geo : props)
			{
				if (geo.m_sName == ROAD_WIDTH_PROPERTY && name == ROAD_GENERATOR_ENTITY)
					width = geo.m_sValue.ToInt(); // TODO - break?
			}

			string polyline = "<path fill=\"none\" stroke=\"#000000\" stroke-width=\"" + width + "\" ";
			string dAttr = "d=\"";
			vector pos, nextPos;
			vector inTangent, outTangent;

			// https://developer.mozilla.org/en-US/docs/Web/SVG/Tutorial/Paths
			int countMinus1 = points.Count() - 1;
			foreach (int i, SCR_GeoPointData point : points)
			{
				if (i == countMinus1)
					return;

				SCR_GeoPointData nextPoint = points[i + 1];

				// Negate pos.z and add m_fYMax because our Z maps to SVG Y and SVG has Y down but we have Z up.
				// We need to mirror the Z(Y) axis. Also negate the tangent Y coordinate for the same reason.
				pos = point.m_vPosition;
				pos[0] = pos[0] + m_fXShift;
				pos[2] = -pos[2] + m_fYMax + m_fYShift;
				vector ot = point.m_vOutTangent;
				ot[2] = -ot[2];
				// Divide by 3 because Enfusion uses cubic Hermite splines, SVG uses cubic Bezier splines
				// Division by 3 converts these two representations https://en.wikipedia.org/wiki/Cubic_Hermite_spline#Representations
				outTangent = pos + ot / 3;

				nextPos = nextPoint.m_vPosition;
				nextPos[0] = nextPos[0] + m_fXShift;
				nextPos[2] = -nextPos[2] + m_fYMax + m_fYShift;
				vector it = nextPoint.m_vInTangent;
				it[2] = -it[2];
				// In tangent needs to be subtracted, in Enfusion we represent tangents always as "forward"
				inTangent = nextPos - it / 3;

				if (i == 0)
					dAttr += string.Format("M %1 %2", pos[0], pos[2]);

				dAttr += string.Format(" C %1 %2, %3 %4, %5 %6", outTangent[0], outTangent[2], inTangent[0], inTangent[2], nextPos[0], nextPos[2]);
			}

			polyline += dAttr;
			polyline += "\"/>";

			Export(polyline);
		}

	}

	//------------------------------------------------------------------------------------------------
	override void EndExport()
	{
		Export("</svg>");
	}
}

class SCR_GeoJSONExporter : SCR_GeoExporter
{
	protected int m_iIndex;

	//------------------------------------------------------------------------------------------------
	override void BeginExport()
	{
		Export("{\"type\": \"FeatureCollection\", \"features\": [");
	}

	//------------------------------------------------------------------------------------------------
	override void FeatureExport(string name, GeoShapeType shapeType, array<ref SCR_GeoPointData> points, array<ref SCR_GeoProperty> props)
	{
		string type = "Point";

		if (shapeType == GeoShapeType.UNKNOWN) // Spline is polyline for us
			shapeType = GeoShapeType.POLYLINE;

		if (shapeType == GeoShapeType.POLYLINE)
			type = "LineString";
		else if (shapeType == GeoShapeType.POLYGON)
			type = "Polygon";

		if (m_iIndex > 0)
			Export(","); // properties end

		Export("{\"type\": \"Feature\",\"properties\":{"); // properties
		Export("\"name\": \"" + name + "\"");

		foreach (SCR_GeoProperty geo : props)
		{
			Export(",\"" + geo.m_sName + "\": \"" + geo.m_sValue + "\"");
		}

		Export("},\"geometry\":{\"type\": \"" + type + "\",\"coordinates\":"); // geometry/coordinates

		if (shapeType == GeoShapeType.POLYGON)
			Export("[["); // polygons can have holes
		if (shapeType == GeoShapeType.POLYLINE)
			Export("["); // polygons can have holes

		int countMinus1 = points.Count() - 1;
		foreach (int i, SCR_GeoPointData point : points)
		{
			if (i != countMinus1)
				Export(string.Format("[%1,%2],", point.m_vPosition[0] + m_fXShift, point.m_vPosition[2] + m_fYShift));
			else
				Export(string.Format("[%1,%2]", point.m_vPosition[0] + m_fXShift, point.m_vPosition[2] + m_fYShift));
		}

		if (shapeType == GeoShapeType.POLYGON)
			Export("]]"); // polygons can have holes end
		if (shapeType == GeoShapeType.POLYLINE)
			Export("]"); // polygons can have holes end

		Export("}"); // coordinates/geometry end

		Export("}"); // properties end

		m_iIndex++;
	}

	//------------------------------------------------------------------------------------------------
	override void EndExport()
	{
		Export("]}");
	}
}

//! Exports selected entity as a GeoJSON or SVG file.
//! It support exporting PolylineShapeEntity and SplineShapeEntity (both closed and unclosed). Exporting Generators are also supported, but they needs to have a shapeType entity as a parent.
//! Other entities are exported as a single points.
[WorkbenchToolAttribute("Export Geographic data", "Export vector data as Geographical data", shortcut: "5", awesomeFontCode: 0xF56E)]
class SCR_ExportGeoJSONTool : WorldEditorTool
{
	[Attribute("$profile:export", UIWidgets.FileNamePicker, "Where to save exported file. Do not use file suffix, it will be created automatically.")]
	protected string m_sExportPath;

	[Attribute("0", UIWidgets.ComboBox, desc: "Export type", enumType: SCR_EGeoExportType)]
	protected SCR_EGeoExportType m_eType;

	[Attribute("0.0", UIWidgets.EditBox)]
	protected float m_fXShift;

	[Attribute("0.0", UIWidgets.EditBox)]
	protected float m_fYShift;

// you can uncomment this so this functionality can be used as a plugin, not tool
/*
	//------------------------------------------------------------------------------------------------
	override void Configure()
	{
		Workbench.ScriptDialog("Configure Export Geographic data tool", "", this);
	}

	//------------------------------------------------------------------------------------------------
	override void Run()
	{
		Export();
	}
*/

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Export")]
	protected void Execute()
	{
		Export();
	}

	//------------------------------------------------------------------------------------------------
	protected bool Export()
	{
		if (m_sExportPath.IsEmpty())
		{
			Print("Export file must be set.", LogLevel.ERROR);
			return false;
		}

		WorldEditor worldEditor = Workbench.GetModule(WorldEditor);
		WorldEditorAPI worldEditorAPI = worldEditor.GetApi();
		WBProgressDialog progress = new WBProgressDialog("Processing", worldEditor);

		int selectedCount = worldEditorAPI.GetSelectedEntitiesCount();
		if (selectedCount < 1)
		{
			Print("You need to select at least one entity.", LogLevel.ERROR);
			return false;
		}

		SCR_GeoExporter exporter;

		if (m_eType == SCR_EGeoExportType.SVG)
			exporter = new SCR_GeoSVGExporter(m_sExportPath + ".svg", m_fXShift, m_fYShift);
		else
			exporter = new SCR_GeoJSONExporter(m_sExportPath + ".json", m_fXShift, m_fYShift);

		if (!exporter.Init())
		{
			Print("Unable to initialize the exporter.", LogLevel.ERROR);
			return false;
		}

		Print("GeoExport: exporting " + selectedCount + " entities...", LogLevel.NORMAL);
		exporter.BeginExport();

		IEntitySource entitySource;
		array<ref SCR_GeoProperty> properties;
		Color color;
		ShapeEntity shapeEntity;
		SplineShapeEntity splineEntity;
		array<vector> positions;
		array<ref SCR_GeoPointData> pointData;
		typename theTypeName;

		float prevProgress, currProgress;
		for (int i; i < selectedCount; i++)
		{
			currProgress = i / selectedCount;
			if (currProgress - prevProgress >= 0.01)	// min 1%
			{
				progress.SetProgress(currProgress);		// expensive
				prevProgress = currProgress;
			}

			entitySource = worldEditorAPI.GetSelectedEntity(i);

			string classname = entitySource.GetClassName();
			string name;

			properties = {};

			// if a generator is selected, get its classname as name and take its parent shape
			// (but for PrefabGeneratorEntity)

			if (classname != SCR_GeoExporter.PREFAB_GENERATOR_ENTITY)
			{
				theTypeName = classname.ToType();
				if (theTypeName && theTypeName.IsInherited(SCR_GeneratorBaseEntity))
				{
					if (classname == SCR_GeoExporter.ROAD_GENERATOR_ENTITY)
					{
						float roadWidth;
						entitySource.Get(SCR_GeoExporter.ROAD_WIDTH_PROPERTY, roadWidth);
						properties.Insert(new SCR_GeoProperty(SCR_GeoExporter.ROAD_WIDTH_PROPERTY, roadWidth.ToString()));
					}

					name = classname;
					entitySource = entitySource.GetParent();
				}
			}

			// now, let's deal the entitySource itself

			if (entitySource)
				classname = entitySource.GetClassName();

			if (entitySource && classname == SCR_GeoExporter.POLYLINE_SHAPE_ENTITY || classname == SCR_GeoExporter.SPLINE_SHAPE_ENTITY)
			{
				if (name.IsEmpty())
					name = classname;

				bool isClosed;
				entitySource.Get(SCR_GeoExporter.IS_CLOSED_PROPERTY, isClosed);

				GeoShapeType shapeType = GeoShapeType.UNKNOWN; // INFO: Spline will be unknown for us, right now closed splines are not supported, but we can propage it thru properties or change FeatureExport signature
				if (classname == SCR_GeoExporter.POLYLINE_SHAPE_ENTITY)
				{
					if (isClosed)
						shapeType = GeoShapeType.POLYGON;
					else
						shapeType = GeoShapeType.POLYLINE;
				}

				entitySource.Get(SCR_GeoExporter.LINE_COLOR_PROPERTY, color);
				string colorProperty = string.Format("%1,%2,%3,%4", Math.Round(color.R() * 255), Math.Round(color.G() * 255), Math.Round(color.B() * 255), Math.Round(color.A() * 255));
				properties.Insert(new SCR_GeoProperty(SCR_GeoExporter.LINE_COLOR_PROPERTY, colorProperty));
				// we can add isClosed to the properties so we can react in the exporters

				shapeEntity = ShapeEntity.Cast(worldEditorAPI.SourceToEntity(entitySource)); // can't use entity since parent can be changed
				splineEntity = SplineShapeEntity.Cast(shapeEntity);

				positions = {};
				shapeEntity.GetPointsPositions(positions);

				pointData = {};

				foreach (int j, vector position : positions)
				{
					vector pos = shapeEntity.CoordToParent(position);
					vector inTangent;
					vector outTangent;

					if (splineEntity)
						splineEntity.GetTangents(j, inTangent, outTangent); // tangents are relative to entity

					pointData.Insert(new SCR_GeoPointData(pos, inTangent, outTangent));
				}

				if (isClosed && pointData.Count() > 2)
				{
					vector first = pointData[0].m_vPosition;
					vector last = pointData[pointData.Count() - 1].m_vPosition;
					if (first != last)
						pointData.Insert(new SCR_GeoPointData(first));
				}

				exporter.FeatureExport(classname, shapeType, pointData, properties);
			}
			else // null parent or not a shape
			{
				pointData = { new SCR_GeoPointData( worldEditorAPI.SourceToEntity(entitySource).GetOrigin()) };

				exporter.FeatureExport(classname, GeoShapeType.POINT, pointData, properties);
			}
		}

		Print(string.Format("GeoExport: entities export done to %1.", exporter.GetPath()), LogLevel.NORMAL);

		exporter.EndExport();
		exporter.Close();

		return true;
	}
}
#endif // WORKBENCH
