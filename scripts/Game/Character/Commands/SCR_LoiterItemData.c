[BaseContainerProps()]
class SCR_LoiterItemData : SCR_LoiterItemDataBase
{
	[Attribute(defvalue: "{6636333D6A19AF17}Prefabs/Props/Civilian/Cigar/AnimationProp_Cigar.et", desc: "Item which is going to be spawned locally by all clients who see playback of the animation", uiwidget: UIWidgets.ResourcePickerThumbnail, params: "et")]
	protected ResourceName m_sItemPrefab;

	[Attribute(defvalue: "RightHandProp", desc: "Name of the bone to which item will be attached during animation playback.\nEmpty field means that object is not attached to the character")]
	protected string m_sBoneName;

	[Attribute(desc: "Position offset in local space of the bone to which this item will be attached during animation playback.\nIf no bone was provided, then position offset will be applied to the position provided by the entity which playing the animation, while taking into account the direction of that entity.", precision: 4)]
	protected vector m_vPositionOffsetLS;

	[Attribute(desc: "Angle offset applied in local space of the bone to which this item will be attached during animation playback.\nIf no bone was provided, then angle offset will be applied to the angles provided by the entity which playing the animation.", precision: 4)]
	protected vector m_vAngleOffsetLS;

	//------------------------------------------------------------------------------------------------
	override array<IEntity> SpawnPrefabs(notnull IEntity animationOwner, int seed)
	{
		Resource res = Resource.Load(m_sItemPrefab);
		if (!res.IsValid())
			return null;

		EntitySpawnParams params();
		animationOwner.GetTransform(params.Transform);
		IEntity item = GetGame().SpawnEntityPrefabLocal(res, animationOwner.GetWorld(), params);
		if (!item)
			return null;

		vector mat[4];
		Math3D.AnglesToMatrix(m_vAngleOffsetLS, mat);
		mat[3] = m_vPositionOffsetLS;

		if (!m_sBoneName.IsEmpty())
		{
			TNodeId nodeId = animationOwner.GetAnimation().GetBoneIndex(m_sBoneName);
			animationOwner.AddChild(item, nodeId);
		}
		else
		{
			// if it is not attached then translate local offset to world space
			mat[3] = params.Transform[3] + animationOwner.VectorToParent(mat[3]);
			Math3D.MatrixMultiply3(mat, params.Transform, mat);
		}

		item.SetLocalTransform(mat);

		return {item};
	}
}
