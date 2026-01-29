[EntityEditorProps(category: "GameBase/Joints", description: "Physics joint base", sizeMin: "-0.05 -0.05 -0.05", sizeMax: "0.05 0.05 0.05", color: "0 0 255 255")]
class SCR_JointBaseEntityClass: GenericEntityClass
{
};

//------------------------------------------------------------------------------------------------
//! SCR_JointBaseEntity Class
//! 
//! TODO: doc
//------------------------------------------------------------------------------------------------
class SCR_JointBaseEntity: GenericEntity
{
	[Attribute("-1", UIWidgets.ComboBox, "Snap joint to bone in parent (one-time)", "")]
	int SNAP_TO_PARENT_BONE;
	//[Attribute("-1", UIWidgets.ComboBox, "Snap child to joint by bone in child (one-time)", "")]
	//int SNAP_CHILD_BY_JOINT;
	[Attribute("1", UIWidgets.CheckBox, "Whether to create collision blocker between parent and child" )]
	bool m_CollisionBlocker;
	
	IEntity m_JointParent = null;
	IEntity m_JointChild = null;
	SCR_JointDummyHolderEntity m_JointDummy = null;
	
	PhysicsJoint m_Joint = null;
	
	// TODO:
	// - Do stress and breakage (in engine)
	// - SNAP_CHILD_BY_JOINT does not work! After setting of the property, child is NULL!	

	//------------------------------------------------------------------------------------------------
	void MoveJointToParentBone()
	{
		if (!m_JointParent)
			return;
		
		if (SNAP_TO_PARENT_BONE <= 0)
			return;
		
		vector boneMat[4];
		
		m_JointParent.GetAnimation().GetBoneMatrix(SNAP_TO_PARENT_BONE, boneMat);
		
		vector angles = Math3D.MatrixToAngles(boneMat);
		vector pos = boneMat[3];
		
		// TODO: Deselect or at least reselect joint after doing following steps
	#ifdef WORKBENCH
		WorldEditorAPI worldEditorAPI = _WB_GetEditorAPI();
		worldEditorAPI.BeginEntityAction("Move to parent bone", "");
		worldEditorAPI.ClearEntitySelection();
		
		IEntitySource src = _WB_GetEditorAPI().EntityToSource(this);
		worldEditorAPI.SetVariableValue(src, null, "coords", pos.ToString(false));
		worldEditorAPI.SetVariableValue(src, null, "angles", string.Format("%1 %2 %3", angles[1], angles[0], angles[2]));
		worldEditorAPI.SetVariableValue(src, null, "SNAP_TO_PARENT_BONE", "-1");
		worldEditorAPI.EndEntityAction("Move to parent bone");
	#endif
	}
	
	//------------------------------------------------------------------------------------------------
	/*void MoveChildToJointByBone()
	{
		Print(m_JointChild);
		if (!m_JointChild)
			return;
		
		if (SNAP_CHILD_BY_JOINT <= 0)
			return;

		WorldEditorAPI worldEditorAPI = _WB_GetEditorAPI();

		// Special case (origin)
		if (SNAP_CHILD_BY_JOINT == 999999)
		{
			// TODO: Deselect or at least reselect joint after doing following steps
			worldEditorAPI.BeginEntityAction("Move child to joint by bone", "");
			worldEditorAPI.ClearEntitySelection();
			worldEditorAPI.SetVariableValue(m_JointChild, null, "coords", "0 0 0");
			worldEditorAPI.SetVariableValue(m_JointChild, null, "angles", "0 0 0");
			worldEditorAPI.SetVariableValue(this, null, "SNAP_CHILD_BY_JOINT", "-1");
			worldEditorAPI.EndEntityAction("Move child to joint by bone");
		}
		else
		{
			vector boneMat[4], baseMat[4], endBoneMat[4];
			
			GetBoneMatrix(m_JointChild, SNAP_CHILD_BY_JOINT, boneMat);
			Math3D.MatrixIdentity4(baseMat);
			Math3D.MatrixInvMultiply4(baseMat, boneMat, endBoneMat);
			
			
			vector angles = Math3D.MatrixToAngles(endBoneMat);
			vector pos = endBoneMat[3];
			
			// TODO: Deselect or at least reselect joint after doing following steps
			worldEditorAPI.BeginEntityAction("Move child to joint by bone", "");
			worldEditorAPI.ClearEntitySelection();
			worldEditorAPI.SetVariableValue(m_JointChild, null, "coords", pos.ToString(false));
			worldEditorAPI.SetVariableValue(m_JointChild, null, "angles", string.Format("%1 %2 %3", angles[1], angles[0], angles[2]));
			worldEditorAPI.SetVariableValue(this, null, "SNAP_CHILD_BY_JOINT", "-1");
			worldEditorAPI.EndEntityAction("Move child to joint by bone");
		}
	}*/
	
	//------------------------------------------------------------------------------------------------
	#ifdef WORKBENCH
		//------------------------------------------------------------------------------------------------
		override array<ref ParamEnum> _WB_GetUserEnums(string varName, IEntitySource src)
		{
			if (varName == "SNAP_TO_PARENT_BONE" && m_JointParent)
				return SCR_Global.GetBonesAsParamEnums(m_JointParent);
			/*if (varName == "SNAP_CHILD_BY_JOINT" && m_JointChild)
			{
				array<ref ParamEnum> retEnums = SCR_Global.GetBonesAsParamEnums(m_JointChild);
				retEnums.Insert(new ParamEnum("ORIGIN", "999999", "")); // Always have ORIGIN as an option
				return retEnums;
			}*/
			
			return null;
		}
		
		//------------------------------------------------------------------------------------------------
		override int _WB_GetAfterWorldUpdateSpecs(IEntitySource src)
		{
			return EEntityFrameUpdateSpecs.CALL_WHEN_ENTITY_VISIBLE;
		}
		
		//------------------------------------------------------------------------------------------------
		override void _WB_AfterWorldUpdate(float timeSlice)
		{
			if (m_JointParent)
				Shape.CreateArrow(m_JointParent.GetOrigin(), GetOrigin(), 0.05, ARGB(255, 128, 128, 0), ShapeFlags.ONCE|ShapeFlags.NOZBUFFER);
			if (m_JointChild)
				Shape.CreateArrow(GetOrigin(), m_JointChild.GetOrigin(), 0.05, ARGB(255, 128, 128, 0), ShapeFlags.ONCE|ShapeFlags.NOZBUFFER);
			DebugDisplay();
		}
	#endif
	
	//------------------------------------------------------------------------------------------------
	void DestroyJoint()
	{
		if (m_Joint)
		{
			// TODO: Remove this once deletion of joints is ensured before physics scene deletion
			if (m_JointParent || m_JointChild)
				m_Joint.Destroy();
		}
		m_Joint = null;
		
		if (m_JointDummy)
			delete m_JointDummy;
		m_JointDummy = null;
	}
	
	//------------------------------------------------------------------------------------------------
	void TryCreateJoint(bool showErrors)
	{
		DestroyJoint();
		
		if (!m_JointParent && !m_JointChild)
		{
			if (showErrors)
			{
				Print("JointHingeEntity::CreateJoint: ERROR - cannot create joint, no parent and no child!");
				CreateFailedJointDebug(128, 255, 0, 0);
			}
			return;
		}
		
		Physics parentPhysics = null;
		Physics childPhysics = null;
		if (m_JointParent)
			parentPhysics = m_JointParent.GetPhysics();
		if (m_JointChild)
			childPhysics = m_JointChild.GetPhysics();
		if (!parentPhysics && !childPhysics)
		{
			if (showErrors)
			{
				Print("JointHingeEntity::CreateJoint: ERROR - cannot create joint, neither parent nor child have physics!");
				CreateFailedJointDebug(128, 255, 0, 0);
			}
			return;
		}
		
		if (parentPhysics && childPhysics && !parentPhysics.IsDynamic() && !childPhysics.IsDynamic())
		{
			if (showErrors)
			{
				Print("JointHingeEntity::CreateJoint: ERROR - cannot create joint, neither parent nor child are dynamic!");
				CreateFailedJointDebug(128, 255, 0, 0);
			}
			return;
		}
		
		IEntity parent = m_JointParent;
		IEntity child = m_JointChild;
		
		// Nullify parent
		if (parent && parentPhysics && !parentPhysics.IsDynamic())
			parent = null;
		if (child && childPhysics && !childPhysics.IsDynamic())
			child = null;
		
		if (showErrors && !parent && !child)
		{
			Print("JointHingeEntity::CreateJoint: ERROR - cannot create joint, no connected dynamic entities!");
			CreateFailedJointDebug(128, 255, 128, 0);
		}
		
		if (!parent || !child) // We have no parent or child, so create a dummy joint attachment point
		{
			m_JointDummy = SCR_JointDummyHolderEntity.Cast(GetGame().SpawnEntity(SCR_JointDummyHolderEntity));
			
			vector jointDummyMat[4];
			GetTransform(jointDummyMat);
			m_JointDummy.SetDummyTransform(jointDummyMat);
			
			float mass = 0;
			if (parent)
				mass = parentPhysics.GetMass();
			else
				mass = childPhysics.GetMass();
			
			m_JointDummy.GetPhysics().SetMass(mass * 1000);
			
			
			// Link the static dummy point
			if (parent)
				child = m_JointDummy;
			else
				parent = m_JointDummy;
		}
		
		// Ensure static entity is the child (must be 'ent2' for the API)
		if (!parentPhysics || !parentPhysics.IsDynamic())
		{
			IEntity tmp = parent;
			parent = child;
			child = tmp;
		}
		
		// First get matrices of parent, child and joint
		vector parentMat[4], childMat[4], jointMat[4];
		parent.GetTransform(parentMat);
		child.GetTransform(childMat);
		GetTransform(jointMat);
		
		// Rotate the joint if necessary (inout function)
		RotateJoint(jointMat);
		
		// Now get local matrices of parent and child relative to joint
		vector jointMatParentLocal[4], jointMatChildLocal[4];
		Math3D.MatrixInvMultiply4(parentMat, jointMat, jointMatParentLocal);
		Math3D.MatrixInvMultiply4(childMat, jointMat, jointMatChildLocal);
		
		// Subtract center of mass positions
		if (parentPhysics)
			jointMatParentLocal[3] = jointMatParentLocal[3] - parentPhysics.GetCenterOfMass();
		else
			jointMatParentLocal[3] = jointMatParentLocal[3];
		if (childPhysics)
			jointMatChildLocal[3] = jointMatChildLocal[3] - childPhysics.GetCenterOfMass();
		else
			jointMatChildLocal[3] = jointMatChildLocal[3];
		
		// Create the joint
		CreateJoint(parent, child, jointMatParentLocal, jointMatChildLocal);
	}
	
	//------------------------------------------------------------------------------------------------
	void RotateJoint(inout vector jointMat[4])
	{
	}
	
	//------------------------------------------------------------------------------------------------
	void CreateJoint(IEntity parent, IEntity child, vector jointMat1[4], vector jointMat2[4])
	{
	}
	
	//------------------------------------------------------------------------------------------------
	void CreateFailedJointDebug(int a, int r, int g, int b)
	{
		int color = ARGB(a, r, g, b);
		vector myPos = GetOrigin();
		Shape.Create(ShapeType.BBOX, color, ShapeFlags.NOOUTLINE|ShapeFlags.NOZBUFFER|ShapeFlags.TRANSP, "-0.1 0.2 -0.1" + myPos, "0.1 1.2 0.1" + myPos);
		Shape.Create(ShapeType.BBOX, color, ShapeFlags.NOOUTLINE|ShapeFlags.NOZBUFFER|ShapeFlags.TRANSP, "-0.1 -0.1 -0.1" + myPos, "0.1 0.1 0.1" + myPos);
	}
	
	//------------------------------------------------------------------------------------------------
	void DebugDisplay()
	{
	}
	
	//------------------------------------------------------------------------------------------------
	override event void EOnInit(IEntity owner) //!EntityEvent.INIT
	{
		m_JointChild = GetChildren();
		
		TryCreateJoint(GetGame().GetGameStarted());
		
		MoveJointToParentBone();
		//MoveChildToJointByBone();
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_JointBaseEntity(IEntitySource src, IEntity parent)
	{
		m_JointParent = parent;
		
		SetEventMask(EntityEvent.INIT);
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_JointBaseEntity()
	{
		DestroyJoint();
	}
};
