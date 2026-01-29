[EntityEditorProps(category: "GameBase/Joints", description: "Physics joint - Cone Twist", sizeMin: "-0.05 -0.05 -0.05", sizeMax: "0.05 0.05 0.05", color: "0 0 255 255")]
class SCR_JointConeTwistEntityClass: SCR_JointBaseEntityClass
{
};

//------------------------------------------------------------------------------------------------
//! SCR_JointConeTwistEntity Class
//!
//! TODO: doc
//------------------------------------------------------------------------------------------------
class SCR_JointConeTwistEntity: SCR_JointBaseEntity
{
	[Attribute("0", UIWidgets.Slider, "Yaw swing range in degrees", "0 180 0.01")]
	float m_SwingSpanYaw;
	[Attribute("0", UIWidgets.Slider, "Pitch swing range in degrees", "0 180 0.01")]
	float m_SwingSpanPitch;
	[Attribute("0", UIWidgets.Slider, "Twist range in degrees (if below zero, can twist freely)", "-1 180 0.01")]
	float m_TwistSpan;
	[Attribute("0.5", UIWidgets.Slider, "Percentage of limits where movement is free, above this the limit is gradually enforced", "0 1 0.001")]
	float m_Softness;
	[Attribute("0.3", UIWidgets.Slider, "The strength with which the constraint resists limit violations", "0 1 0.001")]
	float m_BiasFactor;
	[Attribute("1", UIWidgets.Slider, "The lower the values, the less the constraint will fight velocities which violate the angular limits", "0 1 0.001")]
	float m_RelaxationFactor;
	
	//------------------------------------------------------------------------------------------------
	
	
	//------------------------------------------------------------------------------------------------
	override void DebugDisplay()
	{
		vector mat[4];
		GetTransform(mat);
		vector pos = mat[3];
		
		vector axisVec = mat[2];
		vector axisVec2 = mat[1];
		vector axisVec3 = mat[0];
		
		int coneSubDiv = Math.Ceil(Math.AbsFloat(Math.Sin((m_SwingSpanYaw + m_SwingSpanPitch) / 2 * Math.DEG2RAD)) * 50);
		int sliceSubDiv = 16;
		if (m_TwistSpan > 0)
			sliceSubDiv = Math.Ceil(m_TwistSpan / 360 * 16);
		
		CreateCone(pos, axisVec, m_SwingSpanYaw, m_SwingSpanPitch, 0.05, ARGB(255, 0, 255, 0), coneSubDiv, ShapeFlags.ONCE|ShapeFlags.NOZBUFFER);
		if (m_TwistSpan < 0)
			CreateCircleSlice(pos, axisVec, -axisVec3, -1, 1, 0.075, ARGB(255, 0, 255, 255), sliceSubDiv, ShapeFlags.ONCE|ShapeFlags.NOZBUFFER);
		else
			CreateCircleSlice(pos, axisVec, -axisVec3, 0, m_TwistSpan, 0.075, ARGB(255, 0, 255, 255), sliceSubDiv, ShapeFlags.ONCE|ShapeFlags.NOZBUFFER);
		Shape.Create(ShapeType.LINE, ARGB(255, 0, 255, 0), ShapeFlags.ONCE|ShapeFlags.NOZBUFFER, axisVec * -0.05 + pos, axisVec * 0.05 + pos);
		Shape.Create(ShapeType.LINE, ARGB(255, 0, 255, 0), ShapeFlags.ONCE|ShapeFlags.NOZBUFFER, axisVec2 * -0.05 + pos, axisVec2 * 0.05 + pos);
		Shape.Create(ShapeType.LINE, ARGB(255, 0, 255, 0), ShapeFlags.ONCE|ShapeFlags.NOZBUFFER, axisVec3 * -0.05 + pos, axisVec3 * 0.05 + pos);
	}
	
	//------------------------------------------------------------------------------------------------
	override void RotateJoint(inout vector jointMat[4])
	{
		vector oldJointMat[3], rotMat[3];
		oldJointMat[0] = jointMat[0];
		oldJointMat[1] = jointMat[1];
		oldJointMat[2] = jointMat[2];
		Math3D.AnglesToMatrix("-90 0 0", rotMat);
		Math3D.MatrixMultiply3(oldJointMat, rotMat, jointMat);
	}
	
	//------------------------------------------------------------------------------------------------
	override void CreateJoint(IEntity parent, IEntity child, vector jointMat1[4], vector jointMat2[4])
	{
		// Create the joint
		m_Joint = PhysicsJoint.CreateConeTwist(parent, child, jointMat1, jointMat2, m_CollisionBlocker, -1);
		PhysicsConeTwistJoint jointConeTwist = m_Joint;
		jointConeTwist.SetLimits(m_SwingSpanPitch * Math.DEG2RAD, m_SwingSpanYaw * Math.DEG2RAD, m_TwistSpan * Math.DEG2RAD, m_Softness, m_BiasFactor, m_RelaxationFactor);
	}
};
