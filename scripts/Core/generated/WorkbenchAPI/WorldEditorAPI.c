/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup WorkbenchAPI
\{
*/

sealed class WorldEditorAPI
{
	private void WorldEditorAPI();
	private void ~WorldEditorAPI();

	//! Beginning of logical edit action.
	proto external bool BeginEntityAction(string historyPointName = string.Empty, string historyPointIcon = string.Empty, void userData = NULL);
	//! Ending of edit action.
	proto external bool EndEntityAction(string historyPointName = string.Empty);
	proto external bool IsDoingEditAction();
	//! Together with EndEditSequence() it can be used to define a block where entity will never be re-initialized. Useful for multiple edit actions upon single entity.
	proto external bool BeginEditSequence(IEntitySource entSrc);
	//! Together with BeginEditSequence() it can be used to define a block where entity will never be re-initialized. Useful for multiple edit actions upon single entity.
	proto external bool EndEditSequence(IEntitySource entSrc);
	//! Check whether we are between BeginEditSequence() and EndEditSequence().
	proto external bool IsEditSequenceActive(IEntitySource entSrc);
	//! Check whether editor is restoring undo or redo state.
	proto external bool UndoOrRedoIsRestoring();
	//! Check whether user created an entity in editor window. Eg. drag and drop from hierarchy, etc.
	proto external bool IsCreatingEntityInWindow();
	proto external bool IsModifyingData();
	//! Check whether editor is running in a prefab edit mode where all edit actions should apply their changes directly to a prefab instead of entity instance
	proto external bool IsPrefabEditMode();
	proto external IEntity SourceToEntity(IEntitySource entSrc);
	proto external IEntitySource EntityToSource(IEntity ent);
	proto external IEntitySource FindEntityByName(string name);
	proto external IEntitySource FindEntityByID(EntityID id);
	//! Returns the number of entities in the editor.
	proto external int GetEditorEntityCount();
	//! Returns an entity in the editor on given index. To obtain count, use GetEditorEntityCount().
	proto external IEntitySource GetEditorEntity(int index);
	proto external int GetEntityCount(int subScene);
	proto external IEntitySource GetEntity(int subScene, int index);
	//! Checks whether there previously were any entities copied
	proto external bool HasCopiedEntities();
	//! Copies selected entities. Returns true if any entity was succesfully copied
	proto external bool CopySelectedEntities();
	//! Pastes previously copied entities at the same position. Returns true if any entity was created
	proto external bool PasteEntities();
	//! Pastes previously copied entities at mouse cursor position. Returns true if any entity was created
	proto external bool PasteEntitiesAtMouseCursorPos();
	//! Duplicates selected entities and places them at the same position. Returns true if any entity was duplicated
	proto external bool DuplicateSelectedEntities();
	//! Copies selected entities and deletes them from map. Returns true if any entity was cutted out
	proto external bool CutSelectedEntities();
	//	virtual bool ModifyComponentTemplateKey(enf::EntitySource* entityTmplOwner, enf::EntityComponentSource* tmpl, enf::uint keyIndex, enf::CStr value)=0;
	proto external bool ParentEntity(notnull IEntitySource parent, notnull IEntitySource child, bool transformChildToParentSpace);
	proto external bool RemoveEntityFromParent(notnull IEntitySource child);
	proto external IEntityComponentSource CreateComponent(IEntitySource owner, string className);
	proto external bool DeleteComponent(IEntitySource owner, IEntityComponentSource component);
	proto external int GetSelectedEntitiesCount();
	proto external IEntitySource GetSelectedEntity(int n = 0);
	proto external void ClearEntitySelection();
	proto external void RemoveFromEntitySelection(notnull IEntitySource ent);
	proto external void SetPropertySelection(string id);
	proto external void UpdateSelectionGui();
	proto external bool CreateEntityTemplate(IEntitySource entitySrc, string templateFileAbs);
	proto external bool SaveEntityTemplate(IEntitySource tmpl);
	//create clone of entity
	proto external IEntitySource GetEntityUnderCursor();
	proto external bool RenameEntity(notnull IEntitySource ent, string newName);
	proto external int GetCurrentSubScene();
	proto external int GetNumSubScenes();
	//! Returns width of active scene window (usually it's perspective window but it can be also one of Top, Back, Right view).
	proto external int GetScreenWidth();
	//! Returns height of active scene window (usually it's perspective window but it can be also one of Top, Back, Right view).
	proto external int GetScreenHeight();
	/*!
	Returns horizontal mouse cursor position in active scene window (usually it's perspective window but it can be also one of Top, Back, Right view).
	\param clamped Whether returned values should be clamped in a range between zero and screen width (window width in fact).
	*/
	proto external int GetMousePosX(bool clamped);
	/*!
	Returns vertical mouse cursor position in active scene window (usually it's perspective window but it can be also one of Top, Back, Right view).
	\param clamped Whether returned values should be clamped in a range between zero and screen height (window height in fact).
	*/
	proto external int GetMousePosY(bool clamped);
	//! Enable/disable calling of events on generators. Must be enabled for the VectorTool and generators to work properly.
	proto external void ToggleGeneratorEvents(bool isEnabled);
	proto external bool AreGeneratorEventsEnabled();
	/*!
	Modifies the terrain height at specified position based on given brush.
	\param xWorldPos X coordinate of the modification position.
	\param zWorldPos Z coordinate of the modification position.
	\param operation Operation to perform on the terrain.
	\param toolDesc Parameters of the operation.
	\param shape Shape of the brush.
	\param interpFunc Interpolation function used when creating the brush.
	*/
	proto external void ModifyHeightMap(float xWorldPos, float zWorldPos, FilterMorphOperation operation, notnull TerrainToolDesc toolDesc, FilterMorphShape shape, FilterMorphLerpFunc interpFunc);
	/*!
	Modifies the terrain height at specified position based on given map.
	\param xWorldPos X coordinate of the modification position.
	\param zWorldPos Z coordinate of the modification position.
	\param operation Operation to perform on the terrain.
	\param toolDesc Parameters of the operation.
	\param userShape NxN array describing the modification brush. Values from 0.0 to 1.0 describing no to maximum strength.
	\param userShapeFilter Filtering to use when the tool size is not the same as the userShape array dimension.
	*/
	proto external void ModifyHeightMapUserShape(float xWorldPos, float zWorldPos, FilterMorphOperation operation, notnull TerrainToolDesc toolDesc, array<float> userShape, UserShapeFilter userShapeFilter);
	/*!
	Modifies the terrain layers at specified position based on given brush.
	\param xWorldPos X coordinate of the modification position.
	\param zWorldPos Z coordinate of the modification position.
	\param operation Operation to perform on the terrain.
	\param toolDesc Parameters of the operation.
	\param shape Shape of the brush.
	\param interpFunc Interpolation function used when creating the brush.
	*/
	proto external void ModifyLayers(float xWorldPos, float zWorldPos, FilterMorphOperation operation, notnull TerrainToolDesc toolDesc, FilterMorphShape shape, FilterMorphLerpFunc interpFunc);
	/*!
	Modifies the terrain height at specified position based on given map.
	\param xWorldPos X coordinate of the modification position.
	\param zWorldPos Z coordinate of the modification position.
	\param operation Operation to perform on the terrain.
	\param toolDesc Parameters of the operation.
	\param userShape NxN array describing the modification brush. Values from 0.0 to 1.0 describing no to maximum strength.
	\param userShapeFilter Filtering to use when the tool size is not the same as the userShape array dimension.
	*/
	proto external void ModifyLayersUserShape(float xWorldPos, float zWorldPos, FilterMorphOperation operation, notnull TerrainToolDesc toolDesc, array<float> userShape, UserShapeFilter userShapeFilter);
	proto external bool BeginTerrainAction(TerrainToolType toolType, string historyPointName = string.Empty, string historyPointIcon = string.Empty);
	proto external void EndTerrainAction(string historyPointName = string.Empty);
	//! Returns terrain height (world space) in given point x,z (world space). Very fast method.
	proto external float GetTerrainSurfaceY(float x, float z);
	/*!
	Returns terrain height (world space) for given tile. Data are in final form - with applied roads and other modifiers. Very fast method.
	Output array will be resized by this method and size will be:
	(GetTerrainSizeX(terrainIndex) / GetTerrainTilesX(terrainIndex)) * (GetTerrainSizeY(terrainIndex) / GetTerrainTilesY(terrainIndex))

	\code
	WorldEditor we = Workbench.GetModule(WorldEditor);
	auto api = we.GetApi();
	array<float> heightMap = {};
	api.BeginTerrainAction(TerrainToolType.HEIGHT_EXACT);
	if (api.GetTerrainSurfaceTile(0, 1, 8, heightMap))
	{
		for (int i = 0; i < heightMap.Count(); i++)
			heightMap[i] = heightMap[i] + Math.RandomFloat(-0.3, 0.3);
		api.SetTerrainSurfaceTile(0, 1, 8, heightMap);
	}
	api.EndTerrainAction();
	/endcode
	*/
	proto external bool GetTerrainSurfaceTile(int terrainIndex, int tileX, int tileY, out notnull array<float> surfaceY);
	/*!
	Write terrain height (world space) to given tile. Very fast method.
	Use in combination with BeginTerrainAction(TerrainToolType.HEIGHT_EXACT) /EndTerrainAction
	Input array must have exact size:
	(GetTerrainSizeX(terrainIndex) / GetTerrainTilesX(terrainIndex)) * (GetTerrainSizeY(terrainIndex) / GetTerrainTilesY(terrainIndex))

	\code
	WorldEditor we = Workbench.GetModule(WorldEditor);
	auto api = we.GetApi();
	array<float> heightMap = {};
	api.BeginTerrainAction(TerrainToolType.HEIGHT_EXACT);
	if (api.GetTerrainSurfaceTile(0, 1, 8, heightMap))
	{
		for (int i = 0; i < heightMap.Count(); i++)
			heightMap[i] = heightMap[i] + Math.RandomFloat(-0.3, 0.3);
		api.SetTerrainSurfaceTile(0, 1, 8, heightMap);
	}
	api.EndTerrainAction();
	/endcode
	*/
	proto external bool SetTerrainSurfaceTile(int terrainIndex, int tileX, int tileY, notnull array<float> surfaceY);
	//! Returns terrain X resolution of height map.
	proto external int GetTerrainResolutionX(int terrainIndex = 0);
	//! Returns terrain Y resolution of height map.
	proto external int GetTerrainResolutionY(int terrainIndex = 0);
	//! Returns terrain number of tiles.
	proto external int GetTerrainTilesX(int terrainIndex = 0);
	//! Returns terrain number of tiles.
	proto external int GetTerrainTilesY(int terrainIndex = 0);
	//! Returns terrain Planar resolution (meters).
	proto external float GetTerrainUnitScale(int terrainIndex = 0);
	//! Adds entity to list of terrain flatters and updates heightmap
	proto external void RemoveTerrainFlatterEntity(IEntity entity, bool bUpdateTerrain = true);
	//--------------------------------- Rivers ----------------------------------
	proto external void RegenerateFlowMaps(bool preview = true, bool save = true, bool asyncReturn = true, bool noWarning = false);
	//--------------------------------- Shore -----------------------------------
	proto external void BuildShoreMap();
	//! Returns true, if entity is hidden by functions HIDE/UNHIDE or is hidden because of hidden layer etc.
	proto external bool IsEntityVisible(IEntitySource entity);
	proto external void SetEntityVisible(IEntitySource entity, bool isVisible, bool recursive);
	//! Whether entity is selected (any member of multi-selection)
	proto external bool IsEntitySelected(IEntitySource entity);
	//! Whether entity is selected and it's a main member of multi-selection
	proto external bool IsEntitySelectedAsMain(IEntitySource entity);
	proto external bool IsEditMode();
	proto external bool IsGameMode();
	//! Set world editor perspective view camera position and look direction.
	proto external void SetCamera(vector pos, vector lookVec);
	proto external void AddTerrainFlatterEntity(IEntity entity, vector mins, vector maxs, int iPriority, float fFalloffStart, float fFalloff, bool bForceUpdate = true, array<vector> updateMins = null, array<vector> updateMaxes = null);
	//! Fills `y` with GetTerrainSurfaceY() and returns true if on x, z position is terrain, returns false otherwise.
	proto external bool TryGetTerrainSurfaceY(float x, float z, out float y);
	proto external void AddToEntitySelection(notnull IEntitySource ent);
	proto external void SetEntitySelection(notnull IEntitySource ent);
	[Obsolete("Use SetVariableValue instead!")]
	proto external bool ModifyEntityKey(notnull IEntitySource ent, string key, string value);
	[Obsolete("Use SetVariableValue instead!")]
	proto external bool ModifyComponentKey(notnull IEntitySource ent, string componentName, string key, string value);
	proto external IEntitySource CreateEntity(string className, string name, int layerId, IEntitySource parent, vector coords, vector angles);
	//! Extended version of base method that will use randomization/placement parameters set in Template Library (if any).
	proto external IEntitySource CreateEntityExt(string className, string name, int layerId,  IEntitySource parent, vector coords, vector angles, int traceFlags);
	proto external IEntitySource CreateClonedEntity(notnull IEntitySource ent, string name, IEntitySource parent, bool cloneChildren);
	proto external IEntitySource CreateEntityInWindow(RenderTargetWidget window, int winX, int winY, string className, string name, int layerID);
	proto external IEntitySource CreateEntityInWindowEx(int windowType, int winX, int winY, string className, string name, int layerID);
	//! Parent childs to parent and resolve previous parent->child dependencies
	proto external bool ParentEntities(notnull IEntitySource parent, notnull array<IEntitySource> childs, bool transformChildToParentSpace);
	proto external bool DeleteEntity(notnull IEntitySource ent);
	proto external bool DeleteEntities(notnull array<IEntitySource> ents);
	//! Trace inside WorldEditor window, x and y coords are window coordinates.
	proto external bool TraceWorldPos(int x, int y, TraceFlags traceFlags, out vector traceStart, out vector traceEnd, out vector traceDir, out IEntity hitEntity = null);
	proto external void GetWorldPath(out string path);
	proto external string GetEntityNiceName(IEntitySource entSrc);
	//! Generates unique name for a particular entity which is based on class/prefab name and numerical postfix
	proto external string GenerateDefaultEntityName(IEntitySource entSrc);
	/*!
	Set value of potentially nested variable. The whole path must exist, this function is just to set
	the value. For creation, see CreateObjectArrayVariableMember().

	This function traverses containers from `topLevel` using `containerIdPath`. For each entry selects property with given index.
	If the property is an array index from entry is used to select object at given index. For array properties index must not be -1.

	Let's have following config:
	\code
		Shape { // <- top level container
			Points { // <- array of points
				ShapePoint { // <- first element
					Position 2 3 1
					Metadata {
						Name name1
					}
				}
				ShapePoint { // <- second element
					Position 0 1 1
					Metadata {
						Name name2
					}
				}
			}
		}
	\endcode

	To set a name of the second shape point to "secondPoint" we do:
	\code
		auto containerPath = new array<ref ContainerIdPathEntry>();

		auto entry1 = new ContainerIdPathEntry("Points", 0); // Take the first point
		containerPath.Insert(entry1);

		auto entry2 = new ContainerIdPathEntry("Metadata"); // Go to metadata of the first point
		containerPath.Insert(entry2);

		m_API.SetVariableValue(topLevelShapeContainer, containerPath, "Name", "firstPoint"); // Set Name to given value
	\endcode

	\param topLevel Container from which the search is started.
	\param containerIdPath Path to the variable we want to set.
	\param key The actual variable to set.
	\param value Value which will be set to variable.
	\return `true` if successful, `false` otherwise.
	*/
	proto external bool SetVariableValue(notnull BaseContainer topLevel, array<ref ContainerIdPathEntry> containerIdPath, string key, string value);
	/*!
	Clear value of potentially nested variable. The whole path must exist, this function is just to clear the value.
	\param topLevel Container from which the search is started.
	\param containerIdPath Path to the variable we want to clear.
	\param key The actual variable to clear.
	\return `true` if successful, `false` otherwise.
	*/
	proto external bool ClearVariableValue(notnull BaseContainer topLevel, array<ref ContainerIdPathEntry> containerIdPath, string key);
	/*!
	Creates a new container in object property.
	\param topLevel Container from which the search is started
	\param containerIdPath Path to the variable we want to set
	\param key How is the object property in `container` called
	\param baseClassName What type of object should be created to the property
	\return `true` if successful, `false` otherwise.
	*/
	proto external bool CreateObjectVariableMember(notnull BaseContainer topLevel, array<ref ContainerIdPathEntry> containerIdPath, string key, string baseClassName);
	/*!
	Creates a new element in array of objects property in `container`.
	\param topLevel Container from which the search is started
	\param containerIdPath Path to the variable we want to set
	\param key How is the array of object property in `container` called
	\param baseClassName What type of object should be created to the array
	\param memberIndex At which index should be the new element inserted
	\return `true` if successful, `false` otherwise.
	*/
	proto external bool CreateObjectArrayVariableMember(notnull BaseContainer topLevel, array<ref ContainerIdPathEntry> containerIdPath, string key, string baseClassName, int memberIndex);
	/*!
	Removes an element from array of objects of property `key` in `container`.
	\param topLevel Container from which the search is started
	\param containerIdPath Path to the variable we want to set
	\param key How is the array of object property in `container` called
	\param memberIndex Which element to remove
	\return `true` if successful, `false` otherwise.
	*/
	proto external bool RemoveObjectArrayVariableMember(notnull BaseContainer topLevel, array<ref ContainerIdPathEntry> containerIdPath, string key, int memberIndex);
	proto external bool ChangeObjectClass(notnull BaseContainer topLevel, array<ref ContainerIdPathEntry> containerIdPath, string baseClassName);
	/*!
	Shows a selection dialog with provided items where user can select one or multiple items.
	\param title Title of the dialog window.
	\param message Message shown in the dialog window.
	\param width Width of the dialog window.
	\param height Height of the dialog window.
	\param items List of items that will be shown in the dialog MenuBase.
	\param selectedItems List of indices that will be filled up after selection in the dialog corresponding to selected items.
	\param currentItem Item selected by default.
	*/
	proto external int ShowItemListDialog(string title, string message, int width, int height, notnull array<string> items, notnull out array<int> selectedItems, int currentItem);
	//! Puts new (nowhere used) entity source instances as children of an entity in prefab
	proto external bool AddPrefabMembers(IEntitySource prefabMember, notnull array<IEntitySource> members);
	proto external bool RemovePrefabMembers(notnull array<IEntitySource> members);
	/*!
	Moves existing entity instances from map into a prefab.
	\param newParentInMap It's any entity of the prefab instance that exists in map (can be any child or sub-child). This it's only an optional parameter which ensures that transformations of the entitiesInMap after move to the prefab will be relative to the entity that exists in map. If this parameter is null then world transformations of the entitiesInMap will be stored into a prefab
	\param newParentInPrefab It's a further parent entity in the prefab. It can be root or any sub-child entity stored in a prefab.
	\param entitiesInMap Any entity instances that currently exist in a map. These entities will be deleted from map and added into a prefab as children of newParentInPrefab.
	\param convertTransformations If true, it converts world entity transformations to local ones in prefab (modifies coords and angles props). If false, it just keeps values of coords and angles properties and puts them into a prefab
	*/
	proto external bool MoveEntitiesToPrefab(IEntitySource newParentInMap, IEntitySource newParentInPrefab, notnull array<IEntitySource> entitiesInMap, bool convertTransformations = true);
	proto external string GetCurrentToolName();
	proto external BaseWorld GetWorld();
	proto external string CreateSubsceneLayer(int subScene, string name, string parentPath = string.Empty);
	proto external string CreateSubsceneFolder(int subScene, string name, string parentPath = string.Empty);
	proto external bool RenameSubsceneFolder(int subScene, string itemPath, string newItemName);
	proto external bool DeleteSubsceneFolder(int subScene, string itemPath);
	proto external bool SetSubsceneFolderParent(int subScene, string itemPath, string parentPath);
	proto external void GetSubsceneFolders(int subscene, out notnull array<string> outItemPaths);
	proto external bool SetEntitySubsceneLayer(int subScene, IEntitySource pEntitySource, string itemPath);
	proto external string GetEntitySubsceneLayer(int subScene, IEntitySource pEntitySource);
	proto external string GetActiveSubsceneLayer(int subScene);
	proto external void SetActiveSubsceneLayer(int subScene, string layerPath);
	proto external int GetSubsceneLayerId(int subscene, string layerPath);
	proto external string GetSubsceneLayerPath(int subscene, int layerId);
	proto external bool IsSubsceneLayerVisible(int subscene, string layerPath);
	proto external bool IsEntityLayerVisible(int subscene, int layerId);
	//! Checks whether a layer is explicitly locked. If can return false also when any parent layer is locked! Consider whether you don't need to use IsEntityLayerLockedHierarchy instead
	proto external bool IsEntityLayerLocked(int subscene, int layerId);
	//! Returns true if a layer itself or any its parent layer is locked.
	proto external bool IsEntityLayerLockedHierarchy(int subscene, int layerId);
	proto external void LockEntityLayer(int subscene, int layerId);
	proto external void UnlockEntityLayer(int subscene, int layerId);
	proto external int GetCurrentEntityLayerId();
}

/*!
\}
*/
