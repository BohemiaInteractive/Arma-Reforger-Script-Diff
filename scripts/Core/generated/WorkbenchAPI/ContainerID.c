/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup WorkbenchAPI
\{
*/

/*!
Represents "path" from root container to specific nested object inside config hierarchy.
Each ContainerIdPathEntry item represent one level of hierarchy, and it has:
	-	PropertyName: name of Object or Object array property
	- Index: if the property defined by PropertyName is an array, index is used, otherwise leave it -1 (the default)

Let's have the following config:
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
				Metadata { // <- for example: path to this object is: Points[1]/Metadata
					Name name2
				}
			}
		}
	}
\endcode

Setup ContainerID pointing to second ShapePoint Metadata:
\code
	BaseContainer rootContainer; // the root container point to "Shape"

	ContainerID containerPath = {
		new ContainerIdPathEntry("Points", 1), // second item in "Points" object array
		new ContainerIdPathEntry("Metadata") // "Metadata" nested object of the second point
	};
\endcode
*/
sealed class ContainerID: array<ref ContainerIdPathEntry>
{
	/*!
	Helper method which prepare config object for editing. As nested objects or object arrays can be inherited from another config they need to be setup properly before editing.

	\note Use this method only when editing outside WorldEditor. In WorldEditor, when editing entities or components, just pass ContainerID to WorldEditorAPI.SetVariableValue and let undo stack to do the job.

	Let's have an example container from \ref ContainerID
	To set a Name in Metadata of the second shape point, we do:
	\code
		BaseContainer rootContainer = LoadMe(); // the root container point to "Shape"

		ContainerID containerPath = {
			new ContainerIdPathEntry("Points", 1), // Take the second point
			new ContainerIdPathEntry("Metadata") // Go to metadata of the second point
		};

		BaseContainer shapePoint = containerPath.GetOrCreateDirectPtr(rootContainer);
		if (shapePoint)
			shapePoint.Set("Name", "my new name");

		SaveMe(rootContainer);
	\endcode

	\param topLevel Container from which the search is started.
	\return Pointer to BaseContainer which is ready to modifications.
	*/
	proto external BaseContainer GetOrCreateDirectPtr(notnull BaseContainer topLevel);
}

/*!
\}
*/
