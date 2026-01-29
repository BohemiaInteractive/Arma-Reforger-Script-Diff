class SCR_EntitySourceHelper
{
	//------------------------------------------------------------------------------------------------
	//! Copies bool, float, int, string (including ResourceName), vector and object values from the old to the new component source
	//! using World Editor API
	//! \param[in] entitySource
	//! \param[in] entityDestination
	//! \param[in] componentSource
	//! \param[in] componentDestination
	//! \return true on success, false otherwise
	static bool CopyDataFromOldToNewComponent(notnull IEntitySource entitySource, notnull IEntityComponentSource componentSource, notnull IEntitySource entityDestination, notnull IEntityComponentSource componentDestination)
	{
		// entitySources can be the same, components not
		if (componentSource == componentSource)
			return false;

		int sourceId = -1;
		int destinationId = -1;
		IEntityComponentSource testedComponent;
		for (int i = entitySource.GetComponentCount() - 1; i >= 0; --i)
		{
			testedComponent = entitySource.GetComponent(i);
			if (testedComponent == componentSource)
			{
				sourceId = i;
				break;
			}
		}

		if (sourceId == -1)
		{
			Print("Source component was not found in the provided source entity", LogLevel.ERROR);
			return false;
		}

		for (int i = entityDestination.GetComponentCount() - 1; i >= 0; --i)
		{
			testedComponent = entityDestination.GetComponent(i);
			if (testedComponent == componentDestination)
			{
				destinationId = i;
				break;
			}
		}

		if (destinationId == -1)
		{
			Print("Destination component was not found in the provided destination entity", LogLevel.ERROR);
			return false;
		}

		return CopyVariablesFromTo(entitySource, componentSource, { new ContainerIdPathEntry("components", destinationId) });
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] entitySource
	//! \param[in] source
	//! \param[in] path
	//! \return
	// This method may be renamed later
	protected static bool CopyVariablesFromTo(IEntitySource entitySource, IEntityComponentSource source, array<ref ContainerIdPathEntry> path)
	{
		WorldEditorAPI worldEditorAPI = SCR_WorldEditorToolHelper.GetWorldEditorAPI();

		string varName;
		bool bValue;
		float fValue;
		int iValue;
		string sValue;
		vector vValue;
		array<bool> abValue;
		array<ref Color> acValue;
		array<float> afValue;
		array<int> aiValue;
		array<string> asValue;
		array<vector> avValue;
		for (int i, count = source.GetNumVars(); i < count; i++)
		{
			varName = source.GetVarName(i);
			if (!source.IsVariableSetDirectly(varName))
				continue;

			switch (source.GetDataVarType(i))
			{	// oh noes, the formatting
				case DataVarType.BOOLEAN:	{ if (source.Get(varName, bValue) && worldEditorAPI.SetVariableValue(entitySource, path, varName, bValue.ToString(true))) continue; break; }
				case DataVarType.SCALAR:	{ if (source.Get(varName, fValue) && worldEditorAPI.SetVariableValue(entitySource, path, varName, fValue.ToString())) continue; break; }
				case DataVarType.INTEGER:	{ if (source.Get(varName, iValue) && worldEditorAPI.SetVariableValue(entitySource, path, varName, iValue.ToString())) continue; break; }
				case DataVarType.RESOURCE_NAME:
				case DataVarType.STRING:	{ if (source.Get(varName, sValue) && worldEditorAPI.SetVariableValue(entitySource, path, varName, sValue)) continue; break; }
				case DataVarType.VECTOR3:	{ if (source.Get(varName, vValue) && worldEditorAPI.SetVariableValue(entitySource, path, varName, vValue.ToString(false))) continue; break; }

				case DataVarType.OBJECT:
				{
					if (!source.Get(varName, null))
						break;

					BaseContainer sourceObj = source.GetObject(varName);
					if (!sourceObj) // it's null, let's make sure it is null there too
					{
						if (worldEditorAPI.ClearVariableValue(entitySource, path, varName))
							continue;

						break;
					}

					// create, no check if exists
					if (!worldEditorAPI.CreateObjectVariableMember(entitySource, path, varName, sourceObj.GetClassName()))
						break;

					array<ref ContainerIdPathEntry> path2 = SCR_ArrayHelperRefT<ContainerIdPathEntry>.GetCopy(path);
					path2.Insert(new ContainerIdPathEntry(varName));

					if (CopyVariablesFromTo(entitySource, sourceObj, path2))
						continue;

					break;
				}

				// arrays

				case DataVarType.SCALAR_ARRAY:			{ if (source.Get(varName, afValue) && worldEditorAPI.SetVariableValue(entitySource, path, varName, SCR_StringHelper.Join(",", afValue))) continue; break; }
//				case DataVarType.VECTOR2_ARRAY:			{ break; }
//				case DataVarType.VECTOR3_ARRAY:			{ break; }
//				case DataVarType.VECTOR4_ARRAY:			{ break; }
//				case DataVarType.MATRIX_ARRAY:			{ break; }
//				case DataVarType.MATRIX34_ARRAY:		{ break; }
//				case DataVarType.COLOR_ARRAY:			{ break; }
//				case DataVarType.INTEGER_ARRAY:			{ break; }
//				case DataVarType.TEXTURE_ARRAY:			{ break; }
//				case DataVarType.BOOLEAN_ARRAY:			{ break; }
//				case DataVarType.RESOURCE_NAME_ARRAY:
//				case DataVarType.STRING_ARRAY:			{ break; }
//				case DataVarType.POINTER_ARRAY:			{ break; }
//				case DataVarType.FLAGS_ARRAY:			{ break; }
//				case DataVarType.OBJECT_ARRAY:			{ break; }

				default:
				{
					Print("" + source.GetDataVarType(i) + " not supported", LogLevel.WARNING);
					continue; // do not fail yet, many missing types as commented above
				}
			}

			Print("A variable set manually cannot be auto-converted and must be done by hand: " + varName, LogLevel.WARNING);
			return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] value
	//! \return
	static string GetShortestVectorString(vector value)
	{
		return string.Format("%1 %2 %3", value[0], value[1], value[2]); // 20% faster than ("" + value[0] + " " + value[1] + " " + value[2]);
	}
}
