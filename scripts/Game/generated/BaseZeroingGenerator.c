/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
BaseZeroingGenerator is an object used for generating
zeroing information in Workbench
*/
class BaseZeroingGenerator: ScriptAndConfig
{
	/*!
	Gets the owning entity of the generator object, which might or might not be the same as
	the owner of the Sights component.
	*/
	proto external IEntity GetOwnerEntity();
	/*!
	Get the sights component that the zeroing is computed for
	*/
	proto external BaseSightsComponent GetSights();
	/*!
	Get the Weapon animation component of the sights. This will might be nullptr if the sights do not have
	a WeaponAnimationComponent, like e.g. with the UGL's.
	*/
	proto external WeaponAnimationComponent GetWeaponAnimationComponent();
	/*!
	Flag the generation of PointInfo's. Normal operation is to generate a PointInfo with every SightRangeInfo
	generated, but some zeroing mechanisms like the PiP scopes don't need it.

	By default, PointInfo is generated, so this really only needs to be called in case they are unwanted.

	\param doGenerate boolean flag. If true, PointInfo's are generated. If false, they are not.
	*/
	proto external void SetPointInfoGeneration(bool doGenerate);
	proto external void SetProjectileResource(ResourceName resourceName);
}
