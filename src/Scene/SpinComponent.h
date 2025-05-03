#pragma once
#include "pch.h"

#include "BehaviourComponent.h"

class SpinComponent : public BehaviourComponent {
public:
    void update(float delta) override;

};
