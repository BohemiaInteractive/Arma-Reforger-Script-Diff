/*************************************************************************************
*	This file contains additional debug shape functions
*************************************************************************************/

//------------------------------------------------------------------------------------------------
// Visualizes a bone in an entity with axial lines
void ShowBoneDebug(IEntity ent, int bone, float scale)
{
	vector boneMat[4];
	if (ent.GetAnimation().GetBoneMatrix(bone, boneMat))
	{
		vector mat[4];
		ent.GetTransform(mat);

		vector pBase = boneMat[3];
		vector p0 = boneMat[0] * scale + boneMat[3];
		vector p1 = boneMat[1] * scale + boneMat[3];
		vector p2 = boneMat[2] * scale + boneMat[3];
		pBase = pBase.Multiply4(mat);
		p0 = p0.Multiply4(mat);
		p1 = p1.Multiply4(mat);
		p2 = p2.Multiply4(mat);
		Shape.Create(ShapeType.LINE, ARGB(255, 255, 0, 0), ShapeFlags.ONCE|ShapeFlags.NOZBUFFER, pBase, p0);
		Shape.Create(ShapeType.LINE, ARGB(255, 0, 255, 0), ShapeFlags.ONCE|ShapeFlags.NOZBUFFER, pBase, p1);
		Shape.Create(ShapeType.LINE, ARGB(255, 0, 0, 255), ShapeFlags.ONCE|ShapeFlags.NOZBUFFER, pBase, p2);
	}
}

//------------------------------------------------------------------------------------------------
//! \param[in] pos position in worldspace
//! \param[in] aroundDir normalized direction in worldsapce towards the base
//! \param[in] coneAngX angle on its local X axis
//! \param[in] coneAngY angle on its local Y axis
//! \param[in] coneLength length of the sides (not height of the cone - for round base you can use coneLength = coneHeight / Math.Cos(coneAngleInRadians))
//! \param[in] color
//! \param[in] subdivisions
//! \param[in] flags
Shape CreateCone(vector pos, vector aroundDir, float coneAngX, float coneAngY, float coneLength, int color, int subdivisions, ShapeFlags flags)
{
	if (subdivisions < 2)
		subdivisions = 2;
	if (subdivisions > 50)
		subdivisions = 50;

	vector rt = "1 0 0";
	if (rt == aroundDir)
		rt = "0 1 0";
	vector fw = rt * aroundDir;
	fw.Normalize();
	rt = aroundDir * fw;
	rt.Normalize();

	vector mat[3];
	mat[0] = rt;
	mat[1] = aroundDir;
	mat[2] = fw;

	float sectionDeg = 360 / subdivisions;
	subdivisions++;

	vector pts[200];
	int curPts = 0;
	for (int i = 0; i < subdivisions; i++)
	{
		float xValue = Math.Sin(sectionDeg * i * Math.DEG2RAD);
		xValue *= xValue;
		float yValue = Math.Cos(sectionDeg * i * Math.DEG2RAD);
		yValue *= yValue;

		float lineAng = Math.AbsFloat(xValue * coneAngX);
		lineAng += Math.AbsFloat(yValue * coneAngY);

		vector pt = Vector(0, 0, coneLength);
		vector rotMat[3];
		Math3D.AnglesToMatrix(Vector(sectionDeg * i, 90 - lineAng, 0), rotMat);
		pt = pt.Multiply3(rotMat);
		pt = pt.Multiply3(mat);
		pts[curPts] = pt + pos;
		curPts++;
		pts[curPts] = pos;
		curPts++;
		pts[curPts] = pt + pos; // Have to make an additional line back to the circle to keep it as one shape object
		curPts++;
	}
	pts[curPts] = pts[0];

	return Shape.CreateLines(color, flags, pts, curPts);
}

//------------------------------------------------------------------------------------------------
Shape CreateCircle(vector pos, vector aroundDir, float radius, int color, int subdivisions, ShapeFlags flags)
{
	if (subdivisions < 2)
		subdivisions = 2;
	if (subdivisions > 50)
		subdivisions = 50;

	vector rt = "1 0 0";
	if (rt == aroundDir)
		rt = "0 1 0";
	vector fw = rt * aroundDir;
	fw.Normalize();
	rt = aroundDir * fw;
	rt.Normalize();

	vector mat[3];
	mat[0] = rt;
	mat[1] = aroundDir;
	mat[2] = fw;

	float sectionDeg = 360 / subdivisions;
	subdivisions++;

	vector pts[51];
	for (int i = 0; i < subdivisions; i++)
	{
		vector pt = vector.FromYaw(sectionDeg * i) * radius;
		pt = pt.Multiply3(mat);
		pts[i] = pt + pos;
	}
	pts[subdivisions - 1] = pts[0];

	return Shape.CreateLines(color, flags, pts, subdivisions);
}

//------------------------------------------------------------------------------------------------
Shape CreateCircleArc(vector pos, vector aroundDir, vector forwardDir, float angMin, float angMax, float radius, int color, int subdivisions, ShapeFlags flags)
{
	if (subdivisions < 2)
		subdivisions = 2;

	if (subdivisions > 50)
		subdivisions = 50;

	if (angMin > angMax)
	{
		float angTmp = angMin;
		angMin = angMax;
		angMax = angTmp;
	}

	if (angMin > 360 || angMax > 360)
		return CreateCircle(pos, aroundDir, radius, color, subdivisions, flags);

	vector rt = aroundDir * forwardDir;
	rt.Normalize();

	vector mat[3];
	mat[0] = rt;
	mat[1] = aroundDir;
	mat[2] = forwardDir;

	float sectionDeg = Math.AbsFloat(angMax - angMin) / subdivisions;

	subdivisions++;

	vector pts[51];
	for (int i = 0; i < subdivisions; i++)
	{
		vector pt = vector.FromYaw(sectionDeg * i + angMin) * radius;
		pt = pt.Multiply3(mat);
		pts[i] = pos + pt;
	}

	return Shape.CreateLines(color, flags, pts, subdivisions);
}

//------------------------------------------------------------------------------------------------
//! if angMin > angMax, a full circle will be drawn - use forwardDir to draw e.g 350deg-10deg arc
//! \param angMin min angle in degrees
//! \param angMax max angle in degrees
//! \param radius in metres
Shape CreateCircleSlice(vector pos, vector aroundDir, vector forwardDir, float angMin, float angMax, float radius, int color, int subdivisions, ShapeFlags flags)
{
	if (subdivisions < 2)
		subdivisions = 2;
	if (subdivisions > 50)
		subdivisions = 50;
	if (angMin > angMax) // Min greater than max, do full circle
		return CreateCircle(pos, aroundDir, radius, color, subdivisions, flags);

	subdivisions++;

	vector rt = aroundDir * forwardDir;
	rt.Normalize();

	vector mat[3];
	mat[0] = rt;
	mat[1] = aroundDir;
	mat[2] = forwardDir;

	float sectionDeg = Math.AbsFloat(angMax - angMin) / (subdivisions - 1);

	vector pts[53];
	for (int i = 0; i < subdivisions; i++)
	{
		int ptIndex = i + 1;
		vector pt = vector.FromYaw(sectionDeg * i + angMin) * radius;
		pt = pt.Multiply3(mat);
		pts[ptIndex] = pt + pos;
	}
	subdivisions += 2;
	pts[0] = pos;
	pts[subdivisions - 1] = pos;

	return Shape.CreateLines(color, flags, pts, subdivisions);
}

//------------------------------------------------------------------------------------------------
void CreateArrowLinkLines(vector from, vector to, vector faceDir, float size, int numArrows, int color, ShapeFlags flags)
{
	if (numArrows == 0)
		return;

	vector fw = to - from;
	fw.Normalize();
	vector rt = faceDir * fw;
	rt.Normalize();
	vector up = fw * rt;
	up.Normalize();

	vector scaledMat[3];
	scaledMat[0] = rt;
	scaledMat[1] = up;
	scaledMat[2] = fw;

	Shape shp;

	vector pts[3];

	float distInc = vector.Distance(from, to) / numArrows;
	for (int i = 0; i < numArrows; i++)
	{

		vector pos = fw * (distInc * i) + from;
		pts[0] = Vector(-1, 0, 0).Multiply3(scaledMat) * size + pos;
		pts[1] = Vector(0, 0, 0.5).Multiply3(scaledMat) * size + pos;
		pts[2] = Vector(1, 0, 0).Multiply3(scaledMat) * size + pos;
		shp = Shape.CreateLines(color, flags, pts, 3);
	}
}

//------------------------------------------------------------------------------------------------
void CreateSimpleText(string text, vector mat[4], float size, int color, ShapeFlags flags, array<ref Shape> output = null, float scaleWidth = 1, bool doBackground = false, int backgroundColor = 0x80000000)
{
	vector scaledMat[4];
	scaledMat[0] = mat[0] * size * scaleWidth;
	scaledMat[1] = mat[1] * size;
	scaledMat[2] = mat[2];
	scaledMat[3] = mat[3];

	vector pts[14];

	int numLetters = text.Length();
	float offsetStartLeft = numLetters * -0.5;
	Shape shp;

	// Add background
	if (doBackground)
	{
		ShapeFlags bgFlags = ShapeFlags.TRANSP | ShapeFlags.NOOUTLINE | ShapeFlags.NOZWRITE;
		if (flags & ShapeFlags.ONCE)
			bgFlags |= ShapeFlags.ONCE;
		if (flags & ShapeFlags.NOZBUFFER)
			bgFlags |= ShapeFlags.NOZBUFFER;

		shp = Shape.Create(ShapeType.BBOX, backgroundColor, bgFlags, Vector(offsetStartLeft - 0.25, -0.25, 0.01), Vector(-offsetStartLeft + 0.25, 1.25, 0.01));
		shp.SetMatrix(scaledMat);
		if (output)
			output.Insert(shp);
	}

	for (int i = 0; i < numLetters; i++)
	{
		float start = i + offsetStartLeft + 0.1;
		string letter = text.Get(i);
		string letterUpper = letter;
		letterUpper.ToUpper();

		float caseScale;
		if (letterUpper == letter) // Is upper case
			caseScale = 1;
		else
			caseScale = 0.6;

		switch (letterUpper)
		{
			// Letters --------------------------------------------
			case "A":
			{
				pts[0][0] = start; pts[0][1] = 0;
				pts[1][0] = start + 0.3; pts[1][1] = caseScale * 0.9;
				pts[2][0] = start + 0.6; pts[2][1] = 0;
				pts[3][0] = start + 0.47; pts[3][1] = caseScale * 0.37;
				pts[4][0] = start + 0.13; pts[4][1] = caseScale * 0.37;
				shp = Shape.CreateLines(color, flags, pts, 5);
				shp.SetMatrix(scaledMat);
				if (output)
					output.Insert(shp);
				break;
			}
			case "B":
			{
				pts[0][0] = start + 0.1; pts[0][1] = 0;
				pts[1][0] = start + 0.1; pts[1][1] = caseScale * 0.9;
				pts[2][0] = start + 0.5; pts[2][1] = caseScale * 0.9;
				pts[3][0] = start + 0.6; pts[3][1] = caseScale * 0.8;
				pts[4][0] = start + 0.6; pts[4][1] = caseScale * 0.6;
				pts[5][0] = start + 0.5; pts[5][1] = caseScale * 0.5;
				pts[6][0] = start + 0.1; pts[6][1] = caseScale * 0.5;
				pts[7] = pts[5];
				pts[8][0] = start + 0.6; pts[8][1] = caseScale * 0.4;
				pts[9][0] = start + 0.6; pts[9][1] = caseScale * 0.1;
				pts[10][0] = start + 0.5; pts[10][1] = 0;
				pts[11][0] = start + 0.1; pts[11][1] = 0;
				shp = Shape.CreateLines(color, flags, pts, 12);
				shp.SetMatrix(scaledMat);
				if (output)
					output.Insert(shp);

				break;
			}
			case "C":
			{
				pts[0][0] = start + 0.6; pts[0][1] = caseScale * 0.2;
				pts[1][0] = start + 0.4; pts[1][1] = 0;
				pts[2][0] = start + 0.2; pts[2][1] = 0;
				pts[3][0] = start; pts[3][1] = caseScale * 0.2;
				pts[4][0] = start; pts[4][1] = caseScale * 0.7;
				pts[5][0] = start + 0.2; pts[5][1] = caseScale * 0.9;
				pts[6][0] = start + 0.4; pts[6][1] = caseScale * 0.9;
				pts[7][0] = start + 0.6; pts[7][1] = caseScale * 0.7;
				shp = Shape.CreateLines(color, flags, pts, 8);
				shp.SetMatrix(scaledMat);
				if (output)
					output.Insert(shp);

				break;
			}
			case "D":
			{
				pts[0][0] = start; pts[0][1] = caseScale * 0.9;
				pts[1][0] = start + 0.4; pts[1][1] = caseScale * 0.9;
				pts[2][0] = start + 0.6; pts[2][1] = caseScale * 0.7;
				pts[3][0] = start + 0.6; pts[3][1] = caseScale * 0.2;
				pts[4][0] = start + 0.4; pts[4][1] = 0;
				pts[5][0] = start; pts[5][1] = 0;
				pts[6][0] = start; pts[6][1] = caseScale * 0.9;
				shp = Shape.CreateLines(color, flags, pts, 7);
				shp.SetMatrix(scaledMat);
				if (output)
					output.Insert(shp);

				break;
			}
			case "E":
			{
				pts[0][0] = start + 0.5; pts[0][1] = 0;
				pts[1][0] = start; pts[1][1] = 0;
				pts[2][0] = start; pts[2][1] = caseScale * 0.4;
				pts[3][0] = start + 0.4; pts[3][1] = caseScale * 0.4;
				pts[4] = pts[2];
				pts[5][0] = start; pts[5][1] = caseScale * 0.9;
				pts[6][0] = start + 0.5; pts[6][1] = caseScale * 0.9;
				shp = Shape.CreateLines(color, flags, pts, 7);
				shp.SetMatrix(scaledMat);
				if (output)
					output.Insert(shp);
				break;
			}
			case "F":
			{
				pts[0] = Vector(start, 0, 0);
				pts[1] = Vector(start, caseScale * 0.4, 0);
				pts[2] = Vector(start + 0.4, caseScale * 0.4, 0);
				pts[3] = pts[1];
				pts[4] = Vector(start, caseScale * 0.9, 0);
				pts[5] = Vector(start + 0.5, caseScale * 0.9, 0);
				shp = Shape.CreateLines(color, flags, pts, 6);
				shp.SetMatrix(scaledMat);
				if (output)
					output.Insert(shp);
				break;
			}
			case "G":
			{
				pts[0] = Vector(start + 0.4, caseScale * 0.4, 0);
				pts[1] = Vector(start + 0.6, caseScale * 0.4, 0);
				pts[2] = Vector(start + 0.6, caseScale * 0.2, 0);
				pts[3] = Vector(start + 0.4, 0, 0);
				pts[4] = Vector(start + 0.2, 0, 0);
				pts[5] = Vector(start, caseScale * 0.2, 0);
				pts[6] = Vector(start, caseScale * 0.7, 0);
				pts[7] = Vector(start + 0.2, caseScale * 0.9, 0);
				pts[8] = Vector(start + 0.4, caseScale * 0.9, 0);
				pts[9] = Vector(start + 0.6, caseScale * 0.7, 0);
				shp = Shape.CreateLines(color, flags, pts, 10);
				shp.SetMatrix(scaledMat);
				if (output)
					output.Insert(shp);

				break;
			}
			case "H":
			{
				pts[0] = Vector(start, 0, 0);
				pts[1] = Vector(start, caseScale * 0.9, 0);
				pts[2] = Vector(start, caseScale * 0.4, 0);
				pts[3] = Vector(start + 0.6, caseScale * 0.4, 0);
				pts[4] = Vector(start + 0.6, caseScale * 0.9, 0);
				pts[5] = Vector(start + 0.6, 0, 0);

				shp = Shape.CreateLines(color, flags, pts, 6);
				shp.SetMatrix(scaledMat);
				if (output)
					output.Insert(shp);
				break;
			}
			case "I":
			{
				pts[0][0] = start + 0.1; pts[0][1] = caseScale * 0.9;
				pts[1][0] = start + 0.5; pts[1][1] = caseScale * 0.9;
				pts[2][0] = start + 0.3; pts[2][1] = caseScale * 0.9;
				pts[3][0] = start + 0.3; pts[3][1] = 0;
				pts[4][0] = start + 0.5; pts[4][1] = 0;
				pts[5][0] = start + 0.1; pts[5][1] = 0;
				shp = Shape.CreateLines(color, flags, pts, 6);
				shp.SetMatrix(scaledMat);
				if (output)
					output.Insert(shp);
				break;
			}
			case "J":
			{
				pts[0] = Vector(start, caseScale * 0.9, 0);
				pts[1] = Vector(start + 0.6, caseScale * 0.9, 0);
				pts[2] = Vector(start + 0.3, caseScale * 0.9, 0);
				pts[3] = Vector(start + 0.3, caseScale * 0.1, 0);
				pts[4] = Vector(start + 0.2, 0, 0);
				pts[5] = Vector(start + 0.1, 0, 0);
				pts[6] = Vector(start, caseScale * 0.1, 0);
				pts[7] = Vector(start, caseScale * 0.3, 0);

				shp = Shape.CreateLines(color, flags, pts, 8);
				shp.SetMatrix(scaledMat);
				if (output)
					output.Insert(shp);
				break;
			}
			case "K":
			{
				pts[0] = Vector(start, 0, 0);
				pts[1] = Vector(start, caseScale * 0.9, 0);
				pts[2] = Vector(start, caseScale * 0.5, 0);
				pts[3] = Vector(start + 0.2, caseScale * 0.5, 0);
				pts[4] = Vector(start + 0.6, 0, 0);
				pts[5] = pts[3];
				pts[6] = Vector(start + 0.6, caseScale * 0.9, 0);
				shp = Shape.CreateLines(color, flags, pts, 7);
				shp.SetMatrix(scaledMat);
				if (output)
					output.Insert(shp);
				break;
			}
			case "L":
			{
				pts[0] = Vector(start, caseScale * 0.9, 0);
				pts[1] = Vector(start, 0, 0);
				pts[2] = Vector(start + 0.6, 0, 0);
				shp = Shape.CreateLines(color, flags, pts, 3);
				shp.SetMatrix(scaledMat);
				if (output)
					output.Insert(shp);

				break;
			}
			case "M":
			{
				pts[0] = Vector(start, 0, 0);
				pts[1] = Vector(start, caseScale * 0.9, 0);
				pts[2] = Vector(start + 0.3, caseScale * 0.5, 0);
				pts[3] = Vector(start + 0.6, caseScale * 0.9, 0);
				pts[4] = Vector(start + 0.6, 0, 0);
				shp = Shape.CreateLines(color, flags, pts, 5);
				shp.SetMatrix(scaledMat);
				if (output)
					output.Insert(shp);

				break;
			}
			case "N":
			{
				pts[0] = Vector(start, 0, 0);
				pts[1] = Vector(start, caseScale * 0.9, 0);
				pts[2] = Vector(start + 0.6, 0, 0);
				pts[3] = Vector(start + 0.6, caseScale * 0.9, 0);
				shp = Shape.CreateLines(color, flags, pts, 4);
				shp.SetMatrix(scaledMat);
				if (output)
					output.Insert(shp);

				break;
			}
			case "O":
			{
				pts[0][0] = start + 0.6; pts[0][1] = caseScale * 0.2;
				pts[1][0] = start + 0.4; pts[1][1] = 0;
				pts[2][0] = start + 0.2; pts[2][1] = 0;
				pts[3][0] = start; pts[3][1] = caseScale * 0.2;
				pts[4][0] = start; pts[4][1] = caseScale * 0.7;
				pts[5][0] = start + 0.2; pts[5][1] = caseScale * 0.9;
				pts[6][0] = start + 0.4; pts[6][1] = caseScale * 0.9;
				pts[7][0] = start + 0.6; pts[7][1] = caseScale * 0.7;
				pts[8] = pts[0];
				shp = Shape.CreateLines(color, flags, pts, 9);
				shp.SetMatrix(scaledMat);
				if (output)
					output.Insert(shp);

				break;
			}
			case "P":
			{
				pts[0] = Vector(start, 0, 0);
				pts[1] = Vector(start, caseScale * 0.5, 0);
				pts[2] = Vector(start, caseScale * 0.9, 0);
				pts[3] = Vector(start + 0.4, caseScale * 0.9, 0);
				pts[4] = Vector(start + 0.5, caseScale * 0.8, 0);
				pts[5] = Vector(start + 0.5, caseScale * 0.6, 0);
				pts[6] = Vector(start + 0.4, caseScale * 0.5, 0);
				pts[7] = Vector(start, caseScale * 0.5, 0);
				shp = Shape.CreateLines(color, flags, pts, 8);
				shp.SetMatrix(scaledMat);
				if (output)
					output.Insert(shp);

				break;
			}
			case "Q":
			{
				pts[0] = Vector(start + 0.6, caseScale * 0.2, 0);
				pts[1] = Vector(start + 0.4, 0, 0);
				pts[2] = Vector(start + 0.2, 0, 0);
				pts[3] = Vector(start, caseScale * 0.2, 0);
				pts[4] = Vector(start, caseScale * 0.7, 0);
				pts[5] = Vector(start + 0.2, caseScale * 0.9, 0);
				pts[6] = Vector(start + 0.4, caseScale * 0.9, 0);
				pts[7] = Vector(start + 0.6, caseScale * 0.7, 0);
				pts[8] = pts[0];
				shp = Shape.CreateLines(color, flags, pts, 9);
				shp.SetMatrix(scaledMat);
				if (output)
					output.Insert(shp);

				pts[0] = Vector(start + 0.4, caseScale * 0.2, 0);
				pts[1] = Vector(start + 0.6, 0, 0);
				shp = Shape.CreateLines(color, flags, pts, 2);
				shp.SetMatrix(scaledMat);
				if (output)
					output.Insert(shp);

				break;
			}
			case "R":
			{
				pts[0] = Vector(start, 0, 0);
				pts[1] = Vector(start, caseScale * 0.5, 0);
				pts[2] = Vector(start, caseScale * 0.9, 0);
				pts[3] = Vector(start + 0.4, caseScale * 0.9, 0);
				pts[4] = Vector(start + 0.5, caseScale * 0.8, 0);
				pts[5] = Vector(start + 0.5, caseScale * 0.6, 0);
				pts[6] = Vector(start + 0.4, caseScale * 0.5, 0);
				pts[7] = Vector(start + 0.2, caseScale * 0.5, 0);
				pts[8] = pts[1];
				shp = Shape.CreateLines(color, flags, pts, 9);
				shp.SetMatrix(scaledMat);
				if (output)
					output.Insert(shp);

				pts[0] = Vector(start + 0.2, caseScale * 0.5, 0);
				pts[1] = Vector(start + 0.5, 0, 0);
				shp = Shape.CreateLines(color, flags, pts, 2);
				shp.SetMatrix(scaledMat);
				if (output)
					output.Insert(shp);

				break;
			}
			case "S":
			{
				pts[0] = Vector(start + 0.5, caseScale * 0.8, 0);
				pts[1] = Vector(start + 0.4, caseScale * 0.9, 0);
				pts[2] = Vector(start + 0.1, caseScale * 0.9, 0);
				pts[3] = Vector(start, caseScale * 0.8, 0);
				pts[4] = Vector(start, caseScale * 0.6, 0);
				pts[5] = Vector(start + 0.1, caseScale * 0.5, 0);
				pts[6] = Vector(start + 0.4, caseScale * 0.4, 0);
				pts[7] = Vector(start + 0.5, caseScale * 0.3, 0);
				pts[8] = Vector(start + 0.5, caseScale * 0.1, 0);
				pts[9] = Vector(start + 0.4, 0, 0);
				pts[10] = Vector(start + 0.1, 0, 0);
				pts[11] = Vector(start, caseScale * 0.1, 0);
				shp = Shape.CreateLines(color, flags, pts, 12);
				shp.SetMatrix(scaledMat);
				if (output)
					output.Insert(shp);

				break;
			}
			case "T":
			{
				pts[0] = Vector(start, caseScale * 0.9, 0);
				pts[1] = Vector(start + 0.6, caseScale * 0.9, 0);
				pts[2] = Vector(start + 0.3, caseScale * 0.9, 0);
				pts[3] = Vector(start + 0.3, 0, 0);
				shp = Shape.CreateLines(color, flags, pts, 4);
				shp.SetMatrix(scaledMat);
				if (output)
					output.Insert(shp);
				break;
			}
			case "U":
			{
				pts[0] = Vector(start, caseScale * 0.9, 0);
				pts[1] = Vector(start, caseScale * 0.2, 0);
				pts[2] = Vector(start + 0.2, 0, 0);
				pts[3] = Vector(start + 0.4, 0, 0);
				pts[4] = Vector(start + 0.6, caseScale * 0.2, 0);
				pts[5] = Vector(start + 0.6, caseScale * 0.9, 0);
				shp = Shape.CreateLines(color, flags, pts, 6);
				shp.SetMatrix(scaledMat);
				if (output)
					output.Insert(shp);

				break;
			}
			case "V":
			{
				pts[0] = Vector(start, caseScale * 0.9, 0);
				pts[1] = Vector(start + 0.3, 0, 0);
				pts[2] = Vector(start + 0.6, caseScale * 0.9, 0);
				shp = Shape.CreateLines(color, flags, pts, 3);
				shp.SetMatrix(scaledMat);
				if (output)
					output.Insert(shp);

				break;
			}
			case "W":
			{
				pts[0] = Vector(start, caseScale * 0.9, 0);
				pts[1] = Vector(start + 0.1, 0, 0);
				pts[2] = Vector(start + 0.3, caseScale * 0.4, 0);
				pts[3] = Vector(start + 0.5, 0, 0);
				pts[4] = Vector(start + 0.6, caseScale * 0.9, 0);
				shp = Shape.CreateLines(color, flags, pts, 5);
				shp.SetMatrix(scaledMat);
				if (output)
					output.Insert(shp);

				break;
			}
			case "X":
			{
				pts[0] = Vector(start, caseScale * 0.9, 0);
				pts[1] = Vector(start + 0.3, caseScale * 0.45, 0);
				pts[2] = Vector(start + 0.6, caseScale * 0.9, 0);
				pts[3] = pts[1];
				pts[4] = Vector(start, 0, 0);
				pts[5] = pts[3];
				pts[6] = Vector(start + 0.6, 0, 0);
				shp = Shape.CreateLines(color, flags, pts, 7);
				shp.SetMatrix(scaledMat);
				if (output)
					output.Insert(shp);
				break;
			}
			case "Y":
			{
				pts[0] = Vector(start, caseScale * 0.9, 0);
				pts[1] = Vector(start + 0.3, caseScale * 0.5, 0);
				pts[2] = Vector(start + 0.6, caseScale * 0.9, 0);
				pts[3] = pts[1];
				pts[4] = Vector(start + 0.3, 0, 0);
				shp = Shape.CreateLines(color, flags, pts, 5);
				shp.SetMatrix(scaledMat);
				if (output)
					output.Insert(shp);
				break;
			}
			case "Z":
			{
				pts[0] = Vector(start, caseScale * 0.9, 0);
				pts[1] = Vector(start + 0.6, caseScale * 0.9, 0);
				pts[2] = Vector(start, 0, 0);
				pts[3] = Vector(start + 0.6, 0, 0);
				shp = Shape.CreateLines(color, flags, pts, 4);
				shp.SetMatrix(scaledMat);
				if (output)
					output.Insert(shp);

				break;
			}

			// Numbers --------------------------------------------
			case "0":
			{
				pts[0][0] = start + 0.5; pts[0][1] = 0.7;
				pts[1][0] = start + 0.4; pts[1][1] = 0.9;
				pts[2][0] = start + 0.2; pts[2][1] = 0.9;
				pts[3][0] = start + 0.1; pts[3][1] = 0.7;
				pts[4][0] = start + 0.1; pts[4][1] = 0.2;
				pts[5][0] = start + 0.2; pts[5][1] = 0;
				pts[6][0] = start + 0.4; pts[6][1] = 0;
				pts[7][0] = start + 0.5; pts[7][1] = 0.2;
				pts[8][0] = start + 0.5; pts[8][1] = 0.7;
				pts[9] = pts[4];
				shp = Shape.CreateLines(color, flags, pts, 10);
				shp.SetMatrix(scaledMat);
				if (output)
					output.Insert(shp);

				break;
			}
			case "1":
			{
				pts[0] = Vector(start + 0.1, 0.7, 0);
				pts[1] = Vector(start + 0.3, 0.9, 0);
				pts[2] = Vector(start + 0.3, 0, 0);
				pts[3] = Vector(start + 0.1, 0, 0);
				pts[4] = Vector(start + 0.5, 0, 0);
				shp = Shape.CreateLines(color, flags, pts, 5);
				shp.SetMatrix(scaledMat);
				if (output)
					output.Insert(shp);
				break;
			}
			case "2":
			{
				pts[0] = Vector(start + 0.1, 0.7, 0);
				pts[1] = Vector(start + 0.2, 0.9, 0);
				pts[2] = Vector(start + 0.4, 0.9, 0);
				pts[3] = Vector(start + 0.5, 0.7, 0);
				pts[4] = Vector(start + 0.1, 0, 0);
				pts[5] = Vector(start + 0.5, 0, 0);
				shp = Shape.CreateLines(color, flags, pts, 6);
				shp.SetMatrix(scaledMat);
				if (output)
					output.Insert(shp);

				break;
			}
			case "3":
			{
				pts[0] = Vector(start + 0.1, 0.7, 0);
				pts[1] = Vector(start + 0.2, 0.9, 0);
				pts[2] = Vector(start + 0.4, 0.9, 0);
				pts[3] = Vector(start + 0.5, 0.7, 0);
				pts[4] = Vector(start + 0.4, 0.45, 0);
				pts[5] = Vector(start + 0.3, 0.45, 0);
				shp = Shape.CreateLines(color, flags, pts, 6);
				shp.SetMatrix(scaledMat);
				if (output)
					output.Insert(shp);

				pts[0] = Vector(start + 0.4, 0.45, 0);
				pts[1] = Vector(start + 0.5, 0.2, 0);
				pts[2] = Vector(start + 0.4, 0, 0);
				pts[3] = Vector(start + 0.2, 0, 0);
				pts[4] = Vector(start + 0.1, 0.2, 0);
				shp = Shape.CreateLines(color, flags, pts, 5);
				shp.SetMatrix(scaledMat);
				if (output)
					output.Insert(shp);

				break;
			}
			case "4":
			{
				pts[0] = Vector(start + 0.4, 0, 0);
				pts[1] = Vector(start + 0.4, 0.4, 0);
				pts[2] = Vector(start + 0.4, 0.9, 0);
				pts[3] = Vector(start + 0.1, 0.4, 0);
				pts[4] = Vector(start + 0.4, 0.4, 0);
				pts[5] = Vector(start + 0.5, 0.4, 0);
				shp = Shape.CreateLines(color, flags, pts, 6);
				shp.SetMatrix(scaledMat);
				if (output)
					output.Insert(shp);

				break;
			}
			case "5":
			{
				pts[0] = Vector(start + 0.5, 0.9, 0);
				pts[1] = Vector(start + 0.1, 0.9, 0);
				pts[2] = Vector(start + 0.1, 0.5, 0);
				pts[3] = Vector(start + 0.4, 0.5, 0);
				pts[4] = Vector(start + 0.5, 0.3, 0);
				pts[5] = Vector(start + 0.5, 0.2, 0);
				pts[6] = Vector(start + 0.4, 0, 0);
				pts[7] = Vector(start + 0.2, 0, 0);
				pts[8] = Vector(start + 0.1, 0.2, 0);
				shp = Shape.CreateLines(color, flags, pts, 9);
				shp.SetMatrix(scaledMat);
				if (output)
					output.Insert(shp);

				break;
			}
			case "6":
			{
				pts[0] = Vector(start + 0.4, 0.9, 0);
				pts[1] = Vector(start + 0.2, 0.9, 0);
				pts[2] = Vector(start + 0.1, 0.7, 0);
				pts[3] = Vector(start + 0.1, 0.3, 0);
				pts[4] = Vector(start + 0.1, 0.2, 0);
				pts[5] = Vector(start + 0.2, 0, 0);
				pts[6] = Vector(start + 0.4, 0, 0);
				pts[7] = Vector(start + 0.5, 0.2, 0);
				pts[8] = Vector(start + 0.5, 0.3, 0);
				pts[9] = Vector(start + 0.4, 0.5, 0);
				pts[10] = Vector(start + 0.2, 0.5, 0);
				pts[11] = pts[3];
				shp = Shape.CreateLines(color, flags, pts, 12);
				shp.SetMatrix(scaledMat);
				if (output)
					output.Insert(shp);

				break;
			}
			case "7":
			{
				pts[0] = Vector(start + 0.1, 0.9, 0);
				pts[1] = Vector(start + 0.5, 0.9, 0);
				pts[2] = Vector(start + 0.3, 0, 0);
				shp = Shape.CreateLines(color, flags, pts, 3);
				shp.SetMatrix(scaledMat);
				if (output)
					output.Insert(shp);

				break;
			}
			case "8":
			{
				pts[0] = Vector(start + 0.4, 0.9, 0);
				pts[1] = Vector(start + 0.2, 0.9, 0);
				pts[2] = Vector(start + 0.1, 0.7, 0);
				pts[3] = Vector(start + 0.2, 0.5, 0);
				pts[4] = Vector(start + 0.4, 0.5, 0);
				pts[5] = Vector(start + 0.5, 0.7, 0);
				pts[6] = pts[0];
				shp = Shape.CreateLines(color, flags, pts, 7);
				shp.SetMatrix(scaledMat);
				if (output)
					output.Insert(shp);

				pts[0] = Vector(start + 0.2, 0.5, 0);
				pts[1] = Vector(start + 0.1, 0.3, 0);
				pts[2] = Vector(start + 0.1, 0.2, 0);
				pts[3] = Vector(start + 0.2, 0, 0);
				pts[4] = Vector(start + 0.4, 0, 0);
				pts[5] = Vector(start + 0.5, 0.2, 0);
				pts[6] = Vector(start + 0.5, 0.3, 0);
				pts[7] = Vector(start + 0.4, 0.5, 0);
				shp = Shape.CreateLines(color, flags, pts, 8);
				shp.SetMatrix(scaledMat);
				if (output)
					output.Insert(shp);

				break;
			}
			case "9":
			{
				pts[0] = Vector(start + 0.5, 0.6, 0);
				pts[1] = Vector(start + 0.4, 0.4, 0);
				pts[2] = Vector(start + 0.2, 0.4, 0);
				pts[3] = Vector(start + 0.1, 0.6, 0);
				pts[4] = Vector(start + 0.1, 0.7, 0);
				pts[5] = Vector(start + 0.2, 0.9, 0);
				pts[6] = Vector(start + 0.4, 0.9, 0);
				pts[7] = Vector(start + 0.5, 0.7, 0);
				pts[8] = Vector(start + 0.5, 0.6, 0);
				pts[9] = Vector(start + 0.5, 0.4, 0);
				pts[10] = Vector(start + 0.5, 0.2, 0);
				pts[11] = Vector(start + 0.4, 0, 0);
				pts[12] = Vector(start + 0.2, 0, 0);
				pts[13] = Vector(start + 0.1, 0.2, 0);
				shp = Shape.CreateLines(color, flags, pts, 14);
				shp.SetMatrix(scaledMat);
				if (output)
					output.Insert(shp);

				break;
			}

			// Special --------------------------------------------
			case "_":
			{
				pts[0] = Vector(start, 0.1, 0);
				pts[1] = Vector(start + 0.6, 0.1, 0);
				shp = Shape.CreateLines(color, flags, pts, 2);
				shp.SetMatrix(scaledMat);
				if (output)
					output.Insert(shp);

				break;
			}
			case "-":
			{
				pts[0] = Vector(start + 0.1, 0.45, 0);
				pts[1] = Vector(start + 0.5, 0.45, 0);
				shp = Shape.CreateLines(color, flags, pts, 2);
				shp.SetMatrix(scaledMat);
				if (output)
					output.Insert(shp);

				break;
			}
			case "+":
			{
				pts[0] = Vector(start + 0.1, 0.45, 0);
				pts[1] = Vector(start + 0.5, 0.45, 0);
				pts[2] = Vector(start + 0.3, 0.45, 0);
				pts[3] = Vector(start + 0.3, 0.25, 0);
				pts[4] = Vector(start + 0.3, 0.65, 0);
				shp = Shape.CreateLines(color, flags, pts, 5);
				shp.SetMatrix(scaledMat);
				if (output)
					output.Insert(shp);
				break;
			}
			case ".":
			{
				pts[0] = Vector(start + 0.3, 0, 0);
				pts[1] = Vector(start + 0.3, 0.05, 0);
				pts[2] = Vector(start + 0.25, 0.05, 0);
				pts[3] = Vector(start + 0.25, 0, 0);
				pts[4] = pts[0];
				shp = Shape.CreateLines(color, flags, pts, 5);
				shp.SetMatrix(scaledMat);
				if (output)
					output.Insert(shp);

				break;
			}
			case ":":
			{
				pts[0] = Vector(start + 0.3, 0.1, 0);
				pts[1] = Vector(start + 0.3, 0.15, 0);
				pts[2] = Vector(start + 0.25, 0.15, 0);
				pts[3] = Vector(start + 0.25, 0.1, 0);
				pts[4] = pts[0];
				shp = Shape.CreateLines(color, flags, pts, 5);
				shp.SetMatrix(scaledMat);
				if (output)
					output.Insert(shp);

				pts[0] = Vector(start + 0.3, 0.6, 0);
				pts[1] = Vector(start + 0.3, 0.65, 0);
				pts[2] = Vector(start + 0.25, 0.65, 0);
				pts[3] = Vector(start + 0.25, 0.6, 0);
				pts[4] = pts[0];
				shp = Shape.CreateLines(color, flags, pts, 5);
				shp.SetMatrix(scaledMat);
				if (output)
					output.Insert(shp);

				break;
			}
			case "!":
			{
				pts[0] = Vector(start + 0.3, 0.3, 0);
				pts[1] = Vector(start + 0.3, 0.9, 0);
				pts[2] = Vector(start + 0.25, 0.9, 0);
				pts[3] = Vector(start + 0.25, 0.3, 0);
				pts[4] = pts[0];
				shp = Shape.CreateLines(color, flags, pts, 5);
				shp.SetMatrix(scaledMat);
				if (output)
					output.Insert(shp);

				pts[0] = Vector(start + 0.3, 0, 0);
				pts[1] = Vector(start + 0.3, 0.05, 0);
				pts[2] = Vector(start + 0.25, 0.05, 0);
				pts[3] = Vector(start + 0.25, 0, 0);
				pts[4] = pts[0];
				shp = Shape.CreateLines(color, flags, pts, 5);
				shp.SetMatrix(scaledMat);
				if (output)
					output.Insert(shp);

				break;
			}
			case ",":
			{
				pts[0] = Vector(start + 0.3, 0, 0);
				pts[1] = Vector(start + 0.2, 0, 0);
				pts[2] = Vector(start + 0.1, -0.2, 0);
				pts[3] = Vector(start + 0.2, -0.2, 0);
				pts[4] = pts[0];
				shp = Shape.CreateLines(color, flags, pts, 5);
				shp.SetMatrix(scaledMat);
				if (output)
					output.Insert(shp);

				break;
			}
			case "'":
			{
				pts[0] = Vector(start + 0.4, 0.9, 0);
				pts[1] = Vector(start + 0.25, 0.9, 0);
				pts[2] = Vector(start + 0.3, 0.6, 0);
				pts[3] = Vector(start + 0.35, 0.6, 0);
				pts[4] = pts[0];
				shp = Shape.CreateLines(color, flags, pts, 5);
				shp.SetMatrix(scaledMat);
				if (output)
					output.Insert(shp);

				break;
			}
			case "/":
			{
				pts[0] = Vector(start + 0.54, 0.9, 0);
				pts[1] = Vector(start + 0.25, 0, 0);
				shp = Shape.CreateLines(color, flags, pts, 2);
				shp.SetMatrix(scaledMat);
				if (output)
					output.Insert(shp);

				break;
			}
			case "%":
			{
				pts[0] = Vector(start + 0.55, 0.9, 0);
				pts[1] = Vector(start + 0.25, 0, 0);
				shp = Shape.CreateLines(color, flags, pts, 2);
				shp.SetMatrix(scaledMat);
				if (output)
					output.Insert(shp);

				pts[0] = Vector(start + 0.2, 0.5, 0);
				pts[1] = Vector(start + 0.05, 0.7, 0);
				pts[2] = Vector(start + 0.2, 0.9, 0);
				pts[3] = Vector(start + 0.35, 0.7, 0);
				pts[4] = pts[0];
				shp = Shape.CreateLines(color, flags, pts, 5);
				shp.SetMatrix(scaledMat);
				if (output)
					output.Insert(shp);

				pts[0] = Vector(start + 0.6, 0.1, 0);
				pts[1] = Vector(start + 0.45, 0.3, 0);
				pts[2] = Vector(start + 0.6, 0.5, 0);
				pts[3] = Vector(start + 0.75, 0.3, 0);
				pts[4] = pts[0];
				shp = Shape.CreateLines(color, flags, pts, 5);
				shp.SetMatrix(scaledMat);
				if (output)
					output.Insert(shp);

				break;
			}
		}
	}

	/*
	vector pts[53];
	for (int i = 0; i < subdivisions; i++)
	{
		int ptIndex = i + 1;
		vector pt = vector.YawToVector(sectionDeg * i + angMin) * radius;
		pt = Vector(-pt[0], 0, pt[1]); // Convert from Enforce to Enfusion format
		pt = pt.Multiply3(mat);
		pts[ptIndex] = pt + pos;
	}
	subdivisions += 2;
	pts[0] = pos;
	pts[subdivisions - 1] = pos;

	return Shape.CreateLines(color, flags, pts, subdivisions); */
}
