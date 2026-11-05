class SCR_AIGetRandomCircumferencePosition : AITaskScripted
{
	static const string PORT_ENTITY = "EntityIn";
	static const string PORT_ORIGIN = "OriginIn";
	static const string PORT_RADIUS = "RadiusIn";
	static const string PORT_POSITION = "PositionOut";
	
	[Attribute("1", UIWidgets.CheckBox, "Snap resulting position to terrain surface?")]
	protected bool m_bSnapToSurface;
	
	[Attribute("5", UIWidgets.Auto, "Radius used when input radius is not provided or is below minimum.")]
	protected float m_fRadiusFallback;
	
	[Attribute("25", UIWidgets.Auto, "Maximum radius of the circle.")]
	protected float m_fRadiusMax;
	
	[Attribute("4", UIWidgets.Auto, "Minimum radius of the circle.")]
	protected float m_fRadiusMin;
	
	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		vector origin;
		float radius;
		IEntity entity;
		GetVariableIn(PORT_ENTITY, entity);
		GetVariableIn(PORT_RADIUS, radius);
		
		if (radius < m_fRadiusMin)
			radius = m_fRadiusFallback;
		
		radius = Math.Clamp(radius, m_fRadiusMin, m_fRadiusMax);
		
		if (!entity)
		{
			if (!GetVariableIn(PORT_ORIGIN, origin))
				return NodeError(this, owner, "No valid circle provided!");
		}	
		else
		{
			origin = entity.GetOrigin();
		}
		
		float angle = Math.RandomFloat(0, Math.PI2);
		vector pos = origin + Vector(Math.Cos(angle) * radius, 0, Math.Sin(angle) * radius);
		
		if (m_bSnapToSurface)
		{
			BaseWorld world = GetGame().GetWorld();
			if (world)
			pos[1] = world.GetSurfaceY(pos[0], pos[2]);
		}
		
		SetVariableOut(PORT_POSITION, pos);
		return ENodeResult.SUCCESS;
	}
	
	//------------------------------------------------------------------------------------------------
	override static event protected bool VisibleInPalette() { return true; }	
	override static event protected string GetOnHoverDescription() { return "Returns random point on a circumference of a provided circle by entity as origin. If no entity provided, the circle is given by origin + radius. Radius is clamped.";}
	
	protected static ref TStringArray s_aVarsIn = {	PORT_ENTITY, PORT_RADIUS, PORT_ORIGIN};
	override TStringArray GetVariablesIn() { return s_aVarsIn; }
	
	protected static ref TStringArray s_aVarsOut = { PORT_POSITION };
	override TStringArray GetVariablesOut() { return s_aVarsOut; }
};