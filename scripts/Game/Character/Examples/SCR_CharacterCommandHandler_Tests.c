[ComponentEditorProps(category: "GameScripted/Character", description: "Scripted character command handler FOR TESTING", icon: HYBRID_COMPONENT_ICON)]
class SCR_CharacterCommandHandlerComponent_TestsClass : SCR_CharacterCommandHandlerComponentClass
{
}

class SCR_CharacterCommandHandlerComponent_Tests : SCR_CharacterCommandHandlerComponent
{
	//------------------------------------------------------------------------------------------------
	override bool HandleDeath(CharacterInputContext pInputCtx, float pDt, int pCurrentCommandID, bool pCurrentCommandFinished)
	{
		if ( pCurrentCommandID == ECharacterCommandIDs.DEATH )
		{
			if ( pCurrentCommandFinished )
				SetSimulationDisabled(true);

			return true;
		}

		float die = pInputCtx.GetDie();
		if ( die != 0.0 )
		{
			GetCommandModifier_Death().StartDeath();

			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
 	override bool HandleFinishedCommands(bool pCurrentCommandFinished)
	{
		if( pCurrentCommandFinished )
		{
			// start falling ? (can happen after climbing)
			if( m_CharacterAnimComp.PhysicsIsFalling() )
			{
				StartCommand_Fall(0);
				vector charTM[4];
				m_OwnerEntity.GetTransform(charTM);
				m_fFallYDiff = charTM[3][1];
				return true;
			}
			
			//! show weapon (ladders, swimming and other commands are hiding it)
			m_WeaponManager.SetVisibleCurrentWeapon(true);
			//! default behavior after finish is to start move
			StartCommand_Move();
			return true;
		}

		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool HandleSwimming(CharacterInputContext pInputCtx, float pDt, int pCurrentCommandID)
	{
		//! get water level 
		vector charTM[4];
		m_OwnerEntity.GetTransform(charTM);
		vector pp = charTM[3];
		float waterDepth = pInputCtx.GetWaterBodyApproxDepth();
		float waterCharSubmersion = pInputCtx.GetWaterCharacterSubmersion();

		CharacterCommandSwimSettings swimSettings = GetCommandSwimSettings();
		if( pCurrentCommandID == ECharacterCommandIDs.SWIM )
		{
			if( waterDepth < swimSettings.m_fWaterLevelOut )
			{
				CharacterCommandSwim sc = GetCommandSwim();
				sc.StopSwimming();
				return true;
			}

			return true;
		}

		//! if total water depth >= 1.5m && character is 1.5m in water 
		if( waterDepth >= swimSettings.m_fWaterLevelIn && waterCharSubmersion >= swimSettings.m_fWaterLevelIn )
		{
			m_WeaponManager.SetVisibleCurrentWeapon(false);
			StartCommand_Swim();
			return true;
		}

		//! now movement
		if( pCurrentCommandID == ECharacterCommandIDs.MOVE )
		{
			CharacterCommandMove mc = GetCommandMove();

			if( waterCharSubmersion > swimSettings.m_fToErectLevel )
			{
				m_CharacterControllerComp.ForceStanceUp(0);
			}
			else if( waterCharSubmersion > swimSettings.m_fToCrouchLevel )
			{
				m_CharacterControllerComp.ForceStanceUp(1);
			}
		}

		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool HandleLadders(CharacterInputContext pInputCtx, float pDt, int pCurrentCommandID)
	{
		//! ERC stance only
		if (m_MovementState.m_iStanceIdx != CharacterStance.STANCE_ERECT)
			return false;

		//! not sprinting
		if (m_MovementState.m_fMovement > 2)
			return false;

		if (pCurrentCommandID == ECharacterCommandIDs.LADDER )
			return true;
		
		vector charTM[4];
		m_OwnerEntity.GetTransform(charTM);

		vector mins = "-0.5 -0.5 -0.5";
		vector maxs = "0.5 2.0 0.5";
		autoptr TraceBox trace = new TraceBox;
		trace.Exclude = m_OwnerEntity;
		trace.LayerMask = EPhysicsLayerDefs.Ladder;
		trace.Flags = TraceFlags.ENTS | TraceFlags.WORLD;
		trace.Start = charTM[3];
		trace.Mins = mins;
		trace.Maxs = maxs;

		if (m_OwnerEntity.GetWorld().TracePosition(trace, null) < 0)
		{
			if (!trace.TraceEnt)
				return false;
			
			GenericEntity ge = GenericEntity.Cast(trace.TraceEnt);
			if (!ge)
				return false;
			
			LadderComponent ladder = LadderComponent.Cast(ge.FindComponent(LadderComponent));
			if (ladder)
			{
				vector ladderTM[4];
				ge.GetTransform(ladderTM);

				if (ladder.IsAbove(charTM[3]))
					ladderTM[2] = ladderTM[2] * -1;
				
				float cosFi = vector.Dot(charTM[2], ladderTM[2]);
				vector cross = charTM[2] * ladderTM[2];
				float dirAngle = Math.Acos(cosFi) * Math.RAD2DEG;
		
				if( cross[1] < 0 )
					dirAngle = -dirAngle;

				float movementSpeed;
				vector movementDir;

				//! get info about current movement
				pInputCtx.GetMovement(movementSpeed, movementDir);
				
				//! angle of direction and fwd movement check
				if(((dirAngle <= 30 && dirAngle >= -30) || Math.AbsFloat(dirAngle) == 90) && movementDir[2] > 0)
				{
					m_WeaponManager.SetVisibleCurrentWeapon(false);
					StartCommand_Ladder(ladder);
					return true;
				}
			}
		}

		return false;		
	}

	//------------------------------------------------------------------------------------------------
	override bool HandleClimbing(CharacterInputContext pInputCtx, float pDt, int pCurrentCommandID)
	{
		if( pCurrentCommandID == ECharacterCommandIDs.CLIMB )
			return true;

		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool HandleFalling(CharacterInputContext pInputCtx, float pDt, int pCurrentCommandID)
	{
		if ( pCurrentCommandID == ECharacterCommandIDs.FALL )
		{
			vector charTM[4];
			int landType = ELandType.LANDTYPE_NONE;
			
			CharacterCommandFall fallCmd = GetCommandFall();
			
			if( fallCmd && fallCmd.PhysicsLanded() )
			{
				m_OwnerEntity.GetTransform(charTM);
				m_fFallYDiff = m_fFallYDiff - charTM[3][1];

				float fFallYDiff = Math.AbsFloat(m_fFallYDiff);
				if( fFallYDiff < 0.5 )
					landType = ELandType.LANDTYPE_NONE;
				else if( fFallYDiff < 1.0 )
					landType = ELandType.LANDTYPE_LIGHT;
				else if( fFallYDiff < 2.0 )
					landType = ELandType.LANDTYPE_MEDIUM;
				else
					landType = ELandType.LANDTYPE_HEAVY;

				fallCmd.Land(landType, m_fFallYDiff);			
			}

			return true;
		}

		if ( m_CharacterAnimComp && m_CharacterAnimComp.PhysicsIsFalling() )
		{
			StartCommand_Fall(0);
			
			vector charTM[4];
			m_OwnerEntity.GetTransform(charTM);
			m_fFallYDiff = charTM[3][1];

			return true;
		}

		return false;
	}
	
	/*
	//------------------------------------------------------------------------------------------------
	override bool HandleDamageHit(CharacterInputContext pInputCtx, float pDt, int pCurrentCommandID)
	{
		if( m_iDamageHitAnimType != -1 )
		{
			if( m_bHitLight )
				AddCommandModifier_Damage(m_iDamageHitAnimType, m_fHitDirection);

			m_iDamageHitAnimType = -1;
		}

		return false; // return true in case of full body damage
	}
	*/
	
	/*
	//------------------------------------------------------------------------------------------------
	override bool HandleWeapons(CharacterInputContext pInputCtx, float pDt, int pCurrentCommandID)
	{
		if( !m_CommandWeapons )
			return false;

		if( HandleWeaponChange(pInputCtx, pDt, pCurrentCommandID) )
			return true;

		if( HandleWeaponObstructionDefault(pInputCtx, pDt, pCurrentCommandID) )
			return true;

		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool HandleWeaponChange(CharacterInputContext pInputCtx, float pDt, int pCurrentCommandID)
	{
		switch( m_CommandWeapons.GetWeaponChangeState() )
		{
		case EWeaponChangeState.WEAPON_CHANGE_NONE:
		{
			if( pInputCtx.IsWeaponChange() )
			{
				m_CommandWeapons.SetWeaponChangeState(EWeaponChangeState.WEAPON_CHANGE_HIDE);
				m_pWeaponManager.GetCurrentWeapon().SetSafetyFireMode(true);
				if( m_pWeaponManager.IsWeaponItemAttached() )
				{
					m_CommandWeapons.StartWeaponHide(0);
				}
				return true;
			}
		}
		break;
		case EWeaponChangeState.WEAPON_CHANGE_HIDE:
		{
			if( !m_CommandWeapons.IsChangingWeaponHideTag() || !m_pWeaponManager.IsWeaponItemAttached() )
			{
				m_CommandWeapons.SetWeaponChangeState(EWeaponChangeState.WEAPON_CHANGE_SWITCH);
				auto weapon = m_pWeaponManager.FindWeapon(pInputCtx.GetWantedWeaponIdx());
				if( m_CharacterControllerComp.TakeWeaponInHands(weapon) )
				{
					m_pAnimationComponent.TrackPoseUpdate();
				}
			}
			return true;
		}
		break;
		case EWeaponChangeState.WEAPON_CHANGE_SWITCH:
		{
			// Wait for pose to be updated before start showing weapon 
			// otherwise IK pose will be broken when showing weapon
			if( m_pAnimationComponent.DidPoseUpdated() )
			{
				if( m_pWeaponManager.IsWeaponItemAttached() )
				{
					m_CommandWeapons.StartWeaponShow(0);
					m_CommandWeapons.SetWeaponChangeState(EWeaponChangeState.WEAPON_CHANGE_SHOW);
				}
				else
				{
					m_CommandWeapons.SetWeaponChangeState(EWeaponChangeState.WEAPON_CHANGE_EMPTY);
				}
			}
			return true;
		}
		break;
		case EWeaponChangeState.WEAPON_CHANGE_SHOW:
		{
			if( !m_CommandWeapons.IsChangingWeaponTag() )
			{
				m_pAnimationComponent.RefreshIkPose();
				m_CommandWeapons.SetWeaponChangeState(EWeaponChangeState.WEAPON_CHANGE_NONE);
				m_pWeaponManager.GetCurrentWeapon().SetSafetyFireMode(false);
				return false;
			}
			return true;
		}
		break;
		case EWeaponChangeState.WEAPON_CHANGE_CANCEL:
		{
			m_CommandWeapons.StartWeaponShow(0);
			m_pAnimationComponent.RefreshIkPose();
			m_CommandWeapons.SetWeaponChangeState(EWeaponChangeState.WEAPON_CHANGE_NONE);
			m_pWeaponManager.GetCurrentWeapon().SetSafetyFireMode(false);
			return true;
		}
		break;
		case EWeaponChangeState.WEAPON_CHANGE_EMPTY:
		{
			if( m_pWeaponManager.IsWeaponItemAttached() )
			{
				m_CommandWeapons.StartWeaponShow(0);
				m_CommandWeapons.SetWeaponChangeState(EWeaponChangeState.WEAPON_CHANGE_SHOW);
			}
		}
		break;
		default:
			break;
		}
		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool HandleThrowing(CharacterInputContext pInputCtx, float pDt, int pCurrentCommandID)
	{
		// canceling action
		if( pInputCtx.IsThrowCanceled() && (m_ThrowingState > EThrowingState.THROWING_NONE && m_ThrowingState < EThrowingState.THROWING_THROW) )
		{
			m_ThrowingState = EThrowingState.THROWING_CANCEL_ENTER;
		}

		switch( m_ThrowingState )
		{
		case EThrowingState.THROWING_NONE:
			if( pInputCtx.GetThrow() && m_WeaponManager.IsCurrentWeaponThrowable() )
			{
				m_ThrowingState = EThrowingState.THROWING_UNPIN_ACTION;
				m_CommandWeapons.StartPinGrenadeAction(0);
				// TODO: uncomment when exposed
				//m_WeaponManager.OnAnimationEvent("Weapon_Unpin", nullptr, -1);
				return true;
			}
			break;

		case EThrowingState.THROWING_UNPIN_ACTION:
			if( !m_CommandWeapons.IsGrenadeAttachmentAnimTag() )
			{
				m_fThrowingForce = pDt;
				m_ThrowingState = EThrowingState.THROWING_SETFORCE;
			}
			return true;

		case EThrowingState.THROWING_SETFORCE:
		{
			if( !pInputCtx.GetThrow() )
			{
				m_ThrowingState = EThrowingState.THROWING_THROW;
				m_CommandWeapons.StartThrowingAction();
				return true;
			}

			m_fThrowingForce += pDt;
		}
		return true;

		case EThrowingState.THROWING_THROW:
			if( m_CommandWeapons.WasItemLeaveHandsEvent() )
			{
				//! aim space
				float lrAngle = m_CommandWeapons.GetAimAngleLR();
				float udAngle = m_CommandWeapons.GetAimAngleUD();

				//!
				float ya = Math.DEG2RAD * lrAngle;
				float pa = Math.DEG2RAD * udAngle;
				float cp = Math.Cos(0);

				//enf::Vector3 dirMS{ cp * sinf(ya), sin(pa), cp * cosf(ya) };
				vector dirMS = Vector(cp * Math.Sin(ya), Math.Sin(pa), cp * Math.Cos(ya));
				//enf::Vector3 dirWS = m_pOwner->GetWorldTransform().TransformVector(dirMS.Normalized());
				vector dirWS = m_OwnerEntity.VectorToParent(dirMS.Normalized());

				m_WeaponManager.Throw(m_OwnerEntity, true, dirWS, m_fThrowingForce);
				m_ThrowingState = EThrowingState.THROWING_FINISH;
			}
			return true;

		case EThrowingState.THROWING_FINISH:
			if( m_CommandWeapons.IsThrowTag() )
				return true;
			m_fThrowingForce = 0.0;
			m_ThrowingState = EThrowingState.THROWING_NONE;
			break;

		case EThrowingState.THROWING_CANCEL_ENTER:
			if( !m_CommandWeapons.IsGrenadeAttachmentAnimTag() )
			{
				m_CommandWeapons.StartPinGrenadeAction(1);
				m_WeaponManager.OnAnimationEvent("Weapon_Repin", nullptr, -1);
				m_fThrowingForce = 0.0;
				m_ThrowingState = EThrowingState.THROWING_CANCEL_EXIT;
			}
			return true;

		case EThrowingState.THROWING_CANCEL_EXIT:
			if( !m_CommandWeapons.IsGrenadeAttachmentAnimTag() )
			{
				m_ThrowingState = EThrowingState.THROWING_NONE;
			}
			break;
		}

		return false;
	}
	*/
	
	//------------------------------------------------------------------------------------------------
	override bool TransitionMove_JumpClimb(CharacterInputContext pInputCtx)
	{
		if ( pInputCtx.GetJump() > 0.0 )
		{
			CancelThrowDefault();

			ref CharacterCommandClimbResult climbTestResult = new CharacterCommandClimbResult();
			CharacterCommandClimb.DoClimbTest(m_OwnerEntity, GetCommandClimbSettings(), climbTestResult);

			vector grabPoint = climbTestResult.m_ClimbGrabPointLS;
			if ( climbTestResult.m_ClimbParent )
			{
				vector grabParentTM[4]; 
				climbTestResult.m_ClimbParent.GetWorldTransform(grabParentTM);
				grabPoint = grabPoint.Multiply4(grabParentTM);
			}
			
			vector charTM[4];
			m_OwnerEntity.GetWorldTransform(charTM);			
			const float climbHeight = grabPoint[1] - charTM[3][1];
			
			int climbType = -1;
			if ( climbTestResult.m_bIsClimb || climbTestResult.m_bIsClimbOver )
			{
				if ( climbHeight < 1.1 )
					climbType = 0;
				else if( climbHeight >= 1.1 && climbHeight < 1.7 )
					climbType = 1;
				else if( climbHeight >= 1.7 && climbHeight < 2.5 )
					climbType = 2;

				if ( climbType != -1 )
				{
					StartCommand_Climb(climbTestResult, climbType);
				}
			}

			if ( climbType == -1 )
			{
				m_fFallYDiff = charTM[3][1];
				StartCommand_Fall(pInputCtx.GetJump());
			}
			return true;
		}

		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void OnInit(IEntity owner)
	{
		super.OnInit(owner);
		
		m_OwnerEntity = ChimeraCharacter.Cast(owner);
		m_CharacterAnimComp = CharacterAnimationComponent.Cast(m_OwnerEntity.FindComponent(CharacterAnimationComponent));
		m_CharacterControllerComp = CharacterControllerComponent.Cast(m_OwnerEntity.FindComponent(CharacterControllerComponent));
		
		m_MeleeComponent = SCR_MeleeComponent.Cast(m_OwnerEntity.FindComponent(SCR_MeleeComponent));
		//m_WeaponManager = BaseWeaponManagerComponent.Cast(m_OwnerEntity.FindComponent(BaseWeaponManagerComponent));
		
		//m_CommandWeapons = GetCommandModifier_Weapon();
		//m_ThrowingState = EThrowingState.THROWING_NONE;

		m_SwimST = new SCR_CharacterCommandSwimST(m_CharacterAnimComp);
		m_SwimCommand = new SCR_CharacterCommandSwim(m_CharacterAnimComp, m_SwimST, m_OwnerEntity, GetControllerComponent());
		m_FlyCommand = new SCR_CharacterCommandFly(m_CharacterAnimComp, m_SwimST, m_OwnerEntity, GetControllerComponent());
	}
	
	/*
	//------------------------------------------------------------------------------------------------
	override bool SubhandlerStatesBegin(CharacterInputContext pInputCtx, float pDt, int pCurrentCommandID)
	{
		if( pCurrentCommandID == ECommandIDs.SCRIPTED )
		{
			AnimPhysCommandScripted cmd = m_CharacterAnimComp.GetCommandScripted();
			
			if( SCR_CharacterCommandFly.Cast(cmd) )
				return true;
			
		}
		
		if ( pInputCtx.GetJump() > 0.0 )
		{
			m_CharacterAnimComp.SetCurrentCommand(m_FlyCommand);
			return true;
		}
		
		return false;
	}
	*/

	protected ref SCR_CharacterCommandSwimST m_SwimST;
	protected ref SCR_CharacterCommandSwim m_SwimCommand;
	protected ref SCR_CharacterCommandFly m_FlyCommand;

	protected float m_fFallYDiff;
	//protected float m_fThrowingForce;

	//protected CharacterCommandWeapon m_CommandWeapons;
	//protected EThrowingState m_ThrowingState;
}
