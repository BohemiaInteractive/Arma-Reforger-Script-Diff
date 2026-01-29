[EntityEditorProps(category: "GameScripted/Editor", description: "Core Editor manager", color: "251 91 0 255")]
class SCR_EditorBaseEntityClass: GenericEntityClass
{
};

/** @ingroup Editor_Entities
*/

/*!
Base editor entity which provides overridable functions.

Editor components (SCR_BaseEditorComponent) can be attached only to entities inheriting from this base class.
*/
class SCR_EditorBaseEntity : GenericEntity
{
	/*! @name Owner Invokers
	Script invokers called on owner's machine.
	*/
	///@{
	/*!
	Get event which is called when editor is initialized
	\return script invoker
	*/
	ScriptInvoker GetOnInit()
	{
		return null;
	}
	/*!
	Get event which is called when a request to open/close the editor is sent to server
	\return script invoker
	*/
	ScriptInvoker GetOnRequest()
	{
		return null;
	}
	/*!
	Get event which is called when editor is opened
	\return script invoker
	*/
	ScriptInvoker GetOnOpened()
	{
		return null;
	}
	/*!
	Get event which is called before the entity is activated.
	\return script invoker
	*/
	ScriptInvoker GetOnPreActivate()
	{
		return null;
	}
	/*!
	Get event which is called when the entity is activated.
	\return script invoker
	*/
	ScriptInvoker GetOnActivate()
	{
		return null;
	}
	/*!
	Get event which is called when the entity is activated, but after all GetOnActivate() functions were called
	\return script invoker
	*/
	ScriptInvoker GetOnPostActivate()
	{
		return null;
	}
	/*!
	Get event which is called when the entity is deactivated.
	\return script invoker
	*/
	ScriptInvoker GetOnDeactivate()
	{
		return null;
	}
	/*!
	Get event which is called when the editor is closed
	\return script invoker
	*/
	ScriptInvoker GetOnClosed()
	{
		return null;
	}
	/*!
	Get event which is called every frame when debug is active
	\return script invoker
	*/
	ScriptInvoker GetOnDebug()
	{
		return null;
	}
	///@}
	
	/*! @name Server Invokers
	Script invokers called on server.
	*/
	///@{
	/*!
	Get event which is called when editor is initialized
	\return script invoker
	*/
	ScriptInvoker GetOnOpenedServer()
	{
		return null;
	}
	/*!
	Get event which is called when editor is initialized
	\return script invoker
	*/
	ScriptInvoker GetOnOpenedServerCallback()
	{
		return null;
	}
	/*!
	Get event which is called when editor is initialized
	\return script invoker
	*/
	ScriptInvoker GetOnClosedServer()
	{
		return null;
	}
	/*!
	Get event which is called when editor is initialized and async loading finishes
	\return script invoker
	*/
	ScriptInvoker GetOnClosedServerCallback()
	{
		return null;
	}
	/*!
	Get event which is called when the entity is activated.
	\return script invoker
	*/
	ScriptInvoker GetOnActivateServer()
	{
		return null;
	}
	/*!
	Get event which is called when the entity is deactivated.
	\return script invoker
	*/
	ScriptInvoker GetOnDeactivateServer()
	{
		return null;
	}
	///@}
	
	/*!
	Check if editor is currently opened.
	\return True if opened
	*/
	bool IsOpened()
	{
		return false;
	}
	/*!
	Get editor manager
	\return Editor manager
	*/
	SCR_EditorManagerEntity GetManager()
	{
		return null;
	}
	/*!
	Get all editor components.
	\param[out] outComponents Array to be filled by components (it's not cleared before filling!)
	\return Number of components
	*/
	int FindEditorComponents(out array<SCR_BaseEditorComponent> outComponents)
	{
		if (!outComponents) outComponents = {};
		array<Managed> components = {};
		for (int i = 0, count = FindComponents(SCR_BaseEditorComponent, components); i < count; i++)
		{
			outComponents.Insert(SCR_BaseEditorComponent.Cast(components[i]));
		}
		return outComponents.Count();
	}
	
	protected void InitComponents(bool isServer)
	{
		array<Managed> components = {};
		for (int i = 0, count = FindComponents(SCR_BaseEditorComponent, components); i < count; i++)
		{
			InitComponent(SCR_BaseEditorComponent.Cast(components[i]), isServer);
		}
	}
	protected void InitComponent(SCR_BaseEditorComponent component, bool isServer)
	{
		if (isServer)
			component.InitServer();
		else
			component.InitOwner();
		
		//--- Initialize sub-components
		array<GenericComponent> components = {};
		int componentsCount = component.FindComponents(SCR_BaseEditorComponent, components);
		for (int i = 0; i < componentsCount; i++)
		{
			InitComponent(SCR_BaseEditorComponent.Cast(components[i]), isServer);
		}
	}
};