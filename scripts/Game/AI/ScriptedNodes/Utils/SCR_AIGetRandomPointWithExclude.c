class SCR_AIGetRandomPointWithExclude: AITaskScripted
{
	static const string ENTITY_PORT				= "Entity";
	static const string ORIGIN_PORT				= "SearchCenter";
	static const string RADIUS_PORT				= "SearchRadius";
	static const string EXCLUSION_ORIGIN_PORT	= "ExclusionCenter";
	static const string EXCLUSION_RADIUS_PORT	= "ExclusionRadius";
	static const string PORT_RESULT_VECTOR		= "Position";
	
	[Attribute("", UIWidgets.Coords, "Search Position Center", "", )]
	protected vector m_vSearchCenter;
	
	[Attribute("10", UIWidgets.EditBox, "Search Position Radius", "", )]
	protected float m_fSearchRadius;

	[Attribute("", UIWidgets.Coords, "Exclusion position center", "", )]
	protected vector m_vExcludeCenter;
	
	[Attribute("2", UIWidgets.EditBox, "Radius of exclusion area", "", )]
	protected float m_fExcludeRadius;
	
	[Attribute("10", UIWidgets.EditBox, "How many attempts of finding new position should be made", "", )]
	protected int m_iIterationCount;

	//------------------------------------------------------------------------------------------------
	static override bool VisibleInPalette() 
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsIn = {
		ENTITY_PORT,
		ORIGIN_PORT,
		RADIUS_PORT,
		EXCLUSION_ORIGIN_PORT,
		EXCLUSION_RADIUS_PORT
	};
    override array<string> GetVariablesIn()
    {
        return s_aVarsIn;
    }
    
	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsOut = {
		PORT_RESULT_VECTOR
	};
    override array<string> GetVariablesOut()
    {
		return s_aVarsOut;
    }

    //------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
    {
		vector exclusionPos, searchPos, result;
		IEntity entitySearch;
		float searchRadius, exclusionRadius;
		
		if (GetVariableIn(ENTITY_PORT,entitySearch))
		{
			searchPos = entitySearch.GetOrigin();
		}
		else if(!GetVariableIn(ORIGIN_PORT,searchPos))
		{
			IEntity ent = owner.GetControlledEntity();
			if (!ent)
			{
				searchPos = m_vSearchCenter;
			}
			searchPos = ent.GetOrigin();			
		}
		
		if (!GetVariableIn(EXCLUSION_ORIGIN_PORT,exclusionPos))
			exclusionPos = searchPos;
		if (!GetVariableIn(RADIUS_PORT,searchRadius))
			searchRadius = m_fSearchRadius;
		if (!GetVariableIn(EXCLUSION_RADIUS_PORT,exclusionRadius))
			exclusionRadius = m_fExcludeRadius;
		
		if (!FindPosition2D(result, searchPos, searchRadius, exclusionPos, exclusionRadius, m_iIterationCount))
			return ENodeResult.FAIL;
		
		SetVariableOut(PORT_RESULT_VECTOR,result);		
		return ENodeResult.SUCCESS;
    }	
	
	// returns position inside a circle that is not inside an exlusion circle, iterative
	//------------------------------------------------------------------------------------------------------------------------------------------------------------
	protected bool FindPosition2D(out vector randomPos, vector randomSphereOrigin, float randomSphereRadius, vector excludeSphereOrigin = vector.Zero, float excludeRadius = 0, int iterationCount = 50)
	{
		if (randomSphereOrigin == excludeSphereOrigin || excludeRadius < 1.0e-8)
		{
			randomPos = s_AIRandomGenerator.GenerateRandomPointInRadius(excludeRadius, randomSphereRadius, randomSphereOrigin, true);	
			randomPos[1] = randomSphereOrigin[1];
			return true;
		}	
		else
		{
			float excludeRadiusSq = excludeRadius * excludeRadius;
			float randomRadiusSq = randomSphereRadius * randomSphereRadius;	
			for (int i = iterationCount; i > 0; i--)
			{
				randomPos = s_AIRandomGenerator.GenerateRandomPointInRadius(0, randomSphereRadius, randomSphereOrigin, true);
				
				// Repeat if position is inside exclusion zone
				if (excludeRadius > 0 && vector.DistanceSqXZ(randomPos, excludeSphereOrigin) < excludeRadiusSq)
					continue;
			
				randomPos[1] = randomSphereOrigin[1];
				return true;
			}
		}
		return false;
	}
		
	//------------------------------------------------------------------------------------------------
	static override protected string GetOnHoverDescription()
	{
		return "Returns random position in circle that does not lie inside an exclusion circle. Center is taken from Entity variable or from SearchCenter variable";
	}
};