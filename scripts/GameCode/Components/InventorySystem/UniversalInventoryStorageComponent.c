class UniversalInventoryStorageComponentClass: BaseUniversalInventoryStorageComponentClass
{
};

// Current storage variant allows dynamic scaling of slots and handles Move/Insert/Remove operations
// it will accept any entity for insertion and will remove/add it's visibility flag when inserted/removed from storage
// see CharacterInventoryStorageComponent for example of custom storage inheritance from current class
class UniversalInventoryStorageComponent : BaseUniversalInventoryStorageComponent
{
		
};