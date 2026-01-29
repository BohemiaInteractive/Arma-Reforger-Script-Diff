class SCR_Shape
{
	static void GetBoundsPoints(vector min, vector max, out vector points[19])
	{		
		//--- Front
		points[0] = min;
		points[1] = Vector(max[0], min[1], min[2]);
		points[2] = Vector(max[0], max[1], min[2]);
		points[3] = Vector(min[0], max[1], min[2]);
		
		//--- Left
		points[4] = min;
		points[5] = Vector(min[0], min[1], max[2]);
		points[6] = Vector(min[0], max[1], max[2]);
		
		//--- Top
		points[7] = Vector(min[0], max[1], min[2]);
		points[8] = Vector(max[0], max[1], min[2]);
		points[9] = max;
		
		//--- Back
		points[10] = Vector(min[0], max[1], max[2]);
		points[11] = Vector(min[0], min[1], max[2]);
		points[12] = Vector(max[0], min[1], max[2]);
		
		//--- Right
		points[13] = max;
		points[14] = Vector(max[0], max[1], min[2]);
		points[15] = Vector(max[0], min[1], min[2]);
		
		//--- Bottom
		points[16] = Vector(max[0], min[1], max[2]);
		points[17] = Vector(min[0], min[1], max[2]);
		points[18] = min;
	}
	static Shape CreateBounds(vector min, vector max, int color, ShapeFlags flags)
	{
		vector points[19];
		GetBoundsPoints(min, max, points);
		
		return Shape.CreateLines(Color.RED, ShapeFlags.NOZBUFFER, points, 19);
	}
	
	/*!
	Generate area mesh.
	\param positions Border segment positions
	\param height Border height
	\param material Material path
	\param strechMaterial True to stretch the material along the area border instead of mapping it on each segment.
	\return Mesh to be mapped on an entity
	*/
	static Resource CreateAreaMesh(array<vector> positions, float height, string material, bool strechMaterial)
	{
		if (!material)
			return null;
		
		int resolution = positions.Count();
		if (resolution == 0) return null;
		
		if (resolution * 12 > 720)
		{
			Print(string.Format("Cannot generate shape, resolution %1 is above maximum value %2!", resolution, 720 / 12), LogLevel.ERROR);
			return null;
		}
		
		vector verts[244];
		int indices[732];
		float uvs[1464];
		
		float dirStep = Math.PI2 / resolution;
		float uvsTemplate[] = {0,0, 0,1, 1,0, 1,1};
		int vertsTemplate[] = {0,2,1, 1,2,3, 3,2,1, 1,2,0};
		int indicesMod = 4 * resolution;
		int vI, iI, uI;
		vector vectorUp = vector.Up * height;
		vector pos0, pos1, pos2, pos3;
		vector matrix[4];
		
		for (int v = -1; v < resolution; v++)
		{
			/*
				1 - 3 - 5...
			    | \ | \ |
				0 - 2 - 4...
			
				Vertex 1: 0-2-1
				Vertex 2: 1-2-3
				Vertex 3: 2-4-3
				Vertex 4: 3-4-5
				...
			*/
			int id = v;
			if (id < 0) id += resolution;
			pos2 = positions[id];
			pos3 = pos2 + vectorUp;
			
			if (v != -1)
			{
				verts[vI] = pos3; vI++;
				verts[vI] = pos2; vI++;
				verts[vI] = pos1; vI++; 
				verts[vI] = pos0; vI++;
				
				int iIS = iI;
				for (int i = 0; i < 12; i++)
				{
					indices[iI] = (vI + vertsTemplate[iI - iIS]) % indicesMod; iI++;
				}
				int uIS = uI;
				for (int i = 0; i < 4; i++)
				{
					if (strechMaterial)
					{
						uvs[uI] = (resolution - v + uvsTemplate[uI - uIS]) / resolution;
						if (v == 0) uvs[uI] = uvs[uI] - 1;
					}
					else
					{
						uvs[uI] = uvsTemplate[uI - uIS]; 
					}
					uI++;
					uvs[uI] = uvsTemplate[uI - uIS]; uI++;
				}
			}
			
			pos0 = pos2;
			pos1 = pos3;
		}

		int numVertices[] = {vI};
		int numIndices[] = {iI};
		string materials[1] = {material};
		
		Resource res = MeshObject.Create(1, numVertices, numIndices, materials, 0);
		MeshObject meshObject = res.GetResource().ToMeshObject();
		meshObject.UpdateVerts(0, verts, uvs);
		meshObject.UpdateIndices(0, indices);
		
		return res;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Draw debug circle with outline using specified colors and shape flags
	Circle stems from origin of the transform and rotating around the up vector
	Circle has fixed section count of 36
	\param transform Orientation matrix of the circle - side, normal, direction, center position
	\param radius Radius of the circles [m]
	\param colorOutline color of the outline
	\param colorPlane color of the circle plane
	\param shapeFlags flags for rendering the basic shape
	*/
	static void DrawCircle(vector transform[4], float radius, int colorOutline, int colorPlane, ShapeFlags shapeFlags)
	{

		const int sectionCount = 36;
		const int linesCount = sectionCount + 1; // one extra line is needed to complete outline
		const int trisCount = sectionCount * 3;
		vector lines[linesCount];
		vector tris[trisCount];

		// If angle and section count become parametrized, this could become DrawArc
		float sectionStep = -360 / sectionCount;

		vector pivot = transform[3];
		vector point = (vector.Forward * radius).Multiply4(transform);

		for (int i; i < sectionCount; i++)
		{
			lines[i] = point;
			tris[i*3] = pivot;
			tris[i*3 + 1] = point;
			point = (Vector(sectionStep * (i + 1), 0, 0).AnglesToVector() * radius);
			point = point.Multiply4(transform);
			tris[i*3 + 2] = point;
		}

		// one extra line is needed to complete outline
		lines[sectionCount] = point;

		Shape.CreateLines(colorOutline, shapeFlags, lines, linesCount);
		Shape.CreateTris(colorPlane, shapeFlags, tris, sectionCount);
	}
}
