[BaseContainerProps(), SCR_BaseContainerCustomTitleResourceName("m_Mesh", true)]
class SCR_BasePreviewEntry
{
	[Attribute()]
	int m_iParentID;
	
	[Attribute(uiwidget: UIWidgets.ResourcePickerThumbnail, params: "xob et")]
	ResourceName m_Mesh;
	
	[Attribute(uiwidget: UIWidgets.Flags, enums: ParamEnumArray.FromEnum(EPreviewEntityFlag))]
	EPreviewEntityFlag m_Flags;
	
	[Attribute(uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(EPreviewEntityShape))]
	EPreviewEntityShape m_Shape;
	
	[Attribute("-1")]
	string m_iPivotID;
	
	[Attribute(defvalue: "1 1 1")]
	vector m_vScale;
	
	[Attribute()]
	vector m_vPosition;
	
	[Attribute()]
	vector m_vAngles;
	
	float m_fQuat[4]; //--- Optional quaternion, used only in run-time, i.e., not on prefab
	vector m_vAnglesTerrain;
	float m_vHeightTerrain;
	IEntity m_Entity;
	IEntitySource m_EntitySource;
	
	/*!
	Save scale of entity within vector scale
	\param scale Scale to save
	*/
	void SetScale(float scale)
	{
		m_vScale = Vector(scale, scale, scale);
	}
	
	/*!
	Get the scale saved in the Preview entity. 
	Note that the first entry in the scale vector is used
	\return Scale
	*/
	float GetScale()
	{
		return m_vScale[0];
	}
	
	/*!
	Save transformation matrix in entry data.
	\param transform Local transformation matrix
	*/
	void SaveTransform(vector transform[4])
	{
		m_vAngles = Math3D.MatrixToAngles(transform);
		Math3D.MatrixToQuat(transform, m_fQuat);
		m_vPosition = transform[3];
	}
	/*!
	Load entry data into transformation matrix
	\param[out] outTransform Local transformation matrix to be filled with entry data
	*/
	void LoadTransform(vector outTransform[4])
	{
		if (m_fQuat[0] != 0 || m_fQuat[1] != 0 || m_fQuat[2] != 0 || m_fQuat[3] != 0)
			Math3D.QuatToMatrix(m_fQuat, outTransform);
		else
			Math3D.AnglesToMatrix(Vector(m_vAngles[1], m_vAngles[0], m_vAngles[2]), outTransform); //--- Convert XYZ to XZY
		
		outTransform[3] = m_vPosition;
	}
	
	void Log(int index)
	{
		string mesh = m_Mesh;
		mesh = string.Format("%1: %2", typename.EnumToString(EPreviewEntityShape, m_Shape), mesh);
		string flagNames = SCR_Enum.FlagsToString(EPreviewEntityFlag, m_Flags);
		string text = string.Format("prt: %1 | pvt: '%2' | scl: %3 | pos: %4 | ang: %5 | angT: %6 | hT: %7 | flg: %8 | shape: '%9'", m_iParentID, m_iPivotID, m_vScale, m_vPosition, m_vAngles, m_vAnglesTerrain, m_vHeightTerrain, flagNames, mesh);
		PrintFormat("%1: %2", index, text);
	}
	void CopyFrom(SCR_BasePreviewEntry from)
	{
		m_iParentID = from.m_iParentID;
		m_Mesh = from.m_Mesh;
		m_Flags = from.m_Flags;
		m_Shape = from.m_Shape;
		m_iPivotID = from.m_iPivotID;
		m_vScale = from.m_vScale;
		m_vPosition = from.m_vPosition;
		m_fQuat = from.m_fQuat;
		m_vAngles = from.m_vAngles;
		m_vAnglesTerrain = from.m_vAnglesTerrain;
		m_vHeightTerrain = from.m_vHeightTerrain;
		m_Entity = from.m_Entity;
		m_EntitySource = from.m_EntitySource;
	}
	void SCR_BasePreviewEntry(bool init = false)
	{
		if (init)
		{
			SetScale(1);
		}
	}
#ifdef WORKBENCH
	void SaveToContainer(WorldEditorAPI api, IEntitySource entitySource, array<ref ContainerIdPathEntry> entryPath)
	{
		api.SetVariableValue(entitySource, entryPath, "m_iParentID", m_iParentID.ToString());
		api.SetVariableValue(entitySource, entryPath, "m_Mesh", m_Mesh);
		api.SetVariableValue(entitySource, entryPath, "m_Shape", m_Shape.ToString());
		api.SetVariableValue(entitySource, entryPath, "m_iPivotID", m_iPivotID);
		api.SetVariableValue(entitySource, entryPath, "m_vScale", m_vScale.ToString(false));
		api.SetVariableValue(entitySource, entryPath, "m_vPosition", m_vPosition.ToString(false));
		api.SetVariableValue(entitySource, entryPath, "m_vAngles", m_vAngles.ToString(false));
		api.SetVariableValue(entitySource, entryPath, "m_Flags", m_Flags.ToString());
	}
#endif
};