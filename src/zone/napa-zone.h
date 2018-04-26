// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include "zone.h"

#include "zone/scheduler.h"
#include "settings/settings.h"

#include <memory>
#include <string>


namespace napa {
namespace zone {

    /// <summary> Concrete implementation of a Napa zone. </summary>
    class NapaZone : public Zone {
    public:

        /// <summary> Creates a new zone with the provided id and settings. </summary>
        static std::shared_ptr<NapaZone> Create(const settings::ZoneSettings& settings);

        /// <summary> Retrieves an existing zone by id. </summary>
        static std::shared_ptr<Zone> Get(const std::string& id);

        /// <summary> Retrieves the current zone. </summary>
        static std::shared_ptr<Zone> GetCurrent();

        /// <see cref="Zone::GetId" />
        virtual const std::string& GetId() const override;

        /// <see cref="Zone::GetState" />
        virtual State GetState() const override;

        /// <see cref="Zone::Broadcast" />
        virtual void Broadcast(const FunctionSpec& spec, BroadcastCallback callback) override;

        /// <see cref="Zone::Execute" />
        virtual void Execute(const FunctionSpec& spec, ExecuteCallback callback) override;

        /// <see cref="Zone::Recycle" />
        virtual void Recycle() override;

    private:
        explicit NapaZone(const settings::ZoneSettings& settings);
        ~NapaZone();

        class Impl;
        std::shared_ptr<Impl> _impl;

        std::atomic<bool> _recycling;
    };
}
}