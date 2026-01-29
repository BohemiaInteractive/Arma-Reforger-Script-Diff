[EntityEditorProps(category: "GameBase/Joints", description: "Physics joint - 6DOF (supports spring)", sizeMin: "-0.05 -0.05 -0.05", sizeMax: "0.05 0.05 0.05", color: "0 0 255 255")]
class SCR_Joint6DOFEntityClass: SCR_JointBaseEntityClass
{
	const int AXIS_YAW = 4;
	const int AXIS_PITCH = 3;
	const int AXIS_ROLL = 5;
	
	const int AXIS_RIGHT = 0;
	const int AXIS_UP = 1;
	const int AXIS_FORWARD = 2;
};

//------------------------------------------------------------------------------------------------
//! SCR_Joint6DOFEntity Class
//!
//! TODO: doc
//------------------------------------------------------------------------------------------------
class SCR_Joint6DOFEntity: SCR_JointBaseEntity
{
	[Attribute("0", UIWidgets.CheckBox, "Whether to set spring values on the yaw axis")]
	bool m_YawSetSpring;
	[Attribute("0", UIWidgets.CheckBox, "Whether the yaw axis may rotate freely (ignores limits)")]
	bool m_YawIsFree;
	[Attribute("0", UIWidgets.Slider, "Minimum yaw range in degrees", "-180 0 0.01")]
	float m_YawMin;
	[Attribute("0", UIWidgets.Slider, "Maximum yaw range in degrees", "0 180 0.01")]
	float m_YawMax;
	//------------------------------------------------------------------------------------------------
	
	[Attribute("0", UIWidgets.CheckBox, "Whether to set spring values on the pitch axis")]
	bool m_PitchSetSpring;
	[Attribute("0", UIWidgets.CheckBox, "Whether the pitch axis may rotate freely (ignores limits)")]
	bool m_PitchIsFree;
	[Attribute("0", UIWidgets.Slider, "Minimum pitch range in degrees", "-180 0 0.01")]
	float m_PitchMin;
	[Attribute("0", UIWidgets.Slider, "Maximum pitch range in degrees", "0 180 0.01")]
	float m_PitchMax;
	//------------------------------------------------------------------------------------------------
	
	[Attribute("0", UIWidgets.CheckBox, "Whether to set spring values on the roll axis")]
	bool m_RollSetSpring;
	[Attribute("0", UIWidgets.CheckBox, "Whether the roll axis may rotate freely (ignores limits)")]
	bool m_RollIsFree;
	[Attribute("0", UIWidgets.Slider, "Minimum roll range in degrees", "-180 0 0.01")]
	float m_RollMin;
	[Attribute("0", UIWidgets.Slider, "Maximum roll range in degrees", "0 180 0.01")]
	float m_RollMax;
	//------------------------------------------------------------------------------------------------
	
	[Attribute("0", UIWidgets.CheckBox, "Whether to set spring values along the right axis")]
	bool m_RightSetSpring;
	[Attribute("0", UIWidgets.CheckBox, "Whether the right axis may move freely (ignores limits)")]
	bool m_RightIsFree;
	[Attribute("0", UIWidgets.Slider, "Minimum right axis range in meters", "-50 0 0.01")]
	float m_RightMin;
	[Attribute("0", UIWidgets.Slider, "Maximum right axis range in meters", "0 50 0.01")]
	float m_RightMax;
	//------------------------------------------------------------------------------------------------
	
	[Attribute("0", UIWidgets.CheckBox, "Whether to set spring values along the up axis")]
	bool m_UpSetSpring;
	[Attribute("0", UIWidgets.CheckBox, "Whether the up axis may move freely (ignores limits)")]
	bool m_UpIsFree;
	[Attribute("0", UIWidgets.Slider, "Minimum up axis range in meters", "-50 0 0.01")]
	float m_UpMin;
	[Attribute("0", UIWidgets.Slider, "Maximum up axis range in meters", "0 50 0.01")]
	float m_UpMax;
	//------------------------------------------------------------------------------------------------
	
	[Attribute("0", UIWidgets.CheckBox, "Whether to set spring values along the forward axis" )]
	bool m_ForwardSetSpring;
	[Attribute("0", UIWidgets.CheckBox, "Whether the forward axis may move freely (ignores limits)")]
	bool m_ForwardIsFree;
	[Attribute("0", UIWidgets.Slider, "Minimum forward axis range in meters", "-50 0 0.01")]
	float m_ForwardMin;
	[Attribute("0", UIWidgets.Slider, "Maximum forward axis range in meters", "0 50 0.01")]
	float m_ForwardMax;
	//------------------------------------------------------------------------------------------------
	
	[Attribute("10", UIWidgets.Slider, "Stiffness of the spring", "0 1000 0.001")]
	float m_SpringStiffness;
	[Attribute("0.5", UIWidgets.Slider, "Damping of the spring, lower values means more damping", "0 1 0.001")]
	float m_SpringDamping;
	
	//------------------------------------------------------------------------------------------------
	
	
	//------------------------------------------------------------------------------------------------
	override void DebugDisplay()
	{
		vector mat[4];
		GetTransform(mat);
		vector pos = mat[3];
		
		vector axisVec = mat[0];
		vector axisVec2 = mat[1];
		vector axisVec3 = mat[2];
		
		bool noOffsets = true;
		if (m_YawIsFree || m_YawMax != 0 || m_YawMin != 0)
			noOffsets = false;
		else if (m_PitchIsFree || m_PitchMax != 0 || m_PitchMin != 0)
			noOffsets = false;
		else if (m_RollIsFree || m_RollMax != 0 || m_RollMin != 0)
			noOffsets = false;
		else if (m_RightIsFree || m_RightMax != 0 || m_RightMin != 0)
			noOffsets = false;
		else if (m_UpIsFree || m_UpMax != 0 || m_UpMin != 0)
			noOffsets = false;
		else if (m_ForwardIsFree || m_ForwardMax != 0 || m_ForwardMin != 0)
			noOffsets = false;
		
		if (noOffsets)
		{
			Shape.Create(ShapeType.LINE, ARGB(255, 0, 255, 0), ShapeFlags.ONCE|ShapeFlags.NOZBUFFER, axisVec * -0.05 + pos, axisVec * 0.05 + pos);
			Shape.Create(ShapeType.LINE, ARGB(255, 0, 255, 0), ShapeFlags.ONCE|ShapeFlags.NOZBUFFER, axisVec2 * -0.05 + pos, axisVec2 * 0.05 + pos);
			Shape.Create(ShapeType.LINE, ARGB(255, 0, 255, 0), ShapeFlags.ONCE|ShapeFlags.NOZBUFFER, axisVec3 * -0.05 + pos, axisVec3 * 0.05 + pos);
			
			return;
		}
		
		if (m_YawIsFree)
			CreateCircleSlice(pos, axisVec2, axisVec, 1, -1, 0.075, ARGB(255, 0, 255, 255), 16, ShapeFlags.ONCE|ShapeFlags.NOZBUFFER);
		else
		{
			if (m_YawMin != 0 || m_YawMax != 0)
				CreateCircleSlice(pos, axisVec2, axisVec, m_YawMin, m_YawMax, 0.075, ARGB(255, 0, 255, 255), Math.Ceil(Math.AbsFloat(m_YawMax - m_YawMin) / 360 * 16), ShapeFlags.ONCE|ShapeFlags.NOZBUFFER);
		}
		if (m_PitchIsFree)
			CreateCircleSlice(pos, -axisVec, axisVec2, 1, -1, 0.075, ARGB(255, 0, 255, 255), 16, ShapeFlags.ONCE|ShapeFlags.NOZBUFFER);
		else
		{
			if (m_PitchMin != 0 || m_PitchMax != 0)
				CreateCircleSlice(pos, -axisVec, axisVec2, m_PitchMin, m_PitchMax, 0.075, ARGB(255, 0, 255, 255), Math.Ceil(Math.AbsFloat(m_PitchMax - m_PitchMin) / 360 * 16), ShapeFlags.ONCE|ShapeFlags.NOZBUFFER);
		}
		if (m_RollIsFree)
			CreateCircleSlice(pos, -axisVec3, axisVec, 1, -1, 0.075, ARGB(255, 0, 255, 255), 16, ShapeFlags.ONCE|ShapeFlags.NOZBUFFER);
		else
		{
			if (m_RollMin != 0 || m_RollMax != 0)
				CreateCircleSlice(pos, -axisVec3, axisVec, m_RollMin, m_RollMax, 0.075, ARGB(255, 0, 255, 255), Math.Ceil(Math.AbsFloat(m_RollMax - m_RollMin) / 360 * 16), ShapeFlags.ONCE|ShapeFlags.NOZBUFFER);
		}
		
		if (m_RightIsFree)
			Shape.Create(ShapeType.LINE, ARGB(255, 255, 0, 255), ShapeFlags.ONCE|ShapeFlags.NOZBUFFER, axisVec * -0.15 + pos, axisVec * 0.15 + pos);
		else
			Shape.Create(ShapeType.LINE, ARGB(255, 0, 255, 255), ShapeFlags.ONCE|ShapeFlags.NOZBUFFER, axisVec * m_RightMin + pos, axisVec * m_RightMax + pos);
		if (m_UpIsFree)
			Shape.Create(ShapeType.LINE, ARGB(255, 255, 0, 255), ShapeFlags.ONCE|ShapeFlags.NOZBUFFER, axisVec2 * -0.15 + pos, axisVec2 * 0.15 + pos);
		else
			Shape.Create(ShapeType.LINE, ARGB(255, 0, 255, 255), ShapeFlags.ONCE|ShapeFlags.NOZBUFFER, axisVec2 * m_UpMin + pos, axisVec2 * m_UpMax + pos);
		if (m_ForwardIsFree)
			Shape.Create(ShapeType.LINE, ARGB(255, 255, 0, 255), ShapeFlags.ONCE|ShapeFlags.NOZBUFFER, axisVec3 * -0.15 + pos, axisVec3 * 0.15 + pos);
		else
			Shape.Create(ShapeType.LINE, ARGB(255, 0, 255, 255), ShapeFlags.ONCE|ShapeFlags.NOZBUFFER, axisVec3 * m_ForwardMin + pos, axisVec3 * m_ForwardMax + pos);
	}
	
	//------------------------------------------------------------------------------------------------
	float GetSpringScale()
	{
		float result = (SCR_Global.g_fPhysicsHz - 60) / 60;
		result = Math.Clamp(result, 0, 1);
		result = 1 - result + 1;
		result *= 10;
		result *= m_SpringStiffness * m_SpringStiffness;
		
		return result;
	}
	
	//------------------------------------------------------------------------------------------------
	float GetSpringLinDampScale()
	{
		return 0.001 * m_SpringDamping;
	}
	
	//------------------------------------------------------------------------------------------------
	float GetSpringAngDampScale()
	{
		return 0.001 * m_SpringDamping;
	}
	
	//------------------------------------------------------------------------------------------------
	override void CreateJoint(IEntity parent, IEntity child, vector jointMat1[4], vector jointMat2[4])
	{
		bool isSpring = false;
		if (m_YawSetSpring || m_PitchSetSpring || m_RollSetSpring || m_RightSetSpring || m_UpSetSpring || m_ForwardSetSpring)
			isSpring = true;
		
		// Create the joint
		if (isSpring)
		{
			m_Joint = PhysicsJoint.Create6DOFSpring(parent, child, jointMat1, jointMat2, m_CollisionBlocker, -1);
			Physics6DOFSpringJoint joint6DOF = m_Joint;
			if (m_YawSetSpring)
				joint6DOF.SetSpring(SCR_Joint6DOFEntityClass.AXIS_YAW, GetSpringScale(), GetSpringAngDampScale());
			if (m_PitchSetSpring)
				joint6DOF.SetSpring(SCR_Joint6DOFEntityClass.AXIS_PITCH, GetSpringScale(), GetSpringAngDampScale());
			if (m_RollSetSpring)
				joint6DOF.SetSpring(SCR_Joint6DOFEntityClass.AXIS_ROLL, GetSpringScale(), GetSpringAngDampScale());
			if (m_RightSetSpring)
				joint6DOF.SetSpring(SCR_Joint6DOFEntityClass.AXIS_RIGHT, GetSpringScale(), GetSpringLinDampScale());
			if (m_UpSetSpring)
				joint6DOF.SetSpring(SCR_Joint6DOFEntityClass.AXIS_UP, GetSpringScale(), GetSpringLinDampScale());
			if (m_ForwardSetSpring)
				joint6DOF.SetSpring(SCR_Joint6DOFEntityClass.AXIS_FORWARD, GetSpringScale(), GetSpringLinDampScale());
		}
		else
			m_Joint = PhysicsJoint.Create6DOF(parent, child, jointMat1, jointMat2, m_CollisionBlocker, -1);
		
		Physics6DOFJoint joint6DOF = m_Joint;
		if (m_YawIsFree)
			joint6DOF.SetLimit(SCR_Joint6DOFEntityClass.AXIS_YAW, 1, -1);
		else
			joint6DOF.SetLimit(SCR_Joint6DOFEntityClass.AXIS_YAW, m_YawMax * Math.DEG2RAD, m_YawMin * Math.DEG2RAD);
		if (m_PitchIsFree)
			joint6DOF.SetLimit(SCR_Joint6DOFEntityClass.AXIS_PITCH, 1, -1);
		else
			joint6DOF.SetLimit(SCR_Joint6DOFEntityClass.AXIS_PITCH, m_PitchMin * Math.DEG2RAD, m_PitchMax * Math.DEG2RAD);
		if (m_RollIsFree)
			joint6DOF.SetLimit(SCR_Joint6DOFEntityClass.AXIS_ROLL, 1, -1);
		else
			joint6DOF.SetLimit(SCR_Joint6DOFEntityClass.AXIS_ROLL, m_RollMin * Math.DEG2RAD, m_RollMax * Math.DEG2RAD);
		
		if (m_RightIsFree)
			joint6DOF.SetLimit(SCR_Joint6DOFEntityClass.AXIS_RIGHT, 1, -1);
		else
			joint6DOF.SetLimit(SCR_Joint6DOFEntityClass.AXIS_RIGHT, m_RightMin, m_RightMax);
		if (m_UpIsFree)
			joint6DOF.SetLimit(SCR_Joint6DOFEntityClass.AXIS_UP, 1, -1);
		else
			joint6DOF.SetLimit(SCR_Joint6DOFEntityClass.AXIS_UP, m_UpMin, m_UpMax);
		if (m_ForwardIsFree)
			joint6DOF.SetLimit(SCR_Joint6DOFEntityClass.AXIS_FORWARD, 1, -1);
		else
			joint6DOF.SetLimit(SCR_Joint6DOFEntityClass.AXIS_FORWARD, m_ForwardMin, m_ForwardMax);
	}
};
