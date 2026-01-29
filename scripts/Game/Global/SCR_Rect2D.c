/*
Class for generic rectangle operations.
*/

class SCR_Rect2D
{
	/*
	  Y axis
	    ^ 
		|	x---------p1
		|	|         |
		|	|         |
		|	|         |
		|	p0--------x
		|
	  --X--------------------> X Axis
	    |
	*/
	
	vector p0; // lowest point
	vector p1; // highest point, p1[0] > p0[0], p1[1] > p0[1]
	
	
	//-------------------------------------------------------------------------------------
	static SCR_Rect2D FromPoints(vector lowerPoint, vector higherPoint)
	{
		SCR_Rect2D r = new SCR_Rect2D();
		r.p0 = lowerPoint;
		r.p1 = higherPoint;
		return r;
	}
	
	
	//-------------------------------------------------------------------------------------
	static SCR_Rect2D FromPosAndSize(vector pos, vector size)
	{
		SCR_Rect2D r = new SCR_Rect2D();
		
		r.p0[0] = pos[0];
		r.p0[1] = pos[1];
		r.p1[0] = pos[0] + size[0];
		r.p1[1] = pos[1] + size[1];
		
		return r;
	}
	
	//-------------------------------------------------------------------------------------
	static SCR_Rect2D FromRect(SCR_Rect2D other)
	{
		SCR_Rect2D r = new SCR_Rect2D();
		
		r.p0 = other.p0;
		r.p1 = other.p1;
		
		return r;
	}
	
	
	//-------------------------------------------------------------------------------------
	// True when other Rect is fully inside this rect or it at its edge
	bool HasInside(SCR_Rect2D other)
	{
		return	(other.p0[0] >= p0[0]) &&
				(other.p0[1] >= p0[1]) &&
				(other.p1[0] <= p1[0]) &&
				(other.p1[1] <= p1[1]);
	}
	
	//-------------------------------------------------------------------------------------
	// True when a point is inside this rectangle or is at its edge
	bool HasInside(vector point)
	{
		float x = point[0];
		float y = point[1];
		return (x >= p0[0]) && (x <= p1[0]) && (y >= p0[1]) && (y <= p1[1]);
	}
	
	//-------------------------------------------------------------------------------------
	// True when this and the other rects overlap
	bool Overlaps(SCR_Rect2D other)
	{
		return	CheckIntervalIntersection(p0[0], p1[0], other.p0[0], other.p1[0]) &&
				CheckIntervalIntersection(p0[1], p1[1], other.p0[1], other.p1[1]);
	}
	
	
	//-------------------------------------------------------------------------------------
	// Returns pos of center of rectangle
	vector GetCenter()
	{
		vector v;
		
		v[0] = 0.5 * (p0[0] + p1[0]);
		v[1] = 0.5 * (p0[1] + p1[1]);
		
		return v;
	}
	
	//-------------------------------------------------------------------------------------
	float GetWidth()
	{
		return p1[0] - p0[0];
	}
	
	//-------------------------------------------------------------------------------------
	float GetHeight()
	{
		return p1[1] - p0[1];
	}	
	
	//-------------------------------------------------------------------------------------
	// True when intervals [aMin, aMax] and [bMin, bMax] intersect
	protected static bool CheckIntervalIntersection(float aMin, float aMax, float bMin, float bMax)
	{
		if ((bMin > aMax) || (bMax < aMin))
			return false;
		else
			return true;
	}
	
	
	//-------------------------------------------------------------------------------------
	//! This rect will be expanded in all directions by given value
	SCR_Rect2D ExpandAllDirections(float delta)
	{
		p0[0] = p0[0] - delta;
		p0[1] = p0[1] - delta;
		p1[0] = p1[0] + delta;
		p1[1] = p1[1] + delta;
		return this;
	}
	
};
