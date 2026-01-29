// *************************************************************************************
// ! Animation Graph Constants Table
// *************************************************************************************
class SCR_CharacterCommandSwimST
{
	//------------------------------------------------------------------------------------------------
	// constructor
	void SCR_CharacterCommandSwimST(CharacterAnimationComponent pAnimationComponent)
	{
		m_CmdStartSwimming 	= pAnimationComponent.BindCommand("CMD_Swim");		
		m_VarSpeed			= pAnimationComponent.BindVariableFloat("MovementSpeed");
		m_VarDirection		= pAnimationComponent.BindVariableFloat("MovementDirection");
		m_TagIsSwimming 	= pAnimationComponent.BindTag("TagSwimming");
	}	

	TAnimGraphCommand 		m_CmdStartSwimming;		//! command to start swimming
	TAnimGraphVariable 		m_VarSpeed;				//! float variable - 0,1,2,3 speed
	TAnimGraphVariable 		m_VarDirection;			//! float variable - angle 
	TAnimGraphTag			m_TagIsSwimming;		//! 
}

// *************************************************************************************
// ! Fully Working Swimming implemented by AnimPhysCommandScripted
// *************************************************************************************
class SCR_CharacterCommandSwim : ScriptedCommand
{
	//------------------------------------------------------------------------------------------------
	// constructor
	void SCR_CharacterCommandSwim(BaseAnimPhysComponent pAnimPhysComponent, SCR_CharacterCommandSwimST pTable, ChimeraCharacter pCharacter, CharacterControllerComponent pController)
	{
		m_pCharacter 			= pCharacter;
		m_AnimationComponent 	= CharacterAnimationComponent.Cast(pAnimPhysComponent);		
		m_Input					= pController.GetInputContext();
		m_Table 				= pTable;
	}

	//------------------------------------------------------------------------------------------------
	void StartSwimming()
	{
		PreAnim_CallCommand(m_Table.m_CmdStartSwimming, 1, 1);
	}

	//------------------------------------------------------------------------------------------------
	void EndSwimming()
	{
		PreAnim_CallCommand(m_Table.m_CmdStartSwimming, -1, 0);
	}
	
	//------------------------------------------------------------------------------------------------
	//! empty, does nothing
	void UpdateWaterDepth()
	{
		//vector 	pp = m_pPlayer.GetPosition();
		//vector  wl = HumanCommandSwim.WaterLevelCheck(m_pPlayer, pp);

		//m_fWaterLevelDepth = wl[0];		// whats water depth at player's position
		//m_fCharacterDepth = wl[1];		// whats character depth  at player's position
	}

	//------------------------------------------------------------------------------------------------
	override void OnActivate()
	{
		StartSwimming();
		m_AnimationComponent.PhysicsEnableGravity(false);
	}

	//------------------------------------------------------------------------------------------------
	override void OnDeactivate()
	{
		Print("SCR_CharacterCommandSwim::OnDeactivate");
		m_AnimationComponent.PhysicsEnableGravity(true);
	}

	//------------------------------------------------------------------------------------------------
	// called to set values to animation graph processing 
	override void PreAnimUpdate(float pDt)
	{
	    if (m_bNeedFinish)
		{
			EndSwimming();
			return;
		}
		
		// Print("SCR_CharacterCommandSwim::PreAnimUpdate: " + pDt.ToString());

		vector 	localDir;
		float 	speed;
		float 	heading;

		// get int movement 
		m_Input.GetMovement(speed, localDir);
		heading = m_Input.GetHeadingAngle();

		//! if movement moves front - remap 0,1,2,3 to 0,2,3 (in animation graph)
		float 	animSpeed = 0;

		if (speed > 0)
		{
			if (localDir[2] > 0.3)
			{
				if (speed > 2.0)
				{
					animSpeed = 3.0;
				}
				else
				{
					animSpeed = 2.0;
				}
			}
		}

		//! handle animation swimming speed 
		PreAnim_SetFloat(m_Table.m_VarSpeed, animSpeed);		// sets motion variable speed 

		//! rotate character (directly by heading angle - with filtering)
/*		if (animSpeed <= 0)
		{
			// idle
			PreAnim_SetFilteredHeading(heading, m_pSettings.m_fAlignIdleTimeout, m_pSettings.m_fAlignIdleMaxChanged);
		}
		else if (animSpeed <= 2)
		{	
			// normal speed
			PreAnim_SetFilteredHeading(heading, m_pSettings.m_fAlignSlowTimeout, m_pSettings.m_fAlignSlowMaxChanged);
		}
		else
		{
			// sprint
			PreAnim_SetFilteredHeading(heading, m_pSettings.m_fAlignFastTimeout, m_pSettings.m_fAlignFastMaxChanged);
		}*/
	
		//! update time
		m_fTime += pDt;
	}

	//------------------------------------------------------------------------------------------------
	//! called to change local translations / rotations
	//! called to get events / tags from animation graph processing 
	override void 	PrePhysUpdate(float pDt)
	{
		Print("SCR_CharacterCommandSwim::PrePhysUpdate: " + pDt.ToString());

		UpdateWaterDepth();
		// Print("Char Depth: " + m_fCharacterDepth.ToString());

		//! move from m_fCharacterDepth to m_pSettings->m_fWaterLevelSwim
		float filtCharDepth = Math.SmoothCD(m_fCharacterDepth, m_fWaterLevelSwim, m_fWaterAlignV, 0.3, 1.0, pDt);		
		// Print("Filt Depth: " + filtCharDepth.ToString());

		//! get filtered part 
		float delta 		= m_fCharacterDepth - filtCharDepth;
		// Print("Filt Delta: " + delta);
		
		//! set filtered part as movement 
		vector trans		= vector.Zero;

		//! get local translations 
		PrePhys_GetTranslation(trans);		// in case this returns false ... trans is still zero 

		// waves 
		delta += Math.Sin(m_fTime * 3) * 0.003;
		
		//! height correction 
		trans[1] = trans[1] + delta;

		//! XZ side translation 
		vector 	locDir;
		float 	speed;
		float 	swimSpeedX = 0, swimSpeedZ = 0;

		// get int movement 
		m_Input.GetMovement(speed, locDir);

		if (speed > 0)
		{
			if (locDir[2] < 0)	// backwards ? 
			{
				swimSpeedZ = locDir[2] * m_fMovementSpeed;
			}

			swimSpeedX = locDir[0] * m_fMovementSpeed;
		} 

		// filter velocities			
		m_fSpeedX = Math.SmoothCD(m_fSpeedX, swimSpeedX, m_fSpeedXV, m_fMovementSpeedFltTime, m_fMovementSpeedFltMaxChange, pDt);		
		m_fSpeedZ = Math.SmoothCD(m_fSpeedZ, swimSpeedZ, m_fSpeedZV, m_fMovementSpeedFltTime, m_fMovementSpeedFltMaxChange, pDt);		

		// add velocity
		trans[0] = trans[0] + m_fSpeedX * pDt;
		trans[2] = trans[2] + m_fSpeedZ * pDt;

		//!
		PrePhys_SetTranslation(trans);
	}

	//------------------------------------------------------------------------------------------------
	//! called when all animation / pre phys update is handled
	override bool PostPhysUpdate(float pDt)
	{
		Print("SCR_CharacterCommandSwim::PostPhysUpdate: " + pDt.ToString());

		if (m_bNeedFinish)
		{
			return false;
		}

		//! finish if water level is smaller than some value (around 1m of depth)
		if (m_fWaterLevelDepth < m_fWaterLevelOut)
		{
			m_bNeedFinish = true;	// let it run 1 frame more
		}

		return true;	// handled with SetFlagFinished();
	}

	ChimeraCharacter					m_pCharacter;
	CharacterInputContext 				m_Input;
	CharacterAnimationComponent			m_AnimationComponent;
	
	SCR_CharacterCommandSwimST		 	m_Table;
	float 								m_fTime;
	bool								m_bNeedFinish;

	float 								m_fWaterLevelDepth;		//! water depth
	float 								m_fCharacterDepth;		//! char's depth
	float 								m_fWaterAlignV;			//! filter value 

	float 								m_fSpeedX;				//! x speed
	float 								m_fSpeedZ;				//! x speed
	float 								m_fSpeedXV;				//! filter value 
	float 								m_fSpeedZV;				//! filter value 
	
	//! settings
	float		m_fMovementSpeed = 0.7;
	float 		m_fMovementSpeedFltTime = 0.3;
	float		m_fMovementSpeedFltMaxChange = 1;
	float		m_fWaterLevelSwim = 1.4;				//!< when swimming - entity position depth (1.4 m)
	float		m_fWaterLevelIn = 1.5;					//!< when entering water - what level cases swimming (1.5m)
	float		m_fWaterLevelOut = 1.3;
}
