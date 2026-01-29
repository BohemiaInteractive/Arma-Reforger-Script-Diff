class EquipmentStorageComponentClass: BaseEquipmentStorageComponentClass
{
};

//! Current storage variant uses equipment storage slots to attach equipment to different item variants (eg Flashlight attached to Jacket)
//! supports automatic repositioning at runtime based on parent's active mesh variant (rigged and static)
//! dynamic scaling of slots is not supported, which means that available storage slots should be configured ahead on target prefab
class EquipmentStorageComponent : BaseEquipmentStorageComponent
{
};
