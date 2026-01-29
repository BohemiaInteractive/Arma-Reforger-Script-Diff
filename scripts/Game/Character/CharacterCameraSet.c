class CharacterCameraSet : ScriptedCameraSet
{
	static const int 	CHARACTERCAMERA_DEBUG 					= 0;		//!< just a debug
	static const int 	CHARACTERCAMERA_1ST 					= 1;		//!< 1st person camera
	static const int 	CHARACTERCAMERA_1ST_BONE_TRANSFORM		= 2;		//!< same as 1st, only uses full transform of head bone with given offset
	static const int	CHARACTERCAMERA_1ST_VEHICLE				= 3;		//!< vehicle 1st person
	static const int	CHARACTERCAMERA_1ST_VEHICLE_TRANSITION	= 4;		//!< vehicle 1st person
	static const int	CHARACTERCAMERA_1ST_TURRET				= 5;		//!< turret in 1st person
	static const int 	CHARACTERCAMERA_1ST_TURRET_TRANSITION	= 35;
	static const int	CHARACTERCAMERA_1ST_READY				= 6;		//!< weapon ready in 1st person
	static const int	CHARACTERCAMERA_1ST_FLOAT				= 7;		//!< 1st person floating cam
	
	static const int 	CHARACTERCAMERA_3RD_ERC 				= 10;		//!< 3rd - standing
	static const int 	CHARACTERCAMERA_3RD_ERC_SPR 			= 11;		//!< 3rd - standing sprint
	static const int 	CHARACTERCAMERA_3RD_CRO 				= 12;		//!< 3rd - crouch
	static const int 	CHARACTERCAMERA_3RD_PRO 				= 13;		//!< 3rd - laying
	static const int	CHARACTERCAMERA_3RD_JUMP				= 14;		//!< jump
	static const int	CHARACTERCAMERA_3RD_CLIMB				= 15;		//!< climb / vault
	static const int	CHARACTERCAMERA_3RD_VEHICLE				= 16;		//!< generic vehicle 3rd person
	static const int	CHARACTERCAMERA_3RD_TURRET				= 17; 		//!< turret camera
	static const int	CHARACTERCAMERA_3RD_SITTING				= 18; 		//!< sitting loitering camera
	static const int	CHARACTERCAMERA_3RD_FLOAT				= 19; 		//!< 3rd person floating cam
	static const int	CHARACTERCAMERA_3RD_UNCONSCIOUS			= 20; 		//!< 3rd person uncon cam
	
	static const int 	CHARACTERCAMERA_ADS						= 30;		//!< ironsights camera
	static const int 	CHARACTERCAMERA_OPTICS					= 31;		//!< optics
	static const int 	CHARACTERCAMERA_BINOCULARS				= 32;		//!< binoculars

	static const int	CHARACTERCAMERA_ADS_VEHICLE				= 33;		//!< turret camera
	
	static const int	CHARACTERCAMERA_1ST_UNCONSCIOUS			= 34;		//!< unconscious 1st person

	//! debug view Types
	static const int DEBUGVIEW_NONE = 0;
	static const int DEBUGVIEW_FACE = 1;
	static const int DEBUGVIEW_WEAPON = 2;
	static const int DEBUGVIEW_LEFT = 3;
	static const int DEBUGVIEW_RIGHT = 4;
	static const int DEBUGVIEW_FRONT = 5;
	static const int DEBUGVIEW_REAR = 6;
	static const int DEBUGVIEW_TOP = 7;
	static const int DEBUGVIEW_BOTTOM = 8;

	// some times for camera changes
	static const float 	TIME_CAMERACHANGE_01 				= 0.1;
	static const float 	TIME_CAMERACHANGE_02 				= 0.2;
	static const float 	TIME_CAMERACHANGE_03 				= 0.3;

	//------------------------------------------------------------------------------------------------
	//! called when camera set is created - used for adding camera items and for transition between cameras settings
	override void Init()
	{
		//
		// this registers camera ids for camera script class implementation
		// which camera is used at the time is specified by CameraHandlerComponent.CameraSelector which returns id
		//
		RegisterCameraCreator(CHARACTERCAMERA_DEBUG, CharacterCameraDebug);

		//
		// 1PV
		//
		RegisterCameraCreator(CHARACTERCAMERA_1ST, CharacterCamera1stPerson);
		RegisterCameraCreator(CHARACTERCAMERA_1ST_BONE_TRANSFORM, CharacterCamera1stPersonBoneTransform);
		RegisterCameraCreator(CHARACTERCAMERA_1ST_VEHICLE, CharacterCamera1stPersonVehicle);
		RegisterCameraCreator(CHARACTERCAMERA_1ST_VEHICLE_TRANSITION, CharacterCamera1stPersonVehicleTransition);
		RegisterCameraCreator(CHARACTERCAMERA_1ST_TURRET, CharacterCamera1stPersonTurret);
		RegisterCameraCreator(CHARACTERCAMERA_1ST_TURRET_TRANSITION, CharacterCamera1stPersonTurretTransition);
		RegisterCameraCreator(CHARACTERCAMERA_1ST_READY, CharacterCamera1stPersonReady);
		RegisterCameraCreator(CHARACTERCAMERA_1ST_FLOAT, CharacterCamera1stPersonFloat);
		RegisterCameraCreator(CHARACTERCAMERA_1ST_UNCONSCIOUS, CharacterCamera1stPersonUnconscious);

		//
		// 3PV
		//
		RegisterCameraCreator(CHARACTERCAMERA_3RD_ERC, CharacterCamera3rdPersonErc);
		//RegisterCameraCreator(CHARACTERCAMERA_3RD_ERC_SPR, CharacterCamera3rdPersonErcSpr);
		RegisterCameraCreator(CHARACTERCAMERA_3RD_CRO, CharacterCamera3rdPersonCrouch);
		RegisterCameraCreator(CHARACTERCAMERA_3RD_PRO, CharacterCamera3rdPersonProne);
		//RegisterCameraCreator(CHARACTERCAMERA_3RD_JUMP, CharacterCamera3rdPersonJump);
		RegisterCameraCreator(CHARACTERCAMERA_3RD_CLIMB, CharacterCamera3rdPersonClimb);
		RegisterCameraCreator(CHARACTERCAMERA_3RD_VEHICLE, CharacterCamera3rdPersonVehicle);
		RegisterCameraCreator(CHARACTERCAMERA_3RD_TURRET, CharacterCamera3rdPersonTurret);
		RegisterCameraCreator(CHARACTERCAMERA_3RD_SITTING, CharacterCamera3rdPersonSitting);
		RegisterCameraCreator(CHARACTERCAMERA_3RD_FLOAT, CharacterCamera3rdPersonFloat);
		RegisterCameraCreator(CHARACTERCAMERA_3RD_UNCONSCIOUS, CharacterCamera3rdPersonUnconscious);

		//
		// ADS + FOCUS
		//
		RegisterCameraCreator(CHARACTERCAMERA_ADS, CharacterCameraADS);
		//RegisterCameraCreator(CHARACTERCAMERA_OPTICS, CharacterCameraOptics);
		RegisterCameraCreator(CHARACTERCAMERA_BINOCULARS, CharacterCameraBinoculars);
		RegisterCameraCreator(CHARACTERCAMERA_ADS_VEHICLE, CharacterCameraADSVehicle);		
		
		//! Blend times when changing cameras
		//! 3rd person erected camera transition
		RegisterTransitionTime(CHARACTERCAMERA_3RD_ERC, CHARACTERCAMERA_3RD_CRO, 0.4, true);
		RegisterTransitionTime(CHARACTERCAMERA_3RD_PRO, CHARACTERCAMERA_3RD_CRO, 0.6, true);
		RegisterTransitionTime(CHARACTERCAMERA_1ST_VEHICLE, CHARACTERCAMERA_ADS_VEHICLE, 0.0, true);
		RegisterTransitionTime(CHARACTERCAMERA_3RD_VEHICLE, CHARACTERCAMERA_ADS_VEHICLE, 0.0, true);
	}
}
