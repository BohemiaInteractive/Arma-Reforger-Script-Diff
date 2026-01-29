class SCR_CharacterCommandFly : ScriptedCommand
{
	//------------------------------------------------------------------------------------------------
	// constructor
	void SCR_CharacterCommandFly(BaseAnimPhysComponent pAnimPhysComponent, SCR_CharacterCommandSwimST pTable, ChimeraCharacter pCharacter, CharacterControllerComponent pController)
	{
		m_pCharacter 			= pCharacter;
		m_pController			= pController;
		m_AnimationComponent 	= CharacterAnimationComponent.Cast(pAnimPhysComponent);
		
		m_Input					= pController.GetInputContext();
		m_Table 				= pTable;
	}

	//------------------------------------------------------------------------------------------------
	override void OnActivate()
	{
		m_AnimationComponent.PhysicsEnableGravity(false);
	}

	//------------------------------------------------------------------------------------------------
	override void OnDeactivate()
	{
		Print("SCR_CharacterCommandFly::OnDeactivate");
		m_AnimationComponent.PhysicsEnableGravity(true);
	}

	//------------------------------------------------------------------------------------------------
	// called to set values to animation graph processing 
	override void PreAnimUpdate(float pDt)
	{
	}

	//------------------------------------------------------------------------------------------------
	//! called to change local translations / rotations
	//! called to get events / tags from animation graph processing 
	override void PrePhysUpdate(float pDt)
	{
		Print("SCR_CharacterCommandFly::PrePhysUpdate: " + pDt.ToString());
		
		vector trans = vector.Zero;

		// get local translations 
		PrePhys_GetTranslation(trans);		// in case this returns false ... trans is still zero 
		
		//trans	= vector.Zero;

		// XZ side translation 
		vector 	locDir;
		//float 	speed;
		float 	swimSpeedX = 0, swimSpeedZ = 0, swimSpeedY = 0;

		// get int movement 
		vector movDir = m_pController.GetMovementDirWorld();
		//m_Input.GetMovement(speed, locDir);

		if (movDir.Length() > 0)
		{
			swimSpeedZ = movDir[2] * m_fMovementSpeed;
			swimSpeedX = movDir[0] * m_fMovementSpeed;
		} 

		if (m_Input.GetJump() > 0)
		{
			moveUp = !moveUp;
		}
		else
		{
			// m_bNeedFinish = true;
		}
		
		if( moveUp )
			swimSpeedY = 2;


		// filter velocities			
		m_fSpeedX = Math.SmoothCD(m_fSpeedX, swimSpeedX, m_fSpeedXV, m_fMovementSpeedFltTime, m_fMovementSpeedFltMaxChange, pDt);		
		m_fSpeedY = Math.SmoothCD(m_fSpeedY, swimSpeedY, m_fSpeedYV, m_fMovementSpeedFltTime, m_fMovementSpeedFltMaxChange, pDt);		
		m_fSpeedZ = Math.SmoothCD(m_fSpeedZ, swimSpeedZ, m_fSpeedZV, m_fMovementSpeedFltTime, m_fMovementSpeedFltMaxChange, pDt);		

		// add velocity
		trans[0] = trans[0] + m_fSpeedX * pDt;
		trans[1] = trans[1] + m_fSpeedY * pDt;
		trans[2] = trans[2] + m_fSpeedZ * pDt;

		PrePhys_SetTranslation(trans);
	}

	//------------------------------------------------------------------------------------------------
	// called when all animation / pre phys update is handled
	override bool PostPhysUpdate(float pDt)
	{
		Print("SCR_CharacterCommandFly::PostPhysUpdate: " + pDt.ToString());

		//!
		if (m_bNeedFinish)
		{
			return false;
		}

		return true;	// handled with SetFlagFinished();
	}

	ChimeraCharacter					m_pCharacter;
	CharacterControllerComponent		m_pController;
	CharacterInputContext 				m_Input;
	CharacterAnimationComponent			m_AnimationComponent;
	SCR_CharacterCommandSwimST		 	m_Table;
	float 								m_fTime;
	bool								m_bNeedFinish;

	float 								m_fSpeedX;				//! x speed
	float 								m_fSpeedY;				//! x speed
	float 								m_fSpeedZ;				//! x speed
	float 								m_fSpeedXV;				//! filter value 
	float 								m_fSpeedYV;				//! filter value 
	float 								m_fSpeedZV;				//! filter value 

	//! settings
	float		m_fMovementSpeed = 25;
	float 		m_fMovementSpeedFltTime = 0.3;
	float		m_fMovementSpeedFltMaxChange = 100;
	
	//! state
	bool moveUp = false;
}
