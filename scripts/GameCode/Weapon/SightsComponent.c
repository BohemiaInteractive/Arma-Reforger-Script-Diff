//! Temporary dummy container for generated data
class ZeroingGeneratedData
{
	vector m_Angles;
	vector m_Offset;
};

class SightsComponentClass: BaseSightsComponentClass
{
	[Attribute( defvalue: "-1", uiwidget: UIWidgets.Slider, desc: "Distance from eye to the nearby DOF", "-1 500 1")]
	int m_iOpticDOFDistanceScale;
	
	[Attribute( defvalue: "0", uiwidget: UIWidgets.CheckBox, desc: "Nearby DOF is forced to be simple, not bokeh DOF. Essential for weapon elements extremely close to camera")]
	bool m_bForceSimpleDOF;
};

class SightsComponent : BaseSightsComponent
{
	// TODO@AS:
	// Move this black magic into cpp
	// And add messages to user when something fails!
	const string EYE_BONE = "eye";
	
	//------------------------------------------------------------------------------------------------
	//! Applies the given recoil angles to the camera transform according to the type of sight
	//! By default this method is empty. It should be overridden by each sight to make the appropriate adjustments
	void ApplyRecoilToCamera(inout vector pOutCameraTransform[4], vector aimModAngles);
	
	//------------------------------------------------------------------------------------------------
	bool CanFreelook()
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Used by SCR_DepthOfFieldEffect to scale the DOF based on each particular ironsight-distance
	bool GetDOFRelatedPrefabData(out int opticDOFDistanceScale = -1, out bool forceSimpleDOF = false)
	{
		SightsComponentClass prefabData = SightsComponentClass.Cast(GetComponentData(GetOwner()));
		if (!prefabData)
			return false;
		
		opticDOFDistanceScale = prefabData.m_iOpticDOFDistanceScale;
		forceSimpleDOF = prefabData.m_bForceSimpleDOF;
		
		return true;
	}
	
	#ifdef WORKBENCH
		
	//------------------------------------------------------------------------------------------------
	//! Positive weapon angle tilts weapon upwards (muzzle goes up, stock goes down)
	//! Negative weapon angle tilts weapon downwards (muzzle goes down, stock goes up)
	//! Called from GameCode, do not remove!
	protected override bool WB_GetZeroingData(IEntity owner, BaseSightsComponent sights, float weaponAngle, out vector offset, out vector angles)
	{
		if (!sights)
			return false;
		
		ref ZeroingGeneratedData data;
		IEntity parent = owner.GetParent();
		if (parent)
			data = CalculateDataWithParent(sights, owner, parent, weaponAngle);
		else
			data = CalculateDataWithParent(sights, owner, owner, weaponAngle); // if owner==parent.. its ok
		
		offset = data.m_Offset;
		angles = data.m_Angles;
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool FindParent(IEntity owner, string eyeBoneName, out IEntity parentWeapon, out int boneIndex)
	{
		IEntity parent = owner;
		while (parent)
		{
			BaseWeaponComponent weapon = BaseWeaponComponent.Cast(parent.FindComponent(BaseWeaponComponent));
			if (weapon)
			{
				int eye = parent.GetAnimation().GetBoneIndex(EYE_BONE);
				if (eye != -1)
				{
					boneIndex = eye;
					parentWeapon = parent;
					return true;
				}
			}
			parent = parent.GetParent();
		}
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
		Takes front sights position as the anchor around which it rotates both the owner and the parent under desiredAngle
		(or rather negative desired angle, as we are pivoting around our target) and calculates new transformation
		that needs to be applied to reach desired angle.
	*/
	static ZeroingGeneratedData CalculateDataWithParent(BaseSightsComponent sights, IEntity owner, IEntity parent, float desiredAngle)
	{
		/// Store original data prior to modification
		vector originalParentTransformWS[4];
		parent.GetWorldTransform(originalParentTransformWS);
		vector originalChildTransformLS[4];
		owner.GetLocalTransform(originalChildTransformLS);
		
		/// Re:zero weapon and attachment
		vector parentTransformWS[4];
		Math3D.MatrixIdentity4(parentTransformWS);
		parent.SetWorldTransform(parentTransformWS);
		vector childTransformWS[4]; // transform sights into parent space
		if (parent == owner) // unless we are in that space already
			Math3D.MatrixCopy(parentTransformWS, childTransformWS);
		else
			Math3D.MatrixMultiply4(originalChildTransformLS, parentTransformWS, childTransformWS);
		
		owner.SetWorldTransform(childTransformWS);
		
		/// Get desired pivot point in WS
		vector pivotPointWS = sights.GetSightsFrontPosition(false);
		
		/// Rotate parent around pivot point
		float angle = -desiredAngle * Math.DEG2RAD;
		vector axis = vector.Right;
		vector newParentTransformWS[4];
		SCR_Math3D.RotateAround(parentTransformWS, pivotPointWS, axis, angle, newParentTransformWS);
		vector newChildTransformWS[4];
		SCR_Math3D.RotateAround(childTransformWS, pivotPointWS, axis, angle, newChildTransformWS);
		
		/// Apply new transformation
		parent.SetWorldTransform(newParentTransformWS);
		owner.SetWorldTransform(newChildTransformWS);
		
		
		ref ZeroingGeneratedData data  = new ZeroingGeneratedData();
		data.m_Offset = newParentTransformWS[3];
		data.m_Offset[2] = -data.m_Offset[2];
		vector yawPitchRoll = Math3D.MatrixToAngles(newParentTransformWS);
		data.m_Angles = Vector(yawPitchRoll[1], yawPitchRoll[0], yawPitchRoll[2]);
		return data;
	}
	#endif
};
