class SCR_RaceTrackGeneratorPointData : ShapePointDataScriptBase
{
	[Attribute(defvalue: "1", desc: "Whether or not this point spawns a checkpoint, making this passage mandatory")]
	bool m_bIsCheckpoint;

	[Attribute(defvalue: "0", uiwidget: UIWidgets.Slider, desc: "Checkpoint yaw offset (for the checkpoint ", params: "-180 180")]
	float m_fCheckpointYawOffset;

	[Attribute(desc: "Which Prefab/composition to create on this point (do not add a checkpoint Prefab here as it is already created if Is Checkpoint is ticked)", params: "et")]
	ResourceName m_sPrefab;

	[Attribute(defvalue: "0", uiwidget: UIWidgets.Slider, desc: "Prefab Y Offset", params: "-180 180")]
	float m_fPrefabYawOffset;
}
