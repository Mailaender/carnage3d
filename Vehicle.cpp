#include "stdafx.h"
#include "Vehicle.h"
#include "PhysicsManager.h"
#include "PhysicsObject.h"
#include "GameMapManager.h"

Vehicle::Vehicle(unsigned int id)
    : mActiveCarsNode(this)
    , mDeleteCarsNode(this)
    , mID(id)
    , mPhysicalBody()
    , mDead()
    , mCarStyle()
    , mMarkForDeletion()
{
}

Vehicle::~Vehicle()
{
    if (mPhysicalBody)
    {
        gPhysics.DestroyPhysicsObject(mPhysicalBody);
    }
}

void Vehicle::EnterTheGame()
{
    debug_assert(mCarStyle);

    glm::vec3 startPosition;
    
    mPhysicalBody = gPhysics.CreateVehicleBody(startPosition, 0.0f, mCarStyle);
    debug_assert(mPhysicalBody);

    mMarkForDeletion = false;
    mDead = false;
}

void Vehicle::UpdateFrame(Timespan deltaTime)
{
}

//////////////////////////////////////////////////////////////////////////

bool CarsManager::Initialize()
{
    mIDsCounter = 0;
    return true;
}

void CarsManager::Deinit()
{
    DestroyCarsInList(mActiveCarsList);
    DestroyCarsInList(mDeleteCarsList);
}

void CarsManager::UpdateFrame(Timespan deltaTime)
{
    DestroyPendingCars();
    
    bool hasDeleteCars = false;
    for (Vehicle* currentCar: mActiveCarsList) // warning: dont add or remove cars during this loop
    {
        if (!currentCar->mMarkForDeletion)
        {
            debug_assert(!mDeleteCarsList.contains(&currentCar->mDeleteCarsNode));
            currentCar->UpdateFrame(deltaTime);
        }

        if (currentCar->mMarkForDeletion)
        {
            mDeleteCarsList.insert(&currentCar->mDeleteCarsNode);
            hasDeleteCars = true;
        }
    }

    if (!hasDeleteCars)
        return;

    // deactivate all cars marked for deletion
    for (Vehicle* deleteCar: mDeleteCarsList)
    {
        RemoveFromActiveList(deleteCar);
    }
}

Vehicle* CarsManager::CreateCar(const glm::vec3& position, int carTypeId)
{
    CityStyleData& styleData = gGameMap.mStyleData;

    debug_assert(styleData.IsLoaded());
    debug_assert(carTypeId < (int)styleData.mCars.size());

    unsigned int carID = GenerateUniqueID();

    Vehicle* instance = mCarsPool.create(carID);
    debug_assert(instance);

    AddToActiveList(instance);

    // init
    instance->mCarStyle = &gGameMap.mStyleData.mCars[carTypeId];
    instance->EnterTheGame();
    instance->mPhysicalBody->SetPosition(position);
    return instance;
}

void CarsManager::DestroyCar(Vehicle* car)
{
    debug_assert(car);
    if (car == nullptr)
        return;

    if (mDeleteCarsList.contains(&car->mDeleteCarsNode))
    {
        mDeleteCarsList.remove(&car->mDeleteCarsNode);
    }

    if (mActiveCarsList.contains(&car->mActiveCarsNode))
    {
        mActiveCarsList.remove(&car->mActiveCarsNode);
    }

    mCarsPool.destroy(car);
}

void CarsManager::DestroyPendingCars()
{
    DestroyCarsInList(mDeleteCarsList);
}

void CarsManager::AddToActiveList(Vehicle* car)
{
    debug_assert(car);
    if (car == nullptr)
        return;

    debug_assert(!mActiveCarsList.contains(&car->mActiveCarsNode));
    debug_assert(!mDeleteCarsList.contains(&car->mDeleteCarsNode));
    mActiveCarsList.insert(&car->mActiveCarsNode);
}

unsigned int CarsManager::GenerateUniqueID()
{
    unsigned int newID = ++mIDsCounter;
    if (newID == 0) // overflow
    {
        debug_assert(false);
    }
    return newID;
}

void CarsManager::RemoveFromActiveList(Vehicle* car)
{
    debug_assert(car);
    if (car && mActiveCarsList.contains(&car->mActiveCarsNode))
    {
        mActiveCarsList.remove(&car->mActiveCarsNode);
    }
}

void CarsManager::DestroyCarsInList(cxx::intrusive_list<Vehicle>& carsList)
{
    while (carsList.has_elements())
    {
        cxx::intrusive_node<Vehicle>* carNode = carsList.get_head_node();
        carsList.remove(carNode);

        Vehicle* carInstance = carNode->get_element();
        mCarsPool.destroy(carInstance);
    }
}