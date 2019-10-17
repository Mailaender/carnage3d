#pragma once

#include "GameDefs.h"
#include "PhysicsDefs.h"
#include "GameObject.h"
#include "Sprite2D.h"

class SpriteBatch;

// defines vehicle instance
class Vehicle final: public GameObject
{
    friend class GameObjectsManager;

public:
    // public for convenience, should not be modified directly
    CarPhysicsComponent* mPhysicsComponent;

    bool mDead;
    bool mIsOnScreen;

    CarStyle* mCarStyle; // cannot be null

public:
    // @param id: Unique object identifier, constant
    Vehicle(GameObjectID_t id);
    ~Vehicle();

    // setup initial state when spawned on level
    void EnterTheGame();

    void UpdateFrame(Timespan deltaTime);
    void DrawFrame(SpriteBatch& spriteBatch);

    // doors animations
    void OpenDoor(int doorIndex);
    void CloseDoor(int doorIndex);

    bool HasDoorAnimation(int doorIndex) const;
    bool IsDoorOpened(int doorIndex) const;
    bool IsDoorClosed(int doorIndex) const;
    bool IsDoorOpening(int doorIndex) const;
    bool IsDoorClosing(int doorIndex) const;

    // emergency lights animation
    bool HasEmergencyLightsAnimation() const;
    bool IsEmergencyLightsEnabled() const;
    void EnableEmergencyLights(bool isEnabled);

    // get door index for specific seat
    // @param carSeat: Seat identifier, 'any' treated as driver
    int GetDoorIndexForSeat(eCarSeat carSeat) const;

    // get position of specific door in local or world coords
    // local coords is relative to car center
    // @param doorIndex: Door
    bool GetDoorPosLocal(int doorIndex, glm::vec2& out) const;
    bool GetDoorPos(int doorIndex, glm::vec2& out) const;

    // get position of specific seat in local or world coords
    // local coords is relative to car center
    // @param carSeat: Seat
    bool GetSeatPosLocal(eCarSeat carSeat, glm::vec2& out) const;
    bool GetSeatPos(eCarSeat carSeat, glm::vec2& out) const;

private:
    float ComputeDrawHeight(const glm::vec3& position, cxx::angle_t rotationAngle);
    void SetupDeltaAnimations();
    void UpdateDeltaAnimations(Timespan deltaTime);

    SpriteDeltaBits_t GetSpriteDeltas() const;

private:
    Sprite2D mChassisDrawSprite;

    SpriteAnimation mDoorsAnims[MAX_CAR_DOORS];
    SpriteAnimation mEmergLightsAnim;
    SpriteDeltaBits_t mDamageDeltaBits;

    int mChassisSpriteIndex = 0;

    // internal stuff that can be touched only by CarsManager
    cxx::intrusive_node<Vehicle> mActiveCarsNode;
    cxx::intrusive_node<Vehicle> mDeleteCarsNode;
};