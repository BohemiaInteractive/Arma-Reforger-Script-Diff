class SCR_AIMessageVisualization
{
	private const float RANGE_SQR = 500 * 500;
	IEntity m_TargetEntity;
	private string m_sMessage;
	private float m_fTimeout;
	private int m_Color;
	private float m_fSize;
	
	static const bool DRAW_TRANSPARENCY = false;
	static const float HEIGHT = 2;
	static const ref Color COLOR_BACKGROUND = Color.FromSRGBA(0, 0, 0, 64); //UIColors.TRANSPARENT;
	
	//------------------------------------------------------------------------------------------------
	void SCR_AIMessageVisualization(IEntity entity, string message, float showTime = 10, Color color = Color.White, float fontSize = 16)
	{
		m_TargetEntity = entity;
		m_sMessage = message;
		m_fTimeout = showTime;
		m_Color = color.PackToInt();
		m_fSize = fontSize;
	}
	
	// @TODO: Optimize: No need to get camera transform multiple times, that can be done just once
	//------------------------------------------------------------------------------------------------
	bool Draw(float timeSlice)
	{
		if (m_fTimeout <= 0 || m_sMessage == string.Empty || !m_TargetEntity)
			return true;
		
		m_fTimeout -= timeSlice;

		// Do not draw if camera is not looking at the owner
		vector mat[4];
		BaseWorld world = m_TargetEntity.GetWorld();
		world.GetCurrentCamera(mat);
		vector targetPos = m_TargetEntity.GetOrigin();
		float dot = vector.Dot(mat[2], (targetPos - mat[3]).Normalized());
		float distanceSqr = vector.DistanceSq(mat[3], targetPos);
		if (dot < 0 || distanceSqr > RANGE_SQR) // More than 90 deg to right/left
			return false;

		ShapeFlags flags = ShapeFlags.ONCE | ShapeFlags.TRANSP;
		if (DRAW_TRANSPARENCY)
			flags |= ShapeFlags.TRANSP;
		
		int dtFlags = DebugTextFlags.CENTER | DebugTextFlags.ONCE;
		vector origin = m_TargetEntity.GetOrigin();
		origin[1] = origin[1] + HEIGHT;
		vector pos = GetGame().GetWorkspace().ProjWorldToScreenNative(origin, world);
		
		DebugTextScreenSpace.Create(m_TargetEntity.GetWorld(), m_sMessage, dtFlags, pos[0], pos[1], m_fSize, m_Color, COLOR_BACKGROUND.PackToInt());
		return false;
	}
};