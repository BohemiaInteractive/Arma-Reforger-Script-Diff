class SCR_EditorSettings: ModuleGameSettings
{	
	[Attribute(defvalue: "0", desc: "When enabled the GM can enter layers, add entities to layers and remove them from layers. Some group functions are excluded.")]
	bool m_bLayerEditing;
	
	[Attribute(EEditorTransformVertical.GEOMETRY.ToString(), uiwidget: UIWidgets.ComboBox, desc: "Preview vertical mode, decides along which surface will entities be edited.", enums: ParamEnumArray.FromEnum(EEditorTransformVertical))]
	EEditorTransformVertical m_PreviewVerticleMode;
	
	[Attribute("1", uiwidget: UIWidgets.CheckBox, desc: "Preview vertical snap, When enabled, will snap entities to the terrain, if false it will be possible for entities to be placed inside the ground.", enums: ParamEnumArray.FromEnum(EEditorTransformVertical))]
	bool m_PreviewVerticalSnap;
	
	[Attribute(defvalue: "1", desc: "If true will show identities for characters in the editor tooltip.")]
	bool m_bShowIdentityBioTooltip;
};