/*!
\defgroup EventSystem Event System
\addtogroup EventSystem
\{
\see \ref Page_EventSystem
\}
*/

/*!
\page Page_EventSystem %Event System

\tableofcontents

\section EventSystem_description Description

 The %Event System consists of two main components: an event provider and an event receiver. Events flow in one direction — from the provider to the receiver.

 To ensure proper usage and maintainable design, only specific classes are allowed to act as providers or receivers. These restrictions offer better control over the system and help prevent misuse. Compatibility and connections are validated during compile-time.

 \section EventSystem_event_provider Event Provider

 - Must inherit from the EventProvider base class.
 - To maintain clarity and simplicity, only selected native classes are allowed to inherit from EventProvider. This may vary between projects. For scripting scenarios, the class is sealed to disallow custom script-based providers. Valid providers typically include:
   - \ref Page_WorldSystems "WorldSystem"
   - \ref WorldSystemDocs_WorldController "WorldController"
 - Contains methods marked with [EventAttribute], representing the events it can broadcast.

 \subsection EventSystem_event_provider_api API

 - \b ConnectEvent  
   Establishes a connection between an event on the provider and a callback method on the receiver.
   - The provider may restrict which receiver types are accepted.
   - Receiver callbacks must have a compatible signature with the event.

 - \b DisconnectEvent  
   Removes a specific event connection between a provider and a receiver.
   - Alternatively, can be used to remove all connections between a specific provider and receiver.

 - \b ThrowEvent  
   Invokes all registered callbacks for an event.
   - Callbacks are executed in the order they were connected.
   - Arguments must match the event declaration.

\remarks All connections use weak pointers internally. However, it is strongly recommended to explicitly disconnect events when a receiver is being destroyed to avoid orphaned connections.

\subsection EventSystem_event_provider_example Example
\snippet this Event Provider Example


\section EventSystem_event_receiver Event Receiver

 - Technically, any managed object may act as a receiver.  
   However, each provider can define which receiver types are permitted.
   - For example, WorldSystem and WorldController accept:
     - GenericEntity
     - GenericComponent

 - Receiver methods must be marked with [ReceiverAttribute].  
   These methods can be connected to or disconnected from provider events.

\subsection EventSystem_event_receiver_example Example
\snippet this Event Receiver Example

*/

#ifdef DOXYGEN

//! [Event Provider Example]
class TestWorldSystem: WorldSystem
{
	void Helllo();
	
	[EventAttribute()]
	void SomeEventA(TestWorldSystem sender, int param1, string param2);
	
	[EventAttribute()]
	void SomeEventB(string msg);
	
	void Process()
	{
		ThrowEvent(SomeEventA, this, 45, "hello");
		ThrowEvent(SomeEventB, "wololoo");
	}
}
//! [Event Provider Example]

//! [Event Receiver Example]
class SomeControllerClass: ScriptComponentClass
{
}

class SomeController: ScriptComponent
{
	TestWorldSystem m_system;
	
	
	[ReceiverAttribute()]
	void OnSomeEventA1(TestWorldSystem sender, int param1, string param2)
	{
		Print("OnSomeEventA1");
	}
	
	[ReceiverAttribute()]
	void OnSomeEventA2(TestWorldSystem sender, int param1, string param2)
	{
		Print("OnSomeEventA2");
	}
		
	[ReceiverAttribute()]
	void OnSomeEventB(string msg)
	{
		Print("OnSomeEventB");
	}
	
	void Init(TestWorldSystem system)
	{
		m_system = system;
		TestWorldSystem.ConnectEvent(system.SomeEventA, this.OnSomeEventA1);
		TestWorldSystem.ConnectEvent(system.SomeEventA, this.OnSomeEventA2);
		TestWorldSystem.ConnectEvent(system.SomeEventB, this.OnSomeEventB);
	}
	
	void ~SomeController()
	{
		EventProvider.DisconnectEvents(m_system, this);
	}
}
//! [Event Receiver Example]

#endif