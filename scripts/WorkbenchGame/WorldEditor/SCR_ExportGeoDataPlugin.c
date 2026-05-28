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
	GEO_JSON,
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
	protected float m_fXMax = 20000;
	protected float m_fYMax = 20000;

	protected static const string SVG_HEADER = "<svg version=\"1.1\" id=\"WE\""
		+ " xmlns=\"http:/" + "/www.w3.org/2000/svg\""
		+ " xmlns:xlink=\"http:/" + "/www.w3.org/1999/xlink\""
		+ " x=\"0px\" y=\"0px\" width=\"%1\" height=\"%2\" viewBox=\"0 0 %1 %2\">"; // %1 = xMax, %2 = yMax

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

		Export(string.Format(SVG_HEADER, (int)m_fXMax, (int)m_fYMax));
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
			string polygon = "<polygon ";
			string pointsAttr = "points=\"";
			vector pos;

			foreach (int i, SCR_GeoPointData point : points)
			{
				if (i > 0)
					pointsAttr += " ";

				pos = point.m_vPosition;
				pointsAttr += string.Format("%1,%2", pos[0] + m_fXShift, pos[2] + m_fYShift);
			}

			polygon += pointsAttr;
			polygon += "\"/>";

			Export(polygon);
		}
		else if (shapeType == GeoShapeType.UNKNOWN) // Unknown is Spline for us atm
		{
			int width = 1;
			foreach (SCR_GeoProperty geo : props)
			{
				if (geo.m_sName == ROAD_WIDTH_PROPERTY && name.ToType() && name.ToType().IsInherited(RoadGeneratorEntity))
				{
					width = geo.m_sValue.ToInt();
					break;
				}
			}

			string spline = "<path fill=\"none\" stroke=\"#000000\" stroke-width=\"" + width + "\" ";
			string dAttr = "d=\"";
			vector pos, nextPos;
			vector inTangent, outTangent;

			// https://developer.mozilla.org/en-US/docs/Web/SVG/Tutorial/Paths
			int countMinus1 = points.Count() - 1;
			foreach (int i, SCR_GeoPointData point : points)
			{
				if (i == countMinus1)
					break;

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

			spline += dAttr;
			spline += "\"/>";

			Export(spline);
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
		else
		if (shapeType == GeoShapeType.POLYGON)
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
//! It support exporting PolylineShapeEntity and SplineShapeEntity (both closed and unclosed). Exporting Generators is also supported, but they needs to have a shapeType entity as a parent.
//! Other entities are exported as single points.
[WorkbenchPluginAttribute(
	name: PLUGIN_NAME,
	description: "Export vector data as Geographical data",
	shortcut: "5",
	wbModules: { "WorldEditor" },
	category: SCR_PluginCategory.WORLDEDITOR_IMPORT_EXPORT,
	awesomeFontCode: 0xF56E)]
class SCR_ExportGeoDataPlugin : WorldEditorPlugin
{
	[Attribute("$profile:export", UIWidgets.FileNamePicker, "Where to save exported file. Do not use file suffix, it will be created automatically.")]
	protected string m_sExportPath;

	[Attribute(SCR_EGeoExportType.GEO_JSON.ToString(), UIWidgets.ComboBox, desc: "Export type", enums: SCR_ParamEnumArray.FromString("Geo JSON;SVG"))]
	protected SCR_EGeoExportType m_eType;

	[Attribute("0")]
	protected float m_fXShift;

	[Attribute("0")]
	protected float m_fYShift;

	protected static const string PLUGIN_NAME = "Export Geographic Data";

	//------------------------------------------------------------------------------------------------
	override void Run()
	{
		WorldEditorAPI worldEditorAPI = SCR_WorldEditorToolHelper.GetWorldEditorAPI();
		int selectedCount = worldEditorAPI.GetSelectedEntitiesCount();
		if (selectedCount < 1)
		{
			SCR_WorkbenchHelper.PrintDialog("You need to select at least one entity.", PLUGIN_NAME, level: LogLevel.ERROR);
			return;
		}

		if (Workbench.ScriptDialog(PLUGIN_NAME, "", this) == 0)
			return;

		Export();
	}

	//------------------------------------------------------------------------------------------------
	protected void Export()
	{
		if (m_sExportPath.IsEmpty())
		{
			SCR_WorkbenchHelper.PrintDialog("Export path must be set.", PLUGIN_NAME, level: LogLevel.ERROR);
			return;
		}

		string relativeFilePath;
		SCR_GeoExporter exporter;

		if (m_eType == SCR_EGeoExportType.SVG)
		{
			relativeFilePath = m_sExportPath + ".svg";
			exporter = new SCR_GeoSVGExporter(relativeFilePath, m_fXShift, m_fYShift);
		}
		else // SCR_EGeoExportType.GEO_JSON
		{
			relativeFilePath = m_sExportPath + ".json";
			exporter = new SCR_GeoJSONExporter(relativeFilePath, m_fXShift, m_fYShift);
		}

		if (!exporter.Init())
		{
			SCR_WorkbenchHelper.PrintDialog("Unable to initialise the exporter.", PLUGIN_NAME, level: LogLevel.ERROR);
			return;
		}

		Debug.BeginTimeMeasure();

		WorldEditor worldEditor = Workbench.GetModule(WorldEditor);
		WorldEditorAPI worldEditorAPI = worldEditor.GetApi();
		WBProgressDialog progress = new WBProgressDialog("Processing", worldEditor);

		int selectedCount = worldEditorAPI.GetSelectedEntitiesCount();
		Print("GeoExport: exporting " + selectedCount + " entities...", LogLevel.NORMAL);
		exporter.BeginExport();

		IEntitySource entitySource;
		array<ref SCR_GeoProperty> properties;
		Color color;
		ShapeEntity shapeEntity;
		SplineShapeEntity splineEntity;
		array<vector> positions;
		array<ref SCR_GeoPointData> pointData;
		typename typeName;

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

			typeName = classname.ToType();
			if (typeName && typeName.IsInherited(SCR_GeneratorBaseEntity) && !typeName.IsInherited(PrefabGeneratorEntity))
			{
				if (typeName.IsInherited(RoadGeneratorEntity))
				{
					float roadWidth;
					entitySource.Get(SCR_GeoExporter.ROAD_WIDTH_PROPERTY, roadWidth);
					properties.Insert(new SCR_GeoProperty(SCR_GeoExporter.ROAD_WIDTH_PROPERTY, roadWidth.ToString()));
				}

				name = classname;
				entitySource = entitySource.GetParent();
			}

			// now, let's deal the entitySource itself

			if (entitySource)
				classname = entitySource.GetClassName();

			if (entitySource && typeName == PolylineShapeEntity || typeName == SplineShapeEntity)
			{
				if (name.IsEmpty())
					name = classname;

				bool isClosed;
				entitySource.Get(SCR_GeoExporter.IS_CLOSED_PROPERTY, isClosed);

				GeoShapeType shapeType;
				if (typeName == PolylineShapeEntity)
				{
					if (isClosed)
						shapeType = GeoShapeType.POLYGON;
					else
						shapeType = GeoShapeType.POLYLINE;
				}
				else	// Spline will be unknown for us, right now closed splines are not supported
				{		// but we can propage it through properties or change FeatureExport signature
					shapeType = GeoShapeType.UNKNOWN;
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
				pointData = { new SCR_GeoPointData(worldEditorAPI.SourceToEntity(entitySource).GetOrigin()) };

				exporter.FeatureExport(classname, GeoShapeType.POINT, pointData, properties);
			}
		}

		exporter.EndExport();
		exporter.Close();

		Debug.EndTimeMeasure(typename.EnumToString(SCR_EGeoExportType, m_eType) + " export");

		SCR_WorkbenchHelper.PrintDialog("Geo data successfully exported to " + relativeFilePath, PLUGIN_NAME, level: LogLevel.NORMAL);
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Export", true)]
	protected int ButtonExport()
	{
		return 1;
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Cancel")]
	protected int ButtonCancel()
	{
		return 0;
	}
}
#endif // WORKBENCH
