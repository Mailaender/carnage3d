#pragma once

#include "PhysicsDefs.h"
#include "GameDefs.h"
#include "PhysicsComponents.h"

// note that the physics only works with meter units (Mt) rather then map units

// this class manages physics and collision detections for map and objects
class PhysicsManager final: private b2ContactListener
{
public:
    PhysicsManager();

    bool InitPhysicsWorld();
    void FreePhysicsWorld();

    void UpdateFrame();

    // create pedestrian specific physical body
    // @param pedestrian: Reference ped
    // @param position: World position, meters
    // @param rotationAngle: Heading
    PedPhysicsComponent* CreatePhysicsComponent(Pedestrian* pedestrian, const glm::vec3& position, cxx::angle_t rotationAngle);

    // create car specific physical body
    // @param car: Reference car
    // @param position: World position, meters
    // @param rotationAngle: Heading
    // @param desc: Car class description
    CarPhysicsComponent* CreatePhysicsComponent(Vehicle* car, const glm::vec3& position, cxx::angle_t rotationAngle, CarStyle* desc);

    // free physics object
    // @param object: Object to destroy, pointer becomes invalid
    void DestroyPhysicsComponent(PedPhysicsComponent* object);
    void DestroyPhysicsComponent(CarPhysicsComponent* object);

    // query all physics objects that intersects with line
    // note that depth is ignored so pointA and pointB has only 2 components
    // @param pointA, pointB: Line of intersect points
    // @param aaboxCenter, aabboxExtents: AABBox area of intersections
    // @param outputResult: Output objects
    void QueryObjectsLinecast(const glm::vec2& pointA, const glm::vec2& pointB, PhysicsLinecastResult& outputResult) const;
    void QueryObjectsWithinBox(const glm::vec2& aaboxCenter, const glm::vec2& aabboxExtents, PhysicsQueryResult& outputResult) const;

private:
    // create level map body, used internally
    void CreateMapCollisionShape();

    // apply gravity forces and correct y coord for objects
    void FixedStepGravity();

    void ProcessSimulationStep(bool resetPreviousState);
    void ProcessInterpolation();

    // override b2ContactFilter
	void BeginContact(b2Contact* contact) override;
	void EndContact(b2Contact* contact) override;
	void PreSolve(b2Contact* contact, const b2Manifold* oldManifold) override;
	void PostSolve(b2Contact* contact, const b2ContactImpulse* impulse) override;

    // pre solve collisions
    bool HasCollisionPedVsPed(b2Contact* contact, PedPhysicsComponent* pedA, PedPhysicsComponent* pedB) const;
    bool HasCollisionCarVsCar(b2Contact* contact, CarPhysicsComponent* carA, CarPhysicsComponent* carB) const;
    bool HasCollisionPedVsMap(int mapx, int mapz, float height) const;
    bool HasCollisionCarVsMap(b2Contact* contact, b2Fixture* fixtureCar, int mapx, int mapz) const;
    bool HasCollisionPedVsCar(b2Contact* contact, PedPhysicsComponent* ped, CarPhysicsComponent* car) const;

    // post solve collisions
    void HandleContactPedVsCar(b2Contact* contact, float impulse, PedPhysicsComponent* ped, CarPhysicsComponent* car);

    bool ProcessSensorContact(b2Contact* contact, bool onBegin);

private:
    b2Body* mMapCollisionShape;
    b2World* mPhysicsWorld;

    float mSimulationTimeAccumulator;
    float mSimulationStepTime;

    float mGravityForce; // meters per second

    // physics components pools
    cxx::object_pool<PedPhysicsComponent> mPedsBodiesPool;
    cxx::object_pool<CarPhysicsComponent> mCarsBodiesPool;

    cxx::intrusive_list<PedPhysicsComponent> mPedsBodiesList;
    cxx::intrusive_list<CarPhysicsComponent> mCarsBodiesList;
};

extern PhysicsManager gPhysics;