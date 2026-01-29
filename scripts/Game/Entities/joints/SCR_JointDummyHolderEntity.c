//------------------------------------------------------------------------------------------------
class SCR_JointDummyHolderEntityClass: GenericEntityClass
{
};

//------------------------------------------------------------------------------------------------
//! SCR_JointDummyHolderEntity Class
//!
//! TODO: Doc
//------------------------------------------------------------------------------------------------
class SCR_JointDummyHolderEntity: GenericEntity 
{
	vector m_FixedMat[4];
	
	//------------------------------------------------------------------------------------------------
	void SetDummyTransform(vector mat[4])
	{
		m_FixedMat[0] = mat[0];
		m_FixedMat[1] = mat[1];
		m_FixedMat[2] = mat[2];
		m_FixedMat[3] = mat[3];
		SetTransform(mat);
	}
	
	//------------------------------------------------------------------------------------------------
	override event void EOnSimulate(IEntity owner, float timeSlice) //!EntityEvent.SIMULATE
	{
		Physics physics = owner.GetPhysics();
		if (!physics)
			return;
		
		SetTransform(m_FixedMat);
		physics.SetAngularVelocity(vector.Zero);
		physics.SetVelocity(vector.Zero);
	}
	
	//------------------------------------------------------------------------------------------------
	override event void EOnInit(IEntity owner) //!EntityEvent.INIT
	{
		GetTransform(m_FixedMat);
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_JointDummyHolderEntity(IEntitySource src, IEntity parent)
	{
		SetEventMask(EntityEvent.INIT|EntityEvent.SIMULATE);
		
		ref PhysicsGeomDef geoms[1];
		autoptr PhysicsGeomDef geom =  new PhysicsGeomDef("", PhysicsGeom.CreateSphere(0.1), "material/default", EPhysicsLayerDefs.VehicleCast);
		geoms[0] = geom;
		
		// Note: If using static geometry, the joint gets reoriented!
		//m_Physics = Physics.CreateStaticEx(this, geoms);
		Physics physics = Physics.CreateDynamicEx(this, vector.Zero, 999999, geoms);
		physics.SetLinearFactor(vector.Zero);
		physics.SetDamping(1000, 1000);
		physics.SetAngularVelocity(vector.Zero);
		physics.SetVelocity(vector.Zero);
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_JointDummyHolderEntity()
	{
	}
};

