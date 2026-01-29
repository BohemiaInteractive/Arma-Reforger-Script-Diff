[EntityEditorProps(category: "GameBase/Joints", description: "Physics joint - Fixed", sizeMin: "-0.05 -0.05 -0.05", sizeMax: "0.05 0.05 0.05", color: "0 0 255 255")]
class SCR_JointFixedEntityClass: SCR_JointBaseEntityClass
{
};

//------------------------------------------------------------------------------------------------
//! SCR_JointFixedEntity Class
//!
//! TODO: doc
//------------------------------------------------------------------------------------------------
class SCR_JointFixedEntity: SCR_JointBaseEntity
{
	//------------------------------------------------------------------------------------------------
	override void DebugDisplay()
	{
		vector mat[4];
		GetTransform(mat);
		vector pos = mat[3];
		
		vector axisVec = mat[0];
		vector axisVec2 = mat[1];
		vector axisVec3 = mat[2];
		Shape.Create(ShapeType.LINE, ARGB(255, 0, 255, 0), ShapeFlags.ONCE|ShapeFlags.NOZBUFFER, axisVec * -0.05 + pos, axisVec * 0.05 + pos);
		Shape.Create(ShapeType.LINE, ARGB(255, 0, 255, 0), ShapeFlags.ONCE|ShapeFlags.NOZBUFFER, axisVec2 * -0.05 + pos, axisVec2 * 0.05 + pos);
		Shape.Create(ShapeType.LINE, ARGB(255, 0, 255, 0), ShapeFlags.ONCE|ShapeFlags.NOZBUFFER, axisVec3 * -0.05 + pos, axisVec3 * 0.05 + pos);
	}
	
	//------------------------------------------------------------------------------------------------
	override void CreateJoint(IEntity parent, IEntity child, vector jointMat1[4], vector jointMat2[4])
	{
		m_Joint = PhysicsJoint.CreateFixed(parent, child, jointMat1[3], jointMat2[3], m_CollisionBlocker, -1);
	}
};
