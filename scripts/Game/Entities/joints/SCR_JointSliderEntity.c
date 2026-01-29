[EntityEditorProps(category: "GameBase/Joints", description: "Physics joint - Slider", sizeMin: "-0.05 -0.05 -0.05", sizeMax: "0.05 0.05 0.05", color: "0 0 255 255")]
class SCR_JointSliderEntityClass: SCR_JointBaseEntityClass
{
};

//------------------------------------------------------------------------------------------------
//! SCR_JointSliderEntity Class
//!
//! TODO: doc
//------------------------------------------------------------------------------------------------
class SCR_JointSliderEntity: SCR_JointBaseEntity
{
	[Attribute("0", UIWidgets.CheckBox, "Whether may rotate freely (ignores limits)")]
	bool m_RotateIsFree;
	[Attribute("0", UIWidgets.Slider, "Minimum rotate range in degrees (if minimum is higher than maximum, slider can rotate freely)", "-180 0 0.01")]
	float m_RotateMin;
	[Attribute("0", UIWidgets.Slider, "Maximum rotate range in degrees (if minimum is higher than maximum, slider can rotate freely)", "0 180 0.01")]
	float m_RotateMax;
	[Attribute("0", UIWidgets.Slider, "Minimum movement range in meters (if minimum is higher than maximum, slider can move along its axis freely)", "-50 0 0.01")]
	float m_MoveMin;
	[Attribute("0", UIWidgets.Slider, "Maximum movement range in meters (if minimum is higher than maximum, slider can move along its axis freely)", "0 50 0.01")]
	float m_MoveMax;
	//------------------------------------------------------------------------------------------------
	
	
	//------------------------------------------------------------------------------------------------
	override void DebugDisplay()
	{
		vector mat[4];
		GetTransform(mat);
		vector pos = mat[3];
		
		vector axisVec = mat[2];
		vector axisVec2 = mat[1];
		vector axisVec3 = -mat[0];
		const float addVisAng = 0; // TODO: check for good const usage
		
		int sliceSubDiv = 16;
		if (m_RotateMax > m_RotateMin)
			sliceSubDiv = Math.Ceil(Math.AbsFloat(-m_RotateMax - -m_RotateMin) / 360 * 16);
		
		CreateCircle(axisVec * m_MoveMin + pos, axisVec, 0.05, ARGB(255, 0, 255, 0), 16, ShapeFlags.ONCE|ShapeFlags.NOZBUFFER);
		CreateCircle(axisVec * m_MoveMax + pos, axisVec, 0.05, ARGB(255, 0, 255, 0), 16, ShapeFlags.ONCE|ShapeFlags.NOZBUFFER);
		if (m_RotateIsFree)
		{
			CreateCircle(axisVec * m_MoveMin + pos, axisVec, 0.075, ARGB(255, 0, 255, 255), 16, ShapeFlags.ONCE|ShapeFlags.NOZBUFFER);
			CreateCircle(axisVec * m_MoveMax + pos, axisVec, 0.075, ARGB(255, 0, 255, 255), 16, ShapeFlags.ONCE|ShapeFlags.NOZBUFFER);
		}
		else
		{
			CreateCircleSlice(axisVec * m_MoveMin + pos, axisVec, axisVec3, -m_RotateMax + addVisAng, -m_RotateMin + addVisAng, 0.075, ARGB(255, 0, 255, 255), sliceSubDiv, ShapeFlags.ONCE|ShapeFlags.NOZBUFFER);
			CreateCircleSlice(axisVec * m_MoveMax + pos, axisVec, axisVec3, -m_RotateMax + addVisAng, -m_RotateMin + addVisAng, 0.075, ARGB(255, 0, 255, 255), sliceSubDiv, ShapeFlags.ONCE|ShapeFlags.NOZBUFFER);
		}
		Shape.Create(ShapeType.LINE, ARGB(255, 0, 255, 0), ShapeFlags.ONCE|ShapeFlags.NOZBUFFER, axisVec * m_MoveMin + pos, axisVec * m_MoveMax + pos);
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
		m_Joint = PhysicsJoint.CreateSlider(parent, child, jointMat1, jointMat2, m_CollisionBlocker, -1);
		PhysicsSliderJoint jointSlider = m_Joint;
		jointSlider.SetLinearLimits(m_MoveMin, m_MoveMax);
		if (m_RotateIsFree)
			jointSlider.SetAngularLimits(1, -1);
		else
			jointSlider.SetAngularLimits(-m_RotateMax * Math.DEG2RAD, -m_RotateMin * Math.DEG2RAD);
	}
};
