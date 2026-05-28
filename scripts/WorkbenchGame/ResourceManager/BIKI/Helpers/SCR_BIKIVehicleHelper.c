#ifdef WORKBENCH
//! - expressed speed is in km/h
//! - expressed torque is in Nm
//! - expressed power is in kW (unless specified otherwise)
class SCR_BIKIVehicleHelper
{
	static const float KW2HP = 1.34102;					// mechanical horsepower
	static const float HP2KW = 0.73549875;

	static const float KW2PS = 1.3596216173039;			// metric horsepower
	static const float PS2KW = 0.7457;

	static const float KW2NMRPM = 9549.296585513720;	// metric units FTW
	static const float NMRPM2KW = 0.000104719751;

	//------------------------------------------------------------------------------------------------
	//! \param[in] vehicleContainer
	//! \return
	static array<float> GetGearRatios(notnull BaseContainer vehicleContainer)
	{
		BaseContainer baseContainer = GetSimulationContainer(vehicleContainer);
		if (!baseContainer)
			return {};

		baseContainer = baseContainer.GetObject("Gearbox");
		if (!baseContainer)
			return {};

		array<float> result;
		if (!baseContainer.Get("Forward", result))
			result = {};

		float reverse;
		if (baseContainer.Get("Reverse", reverse))
			result.InsertAt(-reverse, 0);

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] vehicleContainer
	//! \return
	static bool GetAutomaticGearbox(notnull BaseContainer vehicleContainer)
	{
		array<IEntityComponentSource> componentSources = {};
		if (SCR_BaseContainerTools.FindComponentSourcesOfClass(vehicleContainer, SCR_CarControllerComponent, true, componentSources) < 1)
			return false;

		IEntityComponentSource componentSource = componentSources[0];
		if (!componentSource)
			return false;

		int gearboxMode;
		if (!componentSource.Get("Type", gearboxMode))
			return false;

		return gearboxMode == 1; // 0 = manual, 1 = automatic
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] vehicleContainer
	//! \return
	static float GetInitialFuelQuantity(notnull BaseContainer vehicleContainer)
	{
		IEntityComponentSource componentSource = SCR_BaseContainerTools.FindComponentSource(vehicleContainer, SCR_FuelManagerComponent);
		if (!componentSource)
			return 0;

		BaseContainerList list = componentSource.GetObjectArray("FuelNodes");
		if (!list)
			return 0;

		BaseContainer baseContainer;
		float result;
		for (int i, count = list.Count(); i < count; ++i)
		{
			float maxFuel;
			list.Get(i).Get("m_fInitialFuelTankState", maxFuel);
			result += maxFuel;
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] vehicleContainer
	//! \return
	static float GetFuelCapacity(notnull BaseContainer vehicleContainer)
	{
		IEntityComponentSource componentSource = SCR_BaseContainerTools.FindComponentSource(vehicleContainer, SCR_FuelManagerComponent);
		if (!componentSource)
			return 0;

		BaseContainerList list = componentSource.GetObjectArray("FuelNodes");
		if (!list)
			return 0;

		BaseContainer baseContainer;
		float result;
		for (int i, count = list.Count(); i < count; ++i)
		{
			float maxFuel;
			list.Get(i).Get("MaxFuel", maxFuel);
			result += maxFuel;
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] vehicleContainer
	//! \return
	static float GetFuelIdleConsumption(notnull BaseContainer vehicleContainer)
	{
		IEntityComponentSource componentSource = SCR_BaseContainerTools.FindComponentSource(vehicleContainer, SCR_FuelConsumptionComponent);
		if (!componentSource)
			return 0;

		float result;
		componentSource.Get("m_fFuelConsumptionIdle", result);
		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] vehicleContainer
	//! \return
	static float GetFuelMaxConsumption(notnull BaseContainer vehicleContainer)
	{
		IEntityComponentSource componentSource = SCR_BaseContainerTools.FindComponentSource(vehicleContainer, SCR_FuelConsumptionComponent);
		if (!componentSource)
			return 0;

		float result;
		componentSource.Get("m_fFuelConsumption", result);
		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] vehicleContainer
	//! \return
	static array<EFuelType> GetFuelTypes(notnull BaseContainer vehicleContainer)
	{
		IEntityComponentSource componentSource = SCR_BaseContainerTools.FindComponentSource(vehicleContainer, SCR_FuelManagerComponent);
		if (!componentSource)
			return {};

		BaseContainerList list = componentSource.GetObjectArray("FuelNodes");
		if (!list)
			return {};

		array<EFuelType> result = {};
		BaseContainer baseContainer;
		for (int i, count = list.Count(); i < count; ++i)
		{
			EFuelType fuelType;
			list.Get(i).Get("FuelType", fuelType);
			if (!result.Contains(fuelType))
				result.Insert(fuelType);
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] vehicleContainer
	//! \return
	static array<string> GetFuelTypesAsString(notnull BaseContainer vehicleContainer)
	{
		array<string> result = {};
		foreach (EFuelType fuelType : GetFuelTypes(vehicleContainer))
		{
			result.Insert(SCR_StringHelper.FormatSnakeCaseToUserFriendly(typename.EnumToString(EFuelType, fuelType)));
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] vehicleContainer
	//! \return
	static float GetMechanicalHorsePower(notnull BaseContainer vehicleContainer)
	{
		return GetEnginePowerKW(vehicleContainer) * KW2HP;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] vehicleContainer
	//! \return
	static float GetMetricHorsePower(notnull BaseContainer vehicleContainer)
	{
		return GetEnginePowerKW(vehicleContainer) * KW2PS;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] vehicleContainer
	//! \return
	static float GetEnginePowerKW(notnull BaseContainer vehicleContainer)
	{
		BaseContainer baseContainer = GetEngineContainer(vehicleContainer);
		if (!baseContainer)
			return 0;

		float result;
		if (baseContainer.Get("MaxPower", result)) // wheeled engine
			return result;

		// helicopter?
		baseContainer = GetSimulationContainer(vehicleContainer);
		BaseContainerList baseContainerList = baseContainer.GetObjectArray("Rotors");
		if (!baseContainerList)
			return 0;

		if (baseContainerList.Count() < 1)
			return 0;

		baseContainer = baseContainerList.Get(0); // first rotor = main rotor, lessgo?

		float rpm;
		if (!baseContainer.Get("TargetRPM", rpm))
			return 0;

		vector torque;
		if (!baseContainer.Get("TorqueForce", torque))
			return 0;

		return rpm * torque[1] * NMRPM2KW;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] gearIndex
	//! \param[in] rpm
	//! \return
	static float GetEnginePowerKWByGearAndRPM(notnull BaseContainer vehicleContainer, int gearIndex, float rpm)
	{
		array<float> gearRatios = GetGearRatios(vehicleContainer);
		if (!gearRatios.IsIndexValid(gearIndex)) // covers empty gearRatios as well
			return 0;

		float gearRatio = gearRatios[gearIndex];

		return 0;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] vehicleContainer
	//! \return
	static float GetIdleRPM(notnull BaseContainer vehicleContainer)
	{
		BaseContainer baseContainer = GetEngineContainer(vehicleContainer);
		if (!baseContainer)
			return 0;

		float result;
		if (baseContainer.Get("RpmIdle", result)) // VehicleWheeledSimulation
			return result;

		if (baseContainer.Get("RPMIdle", result)) // VehicleHelicopterSimulation
			return result;

		return 0;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] vehicleContainer
	//! \return
	static float GetRPMRedLine(notnull BaseContainer vehicleContainer)
	{
		BaseContainer baseContainer = GetEngineContainer(vehicleContainer);
		if (!baseContainer)
			return 0;

		float result;
		if (baseContainer.Get("RpmRedline", result)) // VehicleWheeledSimulation
			return result;

		return 0;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] vehicleContainer
	//! \return
	static float GetMaxRPM(notnull BaseContainer vehicleContainer)
	{
		BaseContainer baseContainer = GetEngineContainer(vehicleContainer);
		if (!baseContainer)
			return 0;

		float result;
		if (baseContainer.Get("RpmMax", result)) // VehicleWheeledSimulation
			return result;

		if (baseContainer.Get("RPMMax", result)) // VehicleWheeledSimulation
			return result;

		return 0;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] vehicleContainer
	//! \return
	static float GetDifferentialRatio(notnull BaseContainer vehicleContainer)
	{
		BaseContainer baseContainer = GetSimulationContainer(vehicleContainer);
		if (!baseContainer)
			return 0;

		BaseContainerList baseContainerList = baseContainer.GetObjectArray("Differentials");
		if (!baseContainerList || baseContainerList.Count() < 1)
			return 0;

		baseContainer = baseContainerList.Get(0);
		if (!baseContainer)
			return 0;

		float result;
		baseContainer.Get("Ratio", result);

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] vehicleContainer
	//! \return
	static float GetAxleDifferentialRatio(notnull BaseContainer vehicleContainer)
	{
		BaseContainer baseContainer = GetSimulationContainer(vehicleContainer);
		if (!baseContainer)
			return 0;

		BaseContainerList baseContainerList = baseContainer.GetObjectArray("Axles");
		if (!baseContainerList)
			return 0;

		float result;
		for (int i, count = baseContainerList.Count(); i < count; ++i)
		{
			baseContainer = baseContainerList.Get(i);
			if (!baseContainer)
				continue;

			baseContainer = baseContainer.GetObject("Differential");
			if (!baseContainer)
				continue;

			if (baseContainer.Get("Ratio", result))
				return result;
		}

		return 0;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] vehicleContainer
	//! \return
	static float GetAxleCount(notnull BaseContainer vehicleContainer)
	{
		BaseContainer baseContainer = GetSimulationContainer(vehicleContainer);
		if (!baseContainer)
			return 0;

		BaseContainerList baseContainerList = baseContainer.GetObjectArray("Axles");
		if (!baseContainerList)
			return 0;

		return baseContainerList.Count();
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] vehicleContainer
	//! \return
	static float GetWheelsCircumference(notnull BaseContainer vehicleContainer)
	{
		BaseContainer baseContainer = GetSimulationContainer(vehicleContainer);
		if (!baseContainer)
			return 0;

		BaseContainerList baseContainerList = baseContainer.GetObjectArray("Axles");
		if (!baseContainerList || baseContainerList.Count() < 1)
			return 0;

		baseContainer = baseContainerList.Get(0);
		if (!baseContainer)
			return 0;

		baseContainer = baseContainer.GetObject("Wheel");

		float result;
		baseContainer.Get("Radius", result);

		return 2 * Math.PI * result;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] vehicleContainer
	//! \return max speed in km/h
	static float GetTheoreticalMaxSpeed(notnull BaseContainer vehicleContainer, bool useMaxRPM = false)
	{
		array<float> gearRatios = GetGearRatios(vehicleContainer);
		if (gearRatios.Count() < 1)
			return 0;

		float engineRPM;
		if (useMaxRPM)
			engineRPM = GetMaxRPM(vehicleContainer);
		else
			engineRPM = GetRPMRedLine(vehicleContainer);

		if (SCR_BIKIGeneralHelper.IsWheeled(vehicleContainer))
		{
			float axleDifferentialRatio = GetAxleDifferentialRatio(vehicleContainer);
			float wheelCircumference = GetWheelsCircumference(vehicleContainer);
			return GetTheoreticalSpeed(engineRPM, gearRatios[gearRatios.Count() - 1], axleDifferentialRatio, wheelCircumference);
		}

		if (SCR_BIKIGeneralHelper.IsTracked(vehicleContainer))
		{
			BaseContainer roadWheel = GetSimulationContainer(vehicleContainer).GetObject("RoadWheel");
			if (!roadWheel)
				return 0;

			float wheelCircumference;
			if (!roadWheel.Get("Radius", wheelCircumference))
				return 0;

			return GetTheoreticalSpeed(engineRPM, gearRatios[gearRatios.Count() - 1], 1, wheelCircumference);
		}

		return 0;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] vehicleContainer
	//! \return max speed in km/h
	static array<float> GetTheoreticalMaxSpeedForAllGears(notnull BaseContainer vehicleContainer)
	{
		array<float> result = GetGearRatios(vehicleContainer);
		if (result.IsEmpty())
			return result;

		float engineRPM = GetRPMRedLine(vehicleContainer);
		float axleDifferentialRatio = GetAxleDifferentialRatio(vehicleContainer);
		float wheelCircumference = GetWheelsCircumference(vehicleContainer);
		foreach (int i, float gearRatio : result)
		{
			result[i] = GetTheoreticalSpeed(engineRPM, gearRatio, axleDifferentialRatio, wheelCircumference);
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] engineRPM
	//! \param[in] gearRatio
	//! \param[in] axleDifferentialRatio
	//! \param[in] wheelCircumference
	//! \return gear's max speed in km/h
	protected static float GetTheoreticalSpeed(float engineRPM, float gearRatio, float axleDifferentialRatio, float wheelCircumference)
	{
		if (engineRPM <= 0 || axleDifferentialRatio <= 0 || wheelCircumference <= 0 || gearRatio == 0)
			return 0;

		float axleRPM = engineRPM / (gearRatio * axleDifferentialRatio);
		float groundSpeed = axleRPM * (wheelCircumference / 60) * 3.6;

		return groundSpeed;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] vehicleContainer
	//! \return
	static float GetMaxSpeed(notnull BaseContainer vehicleContainer)
	{
return 0;
		float theoreticalMaxSpeed = GetTheoreticalMaxSpeed(vehicleContainer);
		if (theoreticalMaxSpeed == 0)
			return 0;

		// get air drag
		// AirDrag we use 0.5f * air_density * dragCoef * frontArea * speed^2
		// air_density = 1.225

		// const float airDensity = 1.225;
		// dragCoef = ?
		// float airDrag = 0.5 * airDensity * dragCoef * frontArea * theoreticalMaxSpeed * theoreticalMaxSpeed;

//		return theoreticalMaxSpeed - airDrag;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] baseCompartmentHolder
	//! \return array of number of seats (driver/pilots, gunners, commander, cargo)
	static array<int> GetSeats(notnull BaseContainer baseCompartmentHolder)
	{
		array<int> result = { 0, 0, 0, 0 };

		IEntityComponentSource componentSource = SCR_BaseContainerTools.FindComponentSource(baseCompartmentHolder, SCR_BaseCompartmentManagerComponent);
		BaseContainerList baseContainerList;
		BaseContainer baseContainer;
		typename typeName;
		if (componentSource)
		{
			baseContainerList = componentSource.GetObjectArray("CompartmentSlots");
			if (baseContainerList)
			{
				for (int i, count = baseContainerList.Count(); i < count; ++i)
				{
					baseContainer = baseContainerList.Get(i);
					typeName = baseContainer.GetClassName().ToType();
					if (!typeName)
						continue;

					// commander
					if (IsTurretCommanderSeat(baseContainer))
					{
						result[2] = result[2] + 1;
						continue;
					}

					// gunner
					if (typeName.IsInherited(TurretCompartmentSlot))
					{
						result[1] = result[1] + 1; // gunner
						continue;
					}

					// driver / pilots
					if (typeName.IsInherited(PilotCompartmentSlot))
					{
						result[0] = result[0] + 1;
						continue;
					}

					// cargo
					if (typeName.IsInherited(CargoCompartmentSlot))
					{
						result[3] = result[3] + 1;
						continue;
					}
				}
			}
		}

		componentSource = SCR_BaseContainerTools.FindComponentSource(baseCompartmentHolder, SlotManagerComponent);
		if (componentSource)
		{
			baseContainerList = componentSource.GetObjectArray("Slots");
			if (baseContainerList)
			{
				ResourceName resourceName;
				Resource resource;
				array<int> subResult;
				for (int i, count = baseContainerList.Count(); i < count; ++i)
				{
					baseContainer = baseContainerList.Get(i);
					if (!baseContainer.Get("Prefab", resourceName) || !resourceName) // .IsEmpty()
						continue;

					resource = Resource.Load(resourceName);
					if (!resource.IsValid())
						continue;

					subResult = GetSeats(resource.GetResource().ToBaseContainer());
					for (int j; j < 4; ++j)
					{
						result[j] = result[j] + subResult[j];
					}
				}
			}
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	// This is the best detection we have for now
	//! \param[in] baseContainer
	//! \return
	protected static bool IsTurretCommanderSeat(notnull BaseContainer compartmentContainer)
	{
		string commanderStr;
		if (compartmentContainer.Get("CompartmentUniqueName", commanderStr))
		{
			commanderStr.ToLower();
			if (commanderStr.Contains("commander"))
				return true;
		}

		BaseContainer baseContainer = compartmentContainer.GetObject("UIInfo");
		if (baseContainer)
		{
			if (baseContainer.Get("Name", commanderStr)) // eventually #AR-VehiclePosition_Commander
			{
				commanderStr.ToLower();
				if (commanderStr.Contains("commander"))
					return true;
			}
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	// PROTECTED METHODS (helper's helpers)
	//------------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	protected static BaseContainer GetSimulationContainer(notnull BaseContainer vehicleContainer)
	{
		IEntityComponentSource componentSource = SCR_BaseContainerTools.FindComponentSource(vehicleContainer, VehicleBaseSimulation);
		if (!componentSource)
			return null;

		return componentSource.GetObject("Simulation");
	}

	//------------------------------------------------------------------------------------------------
	protected static BaseContainer GetEngineContainer(notnull BaseContainer vehicleContainer)
	{
		BaseContainer baseContainer = GetSimulationContainer(vehicleContainer);
		if (!baseContainer)
			return null;

		return baseContainer.GetObject("Engine");
	}

	//------------------------------------------------------------------------------------------------
	protected static BaseContainer GetGearboxContainer(notnull BaseContainer vehicleContainer)
	{
		BaseContainer baseContainer = GetSimulationContainer(vehicleContainer);
		if (!baseContainer)
			return null;

		return baseContainer.GetObject("Gearbox");
	}
}
#endif // WORKBENCH
