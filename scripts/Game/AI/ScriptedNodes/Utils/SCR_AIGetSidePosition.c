class SCR_AIGetSidePosition: AITaskScripted
{
	static const string PORT_ENEMY			= "Enemy";
	static const string PORT_ENEMY_POSITION	= "EnemyPosition";
	static const string PORT_KEEP_SIDE		= "KeepSide";
	static const string PORT_POSITION		= "PositionOut";
	
	
	[Attribute("4", UIWidgets.EditBox, "Distance from origin")]
	private float m_fDistance;
	
	private int m_iSide = 0;
	private IEntity m_OwnerEntity;
	
	//------------------------------------------------------------------------------------------------
	override void OnEnter(AIAgent owner)
	{			
		if (GetVariableType(false, PORT_POSITION) != vector)
		{
			NodeError(this, owner, PORT_POSITION + " has to be vector");
		}
		m_OwnerEntity = owner.GetControlledEntity();
		if (!m_OwnerEntity)
		{
			NodeError(this, owner, "Owner must be a character!");
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		vector direction, enemyPos, origin = m_OwnerEntity.GetOrigin();
		IEntity enemy;
		bool keepSide;
		if (!GetVariableIn(PORT_KEEP_SIDE, keepSide) || !keepSide) // if parameter keepSide == true, it will not change side from previous run
			m_iSide = Math.RandomInt(0,2);
		// get direction of enemy
		if (GetVariableIn(PORT_ENEMY, enemy))
		{
			direction = vector.Direction(enemy.GetOrigin(), origin);			
			direction.Normalize();
		}		
		else if (GetVariableIn(PORT_ENEMY_POSITION, enemyPos))	
		{
			direction = vector.Direction(enemyPos, origin);
			direction.Normalize();
			
		}
		else
			direction = m_OwnerEntity.GetYawPitchRoll().AnglesToVector(); // gets normalized forward vector of m_OwnerEntity
		
		// make perpendicular vector 
		if (m_iSide == 0)
		{
			float x = direction[0];
			direction[0] = direction[2];
			direction[2] = -x;
		}
		else
		{
			float x = direction[0];
			direction[0] = -direction[2];
			direction[2] = x;
		}
		
		origin += direction * m_fDistance;
		
		SetVariableOut(PORT_POSITION, origin);

		return ENodeResult.SUCCESS;
	}
	
	//------------------------------------------------------------------------------------------------
	static override bool VisibleInPalette()
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	static override protected string GetOnHoverDescription()
	{
		return "Returns position from owner entity that is either on random side of the direction towards enemy pos or the keeps the same side as previously if KeepSide is true";
	}
	
	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsIn = {
		PORT_ENEMY,
		PORT_ENEMY_POSITION,
		PORT_KEEP_SIDE
	};
	override TStringArray GetVariablesIn()
	{
		return s_aVarsIn;
	}
	
	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsOut = {
		PORT_POSITION
	};
	override TStringArray GetVariablesOut()
	{
		return s_aVarsOut;
	}
};