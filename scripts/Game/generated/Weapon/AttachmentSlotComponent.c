/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Weapon
\{
*/

class AttachmentSlotComponent: BaseAttachmentSlotComponent
{
	proto external void SetAttachment(IEntity attachmentEntity);
	proto external IEntity GetAttachedEntity();
	proto external BaseAttachmentType GetAttachmentSlotType();
	//! Is entity compatible to be attached to this slot?
	proto external bool CanSetAttachment(IEntity attachmentEntity);
	/*!
	Does this slot obstruct others? This is for example relevant if an optic slot obstructs the weapons
	iron sights (either built-in or attached). Example: PSO-1 scope on AK pattern rifles, or the P90 tri-rail
	are not obstructed by mounting an optic, while most M4-type rifles are.
	*/
	proto external bool IsObstructing();
	//! Is this slot should show during inspection
	proto external bool ShouldShowInInspection();

	// callbacks

	event bool ShouldSetAttachment(IEntity attachmentEntity) { return true; };
	event bool ShouldSetResource(Resource resource) { return true; };
}

/*!
\}
*/
