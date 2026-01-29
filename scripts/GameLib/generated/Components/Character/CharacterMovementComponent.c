/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Components_Character
\{
*/

class CharacterMovementComponent: PawnMovementComponent
{
	proto external float GetHeadingAngle();
	proto external bool HasRootMotion();
	proto external bool SetRootMotion(bool rootMotion);
	proto external vector GetVelocityWS();
	proto external vector GetVelocityMS();
	proto external vector GetAngularVelocity();
	proto external void SetMovementMode(ECharacterMovementMode mode);
	proto external ECharacterMovementMode GetMovementMode();
	proto external void SetRotationMode(ECharacterRotationMode mode);
	proto external ECharacterRotationMode GetRotationMode();
	proto external void SetMovementMaxSpeed(float movementSpeed);
	proto external float GetMovementMaxSpeed();
	proto external void SetFlyingMaxSpeed(float flyingSpeed);
	proto external float GetFlyingMaxSpeed();
	proto external void SetSwimmingMaxSpeed(float swimmingSpeed);
	proto external float GetSwimmingMaxSpeed();
	proto external float GetMoveMinAnalogSpeed();
	proto external IEntity GetFloorEntity();
	proto external vector GetFloorNormal();
	proto external void GetFloorNormalAngles(out float frontalAngle, out float lateralAngle);
	proto external float GetMovementMaxSlopeAngle();
	proto external vector PredictStopPosition();
	proto external int GetCurrentMoveTimeStamp();
	proto external void SetJumpInput(bool state);
	proto external bool IsJumpInput();
	proto external bool IsMovingOnGround();
	proto external bool IsFalling();
	proto external bool CanChangeCollisionShape(string shapeName);
	proto external void ChangeCollisionShape(string shapeName);
	proto external void EnableSlidingAlongObstacles(bool state);
	proto external bool IsFlyModeColliding();
	proto external void SetFlyModeColliding(bool state);
}

/*!
\}
*/
