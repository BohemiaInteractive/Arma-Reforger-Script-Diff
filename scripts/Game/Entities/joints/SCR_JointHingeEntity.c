[EntityEditorProps(category: "GameBase/Joints", description: "Physics joint - Hinge", sizeMin: "-0.05 -0.05 -0.05", sizeMax: "0.05 0.05 0.05", color: "0 0 255 255")]
class SCR_JointHingeEntityClass: SCR_JointBaseEntityClass
{
	const int AXIS_YAW = 1;
	const int AXIS_PITCH = 0;
	const int AXIS_ROLL = 2;
};

//------------------------------------------------------------------------------------------------
//! SCR_JointHingeEntity Class
//!
//! TODO: doc
//------------------------------------------------------------------------------------------------
class SCR_JointHingeEntity: SCR_JointBaseEntity
{
	[Attribute("0", UIWidgets.ComboBox, "Which axis to use for the hinge", "", { ParamEnum("Yaw", "1"), ParamEnum("Pitch", "0"), ParamEnum("Roll", "2")} )]
	int m_Axis;
	[Attribute("0", UIWidgets.CheckBox, "Whether may rotate freely (ignores limits)")]
	bool m_RotateIsFree;
	[Attribute("0", UIWidgets.Slider, "Minimum rotate range in degrees (if minimum is higher than maximum, hinge can rotate freely)", "-180 0 0.01")]
	float m_RotateMin;
	[Attribute("0", UIWidgets.Slider, "Maximum rotate range in degrees (if minimum is higher than maximum, hinge can rotate freely)", "0 180 0.01")]
	float m_RotateMax;
	[Attribute("0.5", UIWidgets.Slider, "Percentage of limits where movement is free, above this the limit is gradually enforced", "0 1 0.001")]
	float m_Softness;
	[Attribute("0.3", UIWidgets.Slider, "The strength with which the constraint resists limit violations", "0 1 0.001")]
	float m_BiasFactor;
	[Attribute("1", UIWidgets.Slider, "The lower the values, the less the constraint will fight velocities which violate the angular limits", "0 1 0.001")]
	float m_RelaxationFactor;
	
	//------------------------------------------------------------------------------------------------
	
	
	//------------------------------------------------------------------------------------------------
	// NOTE: dSCR_JointHingeEntitySetAxis seems to reset the joint to zero orientations!!!
	/*void SetAxis(int axis)
	{
		Axis = axis;
		
		if (m_Joint)
		{
			vector axisVec;
			if (Axis == SCR_JointHingeEntityClass.AXIS_PITCH) axisVec = "1 0 0";
			if (Axis == SCR_JointHingeEntityClass.AXIS_YAW) axisVec = "0 1 0";
			if (Axis == SCR_JointHingeEntityClass.AXIS_ROLL) axisVec = "0 0 1";
			//dSCR_JointHingeEntitySetAxis(m_Joint, axisVec);
		}
	}*/
	
	//------------------------------------------------------------------------------------------------
	override void DebugDisplay()
	{
		vector mat[4];
		GetTransform(mat);
		vector pos = mat[3];
		
		int axis2 = m_Axis + 1;
		if (axis2 > 2)
			axis2 -= 2;
		int axis3 = axis2 + 1;
		if (axis3 > 2)
			axis3 -= 2;
		vector axisVec = mat[m_Axis];
		vector axisVec2 = mat[axis2];
		vector axisVec3 = mat[axis3];
		float addVisAng = 0;
		if (m_Axis == SCR_JointHingeEntityClass.AXIS_PITCH) addVisAng = 180; // Pitch
		if (m_Axis == SCR_JointHingeEntityClass.AXIS_YAW) addVisAng = 90; // Yaw
		if (m_Axis == SCR_JointHingeEntityClass.AXIS_ROLL) addVisAng = 90; // Roll
		
		int sliceSubDiv = 16;
		if (m_RotateMax > m_RotateMin)
			sliceSubDiv = Math.Ceil(Math.AbsFloat(-m_RotateMax - -m_RotateMin) / 360 * 16);
		
		CreateCircle(pos, axisVec, 0.05, ARGB(255, 0, 255, 0), 16, ShapeFlags.ONCE|ShapeFlags.NOZBUFFER);
		if (m_RotateIsFree)
			CreateCircle(pos, axisVec, 0.075, ARGB(255, 0, 255, 255), 16, ShapeFlags.ONCE|ShapeFlags.NOZBUFFER);
		else
			CreateCircleSlice(pos, axisVec, axisVec2, m_RotateMin + addVisAng, m_RotateMax + addVisAng, 0.075, ARGB(255, 0, 255, 255), sliceSubDiv, ShapeFlags.ONCE|ShapeFlags.NOZBUFFER);
		Shape.Create(ShapeType.LINE, ARGB(255, 0, 255, 0), ShapeFlags.ONCE|ShapeFlags.NOZBUFFER, axisVec * -0.1 + pos, axisVec * 0.1 + pos);
		Shape.Create(ShapeType.LINE, ARGB(255, 0, 255, 0), ShapeFlags.ONCE|ShapeFlags.NOZBUFFER, axisVec2 * -0.05 + pos, axisVec2 * 0.05 + pos);
		Shape.Create(ShapeType.LINE, ARGB(255, 0, 255, 0), ShapeFlags.ONCE|ShapeFlags.NOZBUFFER, axisVec3 * -0.05 + pos, axisVec3 * 0.05 + pos);
	}
	
	//------------------------------------------------------------------------------------------------
	override void RotateJoint(inout vector jointMat[4])
	{
		if (m_Axis == SCR_JointHingeEntityClass.AXIS_ROLL) // Roll
			return;
		
		vector oldJointMat[3], rotMat[3];
		oldJointMat[0] = jointMat[0];
		oldJointMat[1] = jointMat[1];
		oldJointMat[2] = jointMat[2];
		if (m_Axis == SCR_JointHingeEntityClass.AXIS_PITCH) // Pitch
			Math3D.AnglesToMatrix("90 0 0", rotMat);
		else if (m_Axis == SCR_JointHingeEntityClass.AXIS_YAW) // Yaw
			Math3D.AnglesToMatrix("0 90 0", rotMat);
		Math3D.MatrixMultiply3(oldJointMat, rotMat, jointMat);
	}
	
	//------------------------------------------------------------------------------------------------
	override void CreateJoint(IEntity parent, IEntity child, vector jointMat1[4], vector jointMat2[4])
	{
		// Create the joint
		m_Joint = PhysicsJoint.CreateHinge2(parent, child, jointMat1, jointMat2, m_CollisionBlocker, -1);
		PhysicsHingeJoint jointHinge = m_Joint;
		if (m_RotateIsFree)
			jointHinge.SetLimits(1, -1, m_Softness, m_BiasFactor, m_RelaxationFactor);
		else
			jointHinge.SetLimits(-m_RotateMax * Math.DEG2RAD, -m_RotateMin * Math.DEG2RAD, m_Softness, m_BiasFactor, m_RelaxationFactor);
	}
};
