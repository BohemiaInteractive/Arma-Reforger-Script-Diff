/*
===========================================
Do not modify, this script is generated
===========================================
*/

#ifdef WORKBENCH

/*!
\addtogroup WorkbenchAPI_GeoShapes
\{
*/

/*!
GeoShape's attribute set.
Note: For ESRI Shapefiles, all shapes in a Shapefile will have the same attributes,
i.e. Count(), GetName(), GetType(), GetIndexByName() and HasAttrib() will always
return the same value for all shapes in the file for a given arguments.
THIS MAY NOT BE TRUE FOR OTHER FORMATS (e.g. GeoJSON).
*/
sealed class GeoAttribCollection: Managed
{
	private void GeoAttribCollection();

	//! Total number of attributes in the collection.
	proto external int Count();
	/*!
	Gets attribute's name.
	\param	index	attribute index [0 .. count-1]
	*/
	proto external string GetName(int index);
	/*!
	Gets attribute's data type. Determines which Get*(int index) method should
	be use to read the value.
	\param	index	attribute index [0 .. count-1]
	*/
	proto external GeoAttribType GetType(int index);
	/*!
	True iff the attribute has a value (is not null).
	Note: This may not be reliable with ESRI shapefiles as they don't officially
	support null values.
	\param	index	attribute index [0 .. count-1]
	*/
	proto external bool IsAttribSet(int index);
	/*!
	Gets an attribute's value as int.
	\param	index	attribute index [0 .. count-1]
	\return	Original value if attrib's type is INT, floored value if attrib's
			type is FLT, 0 otherwise.
	*/
	proto external int GetInt(int index);
	/*!
	Gets an attribute's value as float.
	\param	index	attribute index [0 .. count-1]
	\return	Original value if attrib's type is FLT, type-casted value if
			attrib's type is INT, 0 otherwise.
	*/
	proto external float GetFloat(int index);
	/*!
	Gets an attribute's value as string.
	\param	index	attribute index [0 .. count-1]
	\return	Original value if attrib's type is STR, otherwise it yields in
			undefined behavior.
	*/
	proto external string GetString(int index);
	/*!
	Gets an attribute's index by its name.
	\param	name	attribute's name
	\return	Attribute's index or -1 if there is no such attribute.
	*/
	proto external int GetIndexByName(string name);
	/*!
	\return	True iff a given attribute exists in a collection. (Still, it may
			not be set: may have no value, be null).
	\param	name	attribute's name
	*/
	proto external bool HasAttrib(string name);
	/*!
	True iff the attribute exists and has a value (is not null).
	Note: All *ByName(string) method versions are slower than their counterparts
	using attribute's index.
	\param	name	attribute's name
	\see	IsAttribSet(int index)
	*/
	proto external bool IsAttribSetByName(string name);
	/*!
	Gets an attribute's value as int.
	Note: All *ByName(string) method versions are slower than their counterparts
	using attribute's index.
	\param	name	attribute's name
	\return	Original value if attrib's type is INT, floored value if attrib's
			type is FLT, 0 if the attribute does not exist or is of another type.
	*/
	proto external int GetIntByName(string name);
	/*!
	Gets an attribute's value as float.
	Note: All *ByName(string) method versions are slower than their counterparts
	using attribute's index.
	\param	name	attribute's name
	\return	Original value if attrib's type is FLT, type-casted value if
			attrib's type is INT, 0 if the attribute does not exist or is of
			another type.
	*/
	proto external float GetFloatByName(string name);
	/*!
	Gets an attribute's value as string.
	Note: All *ByName(string) method versions are slower than their counterparts
	using attribute's index.
	\param      name  attribute's name
	\return     Original value if attrib's type is STR, undefined behavior
							if the attribute does not exist or is of another type.
	*/
	proto external string GetStringByName(string name);
}

/*!
\}
*/

#endif // WORKBENCH
