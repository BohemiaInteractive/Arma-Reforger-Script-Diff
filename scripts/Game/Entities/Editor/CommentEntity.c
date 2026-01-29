[EntityEditorProps(category: "GameScripted/Utility", description: "Editor comment, for leaving notes in the editor", sizeMin: "-1 -1 -1", sizeMax: "1 1 1", visible: false, dynamicBox: true)]
class CommentEntityClass : GenericEntityClass
{
};

class CommentEntity : GenericEntity
{
	[Attribute("New comment", UIWidgets.EditBoxMultiline, "Write any comment you wish")]
	string m_Comment;

	[Attribute("0.2", UIWidgets.EditBox, "Text size in meters")]
	float m_Size;

	[Attribute("1 1 1", UIWidgets.ColorPicker, "Text color")]
	vector m_Color;

	[Attribute("0", UIWidgets.Slider, "Text transparency", "0 1 0.05")]
	float m_Transparency;

	[Attribute("0", UIWidgets.CheckBox, "If true, text is scaled by distance from camera")]
	bool m_ScaleByDistance;

	[Attribute("0", UIWidgets.CheckBox, "If true, text will face the camera")]
	bool m_FaceCamera;

	[Attribute("0", UIWidgets.CheckBox, "If true, text will be visible over everything")]
	bool m_VisibleOverall;

	[Attribute("0", UIWidgets.CheckBox, "If true, text will be visible even ingame")]
	bool m_VisibleIngame;

	[Attribute("0", UIWidgets.CheckBox, "If true, text will have a darkened background plane for easier visibility")]
	bool m_TextBackground;

	[Attribute("0 0 0", UIWidgets.ColorPicker, "Background color")]
	vector m_BackgroundColor;

	[Attribute("0.5", UIWidgets.Slider, "Background transparency", "0 1 0.05")]
	float m_BackgroundTransparency;

	//------------------------------------------------------------------------------------------------
	private void DrawComment()
	{
		const float textWidthScale = 0.7;

		vector textMat[4];
		GetWorld().GetCurrentCamera(textMat);

		if (!m_FaceCamera)
		{
			textMat[0] = GetTransformAxis(0);
			textMat[1] = GetTransformAxis(1);
			textMat[2] = GetTransformAxis(2);
		}

		float distScale = 1;
		if (m_ScaleByDistance)
		{
			distScale = vector.Distance(textMat[3], GetOrigin()) * 0.1;
			distScale = Math.Clamp(distScale, 0.5, 10);
		}

		float textEndSize = (m_Size * distScale) / vector.Distance(textMat[3], GetOrigin());
		if (textEndSize < 0.005)
			return;

		textMat[3] = GetOrigin() - textMat[1] * m_Size * distScale * 0.5;
		ShapeFlags flags = ShapeFlags.ONCE | ShapeFlags.TRANSP;
		if (m_VisibleOverall)
			flags |= ShapeFlags.NOZBUFFER;
		if (m_Transparency > 0)
			flags |= ShapeFlags.TRANSP;

#ifdef WORKBENCH
		m_fWB_Width = m_Comment.Length() * m_Size * distScale * textWidthScale;
		m_fWB_Height = m_Size * distScale;
#endif

		//CreateSimpleText(m_Comment, textMat, m_Size * distScale, ARGBF(1 - m_Transparency, m_Color[0], m_Color[1], m_Color[2]), flags, null, textWidthScale, m_TextBackground, ARGBF(1 - m_BackgroundTransparency, m_BackgroundColor[0], m_BackgroundColor[1], m_BackgroundColor[2]));
		int dtFlags = DebugTextFlags.CENTER | DebugTextFlags.ONCE;
		if (m_FaceCamera)
			dtFlags |= DebugTextFlags.FACE_CAMERA;
		vector mat[4];
		GetWorldTransform(mat);
		float size = m_Size;
		if (m_ScaleByDistance)
			size *= 10; // To keep the sizes similar for now

		int bgColor;
		if (m_TextBackground)
			bgColor = ARGBF(1 - m_BackgroundTransparency, m_BackgroundColor[0], m_BackgroundColor[1], m_BackgroundColor[2]);
		else
			bgColor = ARGBF(0, 0, 0, 0);
		DebugTextWorldSpace.CreateInWorld(GetWorld(), m_Comment, dtFlags, mat, size, ARGBF(1 - m_Transparency, m_Color[0], m_Color[1], m_Color[2]), bgColor);
	}

#ifdef WORKBENCH
	float m_fWB_Width = 1;
	float m_fWB_Height = 1;
	//------------------------------------------------------------------------------------------------
	override void _WB_GetBoundBox(inout vector min, inout vector max, IEntitySource src)
	{
		if (m_FaceCamera)
		{
			min = Vector(m_fWB_Width * -0.5, m_fWB_Height * -0.5, m_fWB_Width * -0.5);
			max = Vector(m_fWB_Width * 0.5, m_fWB_Height * 0.5, m_fWB_Width * 0.5);
		}
		else
		{
			min = Vector(m_fWB_Width * -0.5, m_fWB_Height * -0.5, m_fWB_Height * 0.25 * -0.5);
			max = Vector(m_fWB_Width * 0.5, m_fWB_Height * 0.5, m_fWB_Height * 0.25 * 0.5);
		}
	}

	//------------------------------------------------------------------------------------------------
	override int _WB_GetAfterWorldUpdateSpecs(IEntitySource src)
	{
		return EEntityFrameUpdateSpecs.CALL_WHEN_ENTITY_VISIBLE;
	}
	
	//------------------------------------------------------------------------------------------------
	override void _WB_AfterWorldUpdate(float timeSlice)
	{
		WorldEditorAPI api = _WB_GetEditorAPI();
		if (!api)
			return;

		IEntitySource src = api.EntityToSource(this);
		if (!src)
			return;

		if (api.IsEntityLayerVisible(src.GetSubScene(), src.GetLayerID()))
			DrawComment();
	}
#endif

	//------------------------------------------------------------------------------------------------
	override void EOnFrame(IEntity owner, float timeSlice) //!EntityEvent.FRAME
	{
		DrawComment();
	}

	//------------------------------------------------------------------------------------------------
	void CommentEntity(IEntitySource src, IEntity parent)
	{
		if (m_VisibleIngame && RplSession.Mode() != RplMode.Dedicated)
		{
			SetEventMask(EntityEvent.FRAME);
		}

		ChimeraWorld world = GetGame().GetWorld();
		if (world)
			world.RegisterEntityToBeUpdatedWhileGameIsPaused(this);
	}

	//------------------------------------------------------------------------------------------------
	void ~CommentEntity()
	{
		ChimeraWorld world = GetGame().GetWorld();
		if (world)
			world.UnregisterEntityToBeUpdatedWhileGameIsPaused(this);
	}
};
