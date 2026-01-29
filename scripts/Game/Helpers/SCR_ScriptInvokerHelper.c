//------------------------------------------------------------------------------------------------
// Generic Script invokers. To be used in any script that does not need specific invokers
// If a series of types comes up often, it is good to add it here (under a generic name)
//------------------------------------------------------------------------------------------------

// default no parameters type
void ScriptInvokerVoidMethod();
typedef func ScriptInvokerVoidMethod;
typedef ScriptInvokerBase<ScriptInvokerVoidMethod> ScriptInvokerVoid;

// common values

// int
void ScriptInvokerIntMethod(int i);
void ScriptInvokerInt2Method(int i1, int i2);
void ScriptInvokerInt3Method(int i1, int i2, int i3);
void ScriptInvokerInt4Method(int i1, int i2, int i3, int i4);
void ScriptInvokerInt5Method(int i1, int i2, int i3, int i4, int i5);
typedef func ScriptInvokerIntMethod;
typedef func ScriptInvokerInt2Method;
typedef func ScriptInvokerInt3Method;
typedef func ScriptInvokerInt4Method;
typedef func ScriptInvokerInt5Method;
typedef ScriptInvokerBase<ScriptInvokerIntMethod> ScriptInvokerInt;
typedef ScriptInvokerBase<ScriptInvokerInt2Method> ScriptInvokerInt2;
typedef ScriptInvokerBase<ScriptInvokerInt3Method> ScriptInvokerInt3;
typedef ScriptInvokerBase<ScriptInvokerInt4Method> ScriptInvokerInt4;
typedef ScriptInvokerBase<ScriptInvokerInt5Method> ScriptInvokerInt5;

// bool
void ScriptInvokerBoolMethod(bool b);
void ScriptInvokerBool2Method(bool b1, bool b2);
void ScriptInvokerBool3Method(bool b1, bool b2, bool b3);
void ScriptInvokerBool4Method(bool b1, bool b2, bool b3, bool b4);
void ScriptInvokerBool5Method(bool b1, bool b2, bool b3, bool b4, bool b5);
typedef func ScriptInvokerBoolMethod;
typedef func ScriptInvokerBool2Method;
typedef func ScriptInvokerBool3Method;
typedef func ScriptInvokerBool4Method;
typedef func ScriptInvokerBool5Method;
typedef ScriptInvokerBase<ScriptInvokerBoolMethod> ScriptInvokerBool;
typedef ScriptInvokerBase<ScriptInvokerBool2Method> ScriptInvokerBool2;
typedef ScriptInvokerBase<ScriptInvokerBool3Method> ScriptInvokerBool3;
typedef ScriptInvokerBase<ScriptInvokerBool4Method> ScriptInvokerBool4;
typedef ScriptInvokerBase<ScriptInvokerBool5Method> ScriptInvokerBool5;

// float
void ScriptInvokerFloatMethod(float f);
void ScriptInvokerFloat2Method(float f1, float f2);
void ScriptInvokerFloat3Method(float f1, float f2, float f3);
void ScriptInvokerFloat4Method(float f1, float f2, float f3, float f4);
void ScriptInvokerFloat5Method(float f1, float f2, float f3, float f4, float f5);
typedef func ScriptInvokerFloatMethod;
typedef func ScriptInvokerFloat2Method;
typedef func ScriptInvokerFloat3Method;
typedef func ScriptInvokerFloat4Method;
typedef func ScriptInvokerFloat5Method;
typedef ScriptInvokerBase<ScriptInvokerFloatMethod> ScriptInvokerFloat;
typedef ScriptInvokerBase<ScriptInvokerFloat2Method> ScriptInvokerFloat2;
typedef ScriptInvokerBase<ScriptInvokerFloat3Method> ScriptInvokerFloat3;
typedef ScriptInvokerBase<ScriptInvokerFloat4Method> ScriptInvokerFloat4;
typedef ScriptInvokerBase<ScriptInvokerFloat5Method> ScriptInvokerFloat5;

// string
void ScriptInvokerStringMethod(string s);
void ScriptInvokerString2Method(string s1, string s2);
void ScriptInvokerString3Method(string s1, string s2, string s3);
void ScriptInvokerString4Method(string s1, string s2, string s3, string s4);
void ScriptInvokerString5Method(string s1, string s2, string s3, string s4, string s5);
typedef func ScriptInvokerStringMethod;
typedef func ScriptInvokerString2Method;
typedef func ScriptInvokerString3Method;
typedef func ScriptInvokerString4Method;
typedef func ScriptInvokerString5Method;
typedef ScriptInvokerBase<ScriptInvokerStringMethod> ScriptInvokerString;
typedef ScriptInvokerBase<ScriptInvokerString2Method> ScriptInvokerString2;
typedef ScriptInvokerBase<ScriptInvokerString3Method> ScriptInvokerString3;
typedef ScriptInvokerBase<ScriptInvokerString4Method> ScriptInvokerString4;
typedef ScriptInvokerBase<ScriptInvokerString5Method> ScriptInvokerString5;

// IEntity
void ScriptInvokerEntityMethod(IEntity e);
void ScriptInvokerEntity2Method(IEntity e1, IEntity e2);
//void ScriptInvokerEntity3Method(IEntity e1, IEntity e2, IEntity e3);
//void ScriptInvokerEntity4Method(IEntity e1, IEntity e2, IEntity e3, IEntity e4);
//void ScriptInvokerEntity5Method(IEntity e1, IEntity e2, IEntity e3, IEntity e4, IEntity e5);
typedef func ScriptInvokerEntityMethod;
typedef func ScriptInvokerEntity2Method;
//typedef func ScriptInvokerEntity3Method;
//typedef func ScriptInvokerEntity4Method;
//typedef func ScriptInvokerEntity5Method;
typedef ScriptInvokerBase<ScriptInvokerEntityMethod> ScriptInvokerEntity;
typedef ScriptInvokerBase<ScriptInvokerEntity2Method> ScriptInvokerEntity2;
//typedef ScriptInvokerBase<ScriptInvokerEntity3Method> ScriptInvokerEntity3;
//typedef ScriptInvokerBase<ScriptInvokerEntity4Method> ScriptInvokerEntity4;
//typedef ScriptInvokerBase<ScriptInvokerEntity5Method> ScriptInvokerEntity5;

// vector
void ScriptInvokerVectorMethod(vector v);
typedef func ScriptInvokerVectorMethod;
typedef ScriptInvokerBase<ScriptInvokerVectorMethod> ScriptInvokerVector;

// Widget
void ScriptInvokerWidgetMethod(Widget w);
typedef func ScriptInvokerWidgetMethod;
typedef ScriptInvokerBase<ScriptInvokerWidgetMethod> ScriptInvokerWidget;

// SCR_ScriptedWidgetComponent
void ScriptInvokerSCRScriptedWidgetComponentMethod(SCR_ScriptedWidgetComponent c);
typedef func ScriptInvokerSCRScriptedWidgetComponentMethod;
typedef ScriptInvokerBase<ScriptInvokerSCRScriptedWidgetComponentMethod> ScriptInvokerSCRScriptedWidgetComponent;

// BaseWorld
void ScriptInvokerBaseWorldMethod(BaseWorld w);
typedef func ScriptInvokerBaseWorldMethod;
typedef ScriptInvokerBase<ScriptInvokerBaseWorldMethod> ScriptInvokerBaseWorld;

// RplId
void ScriptInvokerRplIdMethod(RplId id);
typedef func ScriptInvokerRplIdMethod;
typedef ScriptInvokerBase<ScriptInvokerRplIdMethod> ScriptInvokerRplId;

//IEntity and BaseInventoryStorageComponent
void ScriptInvokerEntityAndStorageMethod(IEntity entity, BaseInventoryStorageComponent storageComponent);
typedef func ScriptInvokerEntityAndStorageMethod;
typedef ScriptInvokerBase<ScriptInvokerEntityAndStorageMethod> ScriptInvokerEntityAndStorage;

//~ Faction
void ScriptInvokerFactionMethod(Faction faction);
typedef func ScriptInvokerFactionMethod;
typedef ScriptInvokerBase<ScriptInvokerFactionMethod> ScriptInvokerFaction;

// this is an example class to show how to define and use script invokers
/*
class SCR_ExampleClass
{
	// define the event
	protected ref ScriptInvokerInt m_OnExampleEvent;

	// return script invoker
	ScriptInvokerInt GetOnExampleEvent()
	{
		if (!m_OnExampleEvent)
			m_OnExampleEvent = new ScriptInvokerInt();

		return m_OnExampleEvent;
	}

	protected void ExampleInvoke()
	{
		int exampleInt;
		if (m_OnExampleEvent)	// null if nobody subscribed to it from the outside
			m_OnExampleEvent.Invoke(exampleInt);
	}

	protected void MyExampleFunction(int exampleInt)
	{
		Print(string.Format("I was executed %1", exampleInt), LogLevel.VERBOSE);
	}

	protected void ExampleInsert()
	{
		SCR_ExampleClass exampleClass; // get this from somewhere
		// ...
		exampleClass.GetOnExampleEvent().Insert(MyExampleFunction);
	}

	protected void ExampleRemove()
	{
		SCR_ExampleClass exampleClass; // get this from somewhere
		// ...
		exampleClass.GetOnExampleEvent().Remove(MyExampleFunction);
	}
}
*/
