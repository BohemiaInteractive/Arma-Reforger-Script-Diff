[EntityEditorProps(category: "GameScripted/GameMode")]
class SCR_PositionClass : GenericEntityClass
{
}

class SCR_Position : GenericEntity
{
#ifdef WORKBENCH
	protected int m_iColor = Color.WHITE;
	protected string m_sText = string.Empty;

	//------------------------------------------------------------------------------------------------
	//!
	void SetColorAndText();

	//------------------------------------------------------------------------------------------------
	override int _WB_GetAfterWorldUpdateSpecs(IEntitySource src)
	{
		return EEntityFrameUpdateSpecs.CALL_WHEN_ENTITY_VISIBLE;
	}

	//------------------------------------------------------------------------------------------------
	override void _WB_AfterWorldUpdate(float timeSlice)
	{
		SetColorAndText();

		vector mat[4];
		GetWorldTransform(mat);

		// Draw point and arrow
		vector position = mat[3];
		Shape pointShape = Shape.CreateSphere(m_iColor, ShapeFlags.ONCE | ShapeFlags.NOOUTLINE, position, 0.2);
		Shape arrowShape = Shape.CreateArrow(position, position + mat[2], 0.2, m_iColor, ShapeFlags.ONCE);

		WorldEditorAPI api = _WB_GetEditorAPI();
		if (api)
		{
			IEntity selectEntity = api.SourceToEntity(api.GetSelectedEntity());
			if (!SCR_Position.Cast(selectEntity))
				return;
		}

		if (!m_sText.IsEmpty())
		{
			DebugTextWorldSpace textShape = DebugTextWorldSpace.Create(
				GetWorld(),
				m_sText,
				DebugTextFlags.ONCE | DebugTextFlags.CENTER | DebugTextFlags.FACE_CAMERA,
				position[0],
				position[1] + 0.4,
				position[2],
				20.0,
				m_iColor);
		}
	}
#endif
	
	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] src
	//! \param[in] parent
	void SCR_Position(IEntitySource src, IEntity parent)
	{
		SetEventMask(EntityEvent.INIT);
		SetFlags(EntityFlags.STATIC, true);
	}	
}
