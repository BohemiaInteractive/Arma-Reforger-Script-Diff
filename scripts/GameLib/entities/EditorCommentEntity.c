[EntityEditorProps(category: "GameLib/Scripted", description: "Editor comment, for leaving notes in the editor", sizeMin: "-1 -1 -1", sizeMax: "1 1 1", visible: false, dynamicBox: true)]
class EditorCommentEntityClass : GenericEntityClass
{
	[Attribute("0.2", UIWidgets.EditBox, "Text size in meters")]
	float m_Size;

	[Attribute("1 1 1 1", UIWidgets.ColorPicker, "Text color")]
	ref Color m_Color;

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

	[Attribute("0 0 0 0.5", UIWidgets.ColorPicker, "Background color")]
	ref Color m_BackgroundColor;
	
	[Attribute("", UIWidgets.EditComboBox, "Category")]
	string m_Category;

#ifdef WORKBENCH
	//------------------------------------------------------------------------------------------------
	static override int _WB_GetAfterWorldUpdateSpecs(WorldEditorAPI api, IEntitySource src)
	{
		return EEntityFrameUpdateSpecs.CALL_WHEN_ENTITY_VISIBLE | EEntityFrameUpdateSpecs.CALL_WHEN_ENTITY_SELECTED;
	}

	//------------------------------------------------------------------------------------------------
	static override void _WB_AfterWorldUpdate(WorldEditorAPI api, IEntitySource src, float timeSlice)
	{
		EditorCommentEntity ent = EditorCommentEntity.Cast(api.SourceToEntity(src));
		if (!ent)
			return;

		if (api.IsEntityLayerVisible(src.GetSubScene(), src.GetLayerID()))
			ent.DrawComment();
	}
#endif
};

[Friend(EditorCommentEntityClass)]
class EditorCommentEntity : GenericEntity
{
	[Attribute("New comment", UIWidgets.EditBoxMultiline, "Write any comment you wish")]
	string m_Comment;

#ifdef WORKBENCH
	int m_MaxLineLen;
#endif

	//------------------------------------------------------------------------------------------------
	protected void DrawComment()
	{
		const float textWidthScale = 0.7;

		vector textMat[4];
		GetWorld().GetCurrentCamera(textMat);
		
		EditorCommentEntityClass prefabData = EditorCommentEntityClass.Cast(GetPrefabData());
		if (!prefabData.m_FaceCamera)
		{
			textMat[0] = GetTransformAxis(0);
			textMat[1] = GetTransformAxis(1);
			textMat[2] = GetTransformAxis(2);
		}

		float distScale = 1;
		if (prefabData.m_ScaleByDistance)
		{
			distScale = vector.Distance(textMat[3], GetOrigin()) * 0.1;
			distScale = Math.Clamp(distScale, 0.5, 10);
		}

		float textEndSize = (prefabData.m_Size * distScale) / vector.Distance(textMat[3], GetOrigin());
		if (textEndSize < 0.005)
			return;

		textMat[3] = GetOrigin() - textMat[1] * prefabData.m_Size * distScale * 0.5;
		ShapeFlags flags = ShapeFlags.ONCE | ShapeFlags.TRANSP;
		if (prefabData.m_VisibleOverall)
			flags |= ShapeFlags.NOZBUFFER;
		if (prefabData.m_Color.A() == 0.0)
			flags |= ShapeFlags.TRANSP;

#ifdef WORKBENCH
		m_fWB_Width = m_MaxLineLen * prefabData.m_Size * distScale * textWidthScale;
		m_fWB_Height = prefabData.m_Size * distScale;
#endif

		int dtFlags = DebugTextFlags.CENTER | DebugTextFlags.ONCE;
		if (prefabData.m_FaceCamera)
			dtFlags |= DebugTextFlags.FACE_CAMERA;
		vector mat[4];
		GetWorldTransform(mat);
		float size = prefabData.m_Size;
		if (prefabData.m_ScaleByDistance)
			size *= 10; // To keep the sizes similar for now

		int bgColor;
		if (prefabData.m_TextBackground)
			bgColor = ARGBF(prefabData.m_BackgroundColor.A(), prefabData.m_BackgroundColor.R(), prefabData.m_BackgroundColor.G(), prefabData.m_BackgroundColor.B());
		else
			bgColor = ARGBF(0, 0, 0, 0);
		DebugTextWorldSpace.CreateInWorld(GetWorld(), m_Comment, dtFlags, mat, size, ARGBF(prefabData.m_Color.A(), prefabData.m_Color.R(), prefabData.m_Color.G(), prefabData.m_Color.B()), bgColor);
	}

#ifdef WORKBENCH
	float m_fWB_Width = 1;
	float m_fWB_Height = 1;
	//------------------------------------------------------------------------------------------------
	override void _WB_GetBoundBox(inout vector min, inout vector max, IEntitySource src)
	{
		EditorCommentEntityClass prefabData = EditorCommentEntityClass.Cast(GetPrefabData());
		if (prefabData.m_FaceCamera)
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
	override event void _WB_SetTransform(inout vector mat[4], IEntitySource src)
	{
		OnTransformChanged(mat);
	}

	//------------------------------------------------------------------------------------------------
	//! Called in Workbench whenever this entity's transform is edited.
	protected void OnTransformChanged(vector mat[4])
	{
		WorldEditorAPI worldEditorAPI = _WB_GetEditorAPI();
		if (worldEditorAPI)
			worldEditorAPI.MoveCommentEntity(this, mat[3]);
	}

	//------------------------------------------------------------------------------------------------
	override array<ref ParamEnum> _WB_GetUserEnums(string varName, IEntitySource src)
	{
		WorldEditorAPI api = _WB_GetEditorAPI();
		if (!api)
			return null;

		if (varName != "m_Category")
			return null;

		array<string> cats = {};
		api.GetCommentCategories(cats);

		array<ref ParamEnum> result = {};
		foreach(string cat : cats)
			result.Insert(new ParamEnum(cat, cat, ""));

		return result;
	}
#endif

	//------------------------------------------------------------------------------------------------
	override void EOnFrame(IEntity owner, float timeSlice) //!EntityEvent.FRAME
	{
		DrawComment();
	}

	//------------------------------------------------------------------------------------------------
	override protected void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);
		
		EditorCommentEntityClass prefabData = EditorCommentEntityClass.Cast(GetPrefabData());
		if (prefabData.m_VisibleIngame && RplSession.Mode() != RplMode.Dedicated)
			SetEventMask(EntityEvent.FRAME);
#ifdef WORKBENCH
		WorldEditorAPI worldEditorAPI = _WB_GetEditorAPI();
		if (worldEditorAPI)
			worldEditorAPI.AddCommentEntity(this, m_Comment, prefabData.m_Color, prefabData.m_Category);
#endif
	}

	//------------------------------------------------------------------------------------------------
	void EditorCommentEntity(IEntitySource src, IEntity parent)
	{
		if (System.IsConsoleApp())
			return;

		SetEventMask(EntityEvent.INIT);
#ifdef WORKBENCH
		//calc m_MaxLineLen
		array<string> commentLines = {};
		m_Comment.Split("\n", commentLines, false);
		m_MaxLineLen = 0;
		
		if (!commentLines.IsEmpty())
		{
			m_MaxLineLen = commentLines[0].Length();
			for (int i = 0; i < commentLines.Count(); i++)
			{
				if (commentLines[i].Length() > m_MaxLineLen)
					m_MaxLineLen = commentLines[i].Length();
			}
		}
#endif
	}

	//------------------------------------------------------------------------------------------------
	void ~EditorCommentEntity()
	{
#ifdef WORKBENCH
		WorldEditorAPI worldEditorAPI = _WB_GetEditorAPI();
		if (worldEditorAPI)
			worldEditorAPI.RemoveCommentEntity(this);
#endif
	}
};
