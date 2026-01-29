// #define DEBUG_PRINT
// #define KC_ALPHA_CONTROLS
[EntityEditorProps(category: "GameScripted/Utility", description: "Entity used for spawning grids filled with models", color: "0 0 255 255")]
class SCR_GridSpawnerEntityClass: GenericEntityClass
{
};

enum GridSpawnerObjectType
{
	GSOT_NONE = 0,
	GSOT_TEMPLATE = 1,
	GSOT_MODEL = 2
};

//------------------------------------------------------------------------------------------------
//! Each object is represented by a key on the alphanumeric keyboard (0-9 above QWERTY)
//! Pressing that particular key will spawn a grid filled with the template / model.
//! BACKSPACE can be used to delete all entities at any given moment.
class SCR_GridSpawnerEntity: GenericEntity
{

	//! Total amount of objects to spawn - 100 = 10x10 grid
	[Attribute("100", UIWidgets.Slider, "The amount of objects to spawn", "0 100000 1")]
	private int m_iObjectsCount;
	
	//! If absolute spacing is enabled, objects will be placed exactly n metres apart, without taking BBOX into account
	[Attribute("0", UIWidgets.CheckBox, "If enabled, bounding boxes will not be taken into account, spacing will be absolute", "")]
	private bool m_bUseAbsoluteSpacing;
	
	//! The space between each object in metres.
	[Attribute("0.1", UIWidgets.Slider, "Spacing between each entity (counting with BBOX)", "-5 50 0.05")]
	private float m_fSpacing;
	
	//! If 0, no rotation is used. Otherwise this specifies a seed that rotates model in yaw <0, 360> degrees.
	[Attribute("0", UIWidgets.Slider, "Seed used for randomized rotations. Leave 0 for no randomization.", "0 1337 1")]
	private int m_iRandomizationSeed;
	
	//! If a valid path to a text file containing assets is specified, the list overrides objects specified in the editor.
	//! If left empty (string.Empty or ""), objects from the array in the editor will be used.
	[Attribute("", UIWidgets.EditBox, "List of entities in text file. Overrides the default array if not empty.", "")]
	private string m_sObjectsListPath;
	
	//! List of assets to be used, has to be a model (xob) or prefab (et).
	[Attribute("", UIWidgets.ResourceNamePicker, "Assets to be spawned. Models (xob) or prefabs (et).", "xob et")]
	private ref array<ResourceName> m_aObjects;
		
	//! List of currently spawned entities.
	private ref array<IEntity> m_aSpawnedEntities = null;
	
	//------------------------------------------------------------------------------------------------
	sealed array<IEntity> GetSpawnedEntities()
	{
		return m_aSpawnedEntities;
	}
	
	//------------------------------------------------------------------------------------------------
	protected int GetObjectCount()
	{
		if (m_aObjects)
			return m_aObjects.Count();
		else
			return 0;
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool GetObjectName(int index, out string name)
	{
		if (m_aObjects)
		{
			if (index >= 0 && index < GetObjectCount())
			{
				name = m_aObjects[index];
				return true;
			}
		}
		
		return false;		
	}
	
	//------------------------------------------------------------------------------------------------	
	private int GetObjectType(string object)
	{
		if (object == string.Empty)
			return GridSpawnerObjectType.GSOT_NONE;
		
		int lastDotIndex = object.LastIndexOf(".");
		int count = object.Length();
		string ext = object.Substring(lastDotIndex, count-lastDotIndex);
		
		switch (ext)
		{
			case ".et":
				return GridSpawnerObjectType.GSOT_TEMPLATE;
			
			case ".xob":
				return GridSpawnerObjectType.GSOT_MODEL;
			
			default:
				return GridSpawnerObjectType.GSOT_NONE;
		}
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param index The index of element to spawn
	//! \param centerPoint The center point in world coordinates of the spawned grid
	bool Spawn(int index)
	{
		vector v;
		return Spawn(index, v);
	}
	
	
	//------------------------------------------------------------------------------------------------
	//! \param index The index of element to spawn
	//! \param centerPoint The center point in world coordinates of the spawned grid
	protected bool Spawn(int index, out vector centerPoint)
	{
		// Not in game, bailing out.
		BaseWorld baseWorld = GetGame().GetWorld();
		if (!baseWorld)
			return false;
		
		// No array of objects, bail out.
		if (!m_aObjects)
		{
			#ifdef DEBUG_PRINT
			Print("GridSpawner: Error, no objects specified!", LogLevel.ERROR);
			#endif
			return false;
		}
		
		// Index out of range, bail out.
		int objectsCount = m_aObjects.Count();
		if (index < 0 || index >= objectsCount)
		{
			#ifdef DEBUG_PRINT
			Print("GridSpawner: Error, index " + index + " is out of bounds!", LogLevel.ERROR);
			#endif
			return false;
		}
		
		
		ResourceName object = m_aObjects[index];
		
		// Empty string, bailing out.
		if (object == string.Empty)
			return false;
		
		// Check if object is .et or .xob, else bail out.
		int type = GetObjectType(object);
		if (type == GridSpawnerObjectType.GSOT_NONE)
		{
			#ifdef DEBUG_PRINT
			Print("GridSpawnerEntity: Trying to spawn an invalid object. You must specify a valid template (.et) or mesh (.xob).", LogLevel.ERROR);
			#endif
			return false;
		}
		
		// Delete existing entities if they exist and create new array
		if (m_aSpawnedEntities)
			DeleteEntities();
		
		if (!m_aSpawnedEntities)
			m_aSpawnedEntities = new array<IEntity>();
		
		// Get origin, cell size and resource
		vector currentPosition = GetOrigin();
		int countPerAxis = Math.Floor(Math.Sqrt(m_iObjectsCount));
		Resource resource = Resource.Load(object);
		IEntity spawnedObject = null;
		
		// Define spacing - if not absolute, get data from bounding box, add spacing to that
		float spacing = m_fSpacing;
		
		// If relative (i.e. based on model bounds) spacing is desired,
		// spawn a dummy object and read data from it before deleting it
		if (!m_bUseAbsoluteSpacing)
		{
			// Get identity matrix
			vector dummyMatrix[4];
			Math3D.MatrixIdentity4(dummyMatrix);
			
			// ......... 
			// let's not talk about this
			const float dummyOffset = -999;
			for (int i = 0; i < 3; i++)
				dummyMatrix[3][i] = dummyOffset;
			
			autoptr EntitySpawnParams spawnParams = new EntitySpawnParams();
			spawnParams.TransformMode = ETransformMode.WORLD;
			for (int comp = 0; comp < 4; comp++)
			{
				spawnParams.Transform[comp] = dummyMatrix[comp];
			}
			
			// Spawn dummy object to read mins and maxes
			IEntity dummyObject;
			switch (type)
			{
				case GridSpawnerObjectType.GSOT_TEMPLATE:
					dummyObject = GetGame().SpawnEntityPrefab(resource, baseWorld, spawnParams);
				break;
				
				case GridSpawnerObjectType.GSOT_MODEL:
					VObject obj = resource.GetResource().ToVObject();
					dummyObject = GetGame().SpawnEntity(GenericEntity, baseWorld, spawnParams);
					dummyObject.SetObject(obj, "");
				break;
			}
			
			// Read data from dummy object and dispose of it
			if (dummyObject)
			{
				vector min, max;
				dummyObject.GetBounds(min, max);
				// Do not take height into account
				max[1] = min[1];
				
				float boundsSpan = vector.Distance(min, max);
				spacing += boundsSpan;
				
				// Delete dummy object
				// NOTE: If children don't have Hierarchy component,
				// they will "leak" into the world and never get removed
				SCR_EntityHelper.DeleteEntityAndChildren(dummyObject);
				dummyObject = null;
			}
		}
		
		// Spawn each object on a grid
		for (int i = 0; i < countPerAxis; i++)
		{
			for (int j = 0; j < countPerAxis; j++)
			{
				// Get identity matrix
				vector mat[4];
				Math3D.MatrixIdentity4(mat);
				
				// Set rotation if we are using randomized yaw
				if (m_iRandomizationSeed != 0)
				{
					vector yawPitchRoll = Vector(Math.RandomFloat(0, 360), 0.0, 0.0);
					Math3D.AnglesToMatrix(yawPitchRoll, mat);
				}
				
				// Calculate and set new position
				vector newPos = currentPosition;
				newPos[0] = newPos[0] + i * spacing;
				newPos[2] = newPos[2] + j * spacing;
				mat[3] = newPos;
				
				// Prepare spawn params so entity is spawned
				// at the proper position and with proper rotation
				// straight away
				autoptr EntitySpawnParams spawnParams = new EntitySpawnParams();
				spawnParams.TransformMode = ETransformMode.WORLD;
				for (int comp = 0; comp < 4; comp++)
				{
					spawnParams.Transform[comp] = mat[comp];
				}
				
				// Either spawn a template or create a new generic entity,
				// that will be assigned a model based on GridSpawnerObjectType
				switch (type)
				{
					case GridSpawnerObjectType.GSOT_TEMPLATE:
						spawnedObject = GetGame().SpawnEntityPrefab(resource, baseWorld, spawnParams);
					break;
					
					case GridSpawnerObjectType.GSOT_MODEL:
						VObject obj = resource.GetResource().ToVObject();
						spawnedObject = GetGame().SpawnEntity(GenericEntity, baseWorld, spawnParams);
						spawnedObject.SetObject(obj, "");
						spawnedObject.SetFlags(EntityFlags.VISIBLE, true);
					break;
				}
				
				// Register entity
 				m_aSpawnedEntities.Insert(spawnedObject);
			}
		}
		
		// Set new center point
		centerPoint[0] =  0.5 * (float)countPerAxis * spacing;
		centerPoint[2] = 0.5 * (float)countPerAxis * spacing;
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void DeleteEntities()
	{
		if (m_aSpawnedEntities)
		{
			int count = m_aSpawnedEntities.Count();
			
			// Iterate trough all entities that were spawned
			for (int i = count-1; i >= 0; i--)
			{
				if (m_aSpawnedEntities[i])
				{
					SCR_EntityHelper.DeleteEntityAndChildren( m_aSpawnedEntities[i] );
					
					delete m_aSpawnedEntities[i];
					m_aSpawnedEntities[i] = null;
				}
			}
			
			m_aSpawnedEntities.Clear();
			m_aSpawnedEntities = null;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void EOnFrame(IEntity owner, float timeSlice)
	{
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void EOnInit(IEntity owner)
	{
		// Is in playmode?
		if (!GetGame().GetWorldEntity())
			return;
		
		// Load list from file?
		if (m_sObjectsListPath != string.Empty)
		{
			// Open file
			FileHandle file = FileIO.OpenFile(m_sObjectsListPath, FileMode.READ);
			if (file)
			{
				// Make some room, read data
				m_aObjects.Clear();
				string line = string.Empty;
				while (file.ReadLine(line) > 0)
				{
					if (line != string.Empty)
						m_aObjects.Insert(line);
				}
				
				file.Close();
			}
		}
		
		// Set randomization seed, if we want randomization (!= 0)
		// Note: The generator is shared - seeding may not be deterministic
		if (m_iRandomizationSeed != 0)
		{
			s_AIRandomGenerator.SetSeed(m_iRandomizationSeed);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_GridSpawnerEntity(IEntitySource src, IEntity parent)
	{
		SetEventMask(EntityEvent.POSTFRAME | EntityEvent.FRAME | EntityEvent.INIT);
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_GridSpawnerEntity()
	{
		if (m_aSpawnedEntities)
			DeleteEntities();
	}
};
