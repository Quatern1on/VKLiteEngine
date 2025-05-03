#pragma once
#include "pch.h"

class BehaviourComponent : public Component {
public:
    explicit BehaviourComponent() = default;

    BehaviourComponent(const BehaviourComponent&) = delete;

    BehaviourComponent& operator=(const BehaviourComponent&) = delete;

    virtual void update(float delta) = 0;
};
