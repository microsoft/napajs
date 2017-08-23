// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include "zone.h"

#include "settings/settings.h"
#include "zone/scheduler.h"

#include <memory>
#include <string>
#include <unordered_map>


namespace napa {
namespace zone {

    /// <summary> Concrete implementation of a Napa zone. </summary>
    class NapaZone : public Zone {
    public:
        /// <summary> Creates a new zone with the provided id and settings. </summary>
        static std::shared_ptr<NapaZone> Create(const settings::ZoneSettings& settings);

        /// <summary> Retrieves an existing zone by id. </summary>
        static std::shared_ptr<NapaZone> Get(const std::string& id);

        /// <see cref="Zone::GetId" />
        virtual const std::string& GetId() const override;

        /// <see cref="Zone::Broadcast" />
        virtual void Broadcast(const std::string& source, BroadcastCallback callback) override;

        /// <see cref="Zone::Execute" />
        virtual void Execute(const FunctionSpec& spec, ExecuteCallback callback) override;

        /// <summary> Retrieves the zone settings. </summary>
        const settings::ZoneSettings& GetSettings() const;

        /// <summary> Retrieves the zone scheduler. </summary>
        /// <remark> Asynchronous works keep the reference on scheduler, so they can finish up safely. </remarks>
        std::shared_ptr<zone::Scheduler> GetScheduler();

    private:
        explicit NapaZone(const settings::ZoneSettings& settings);

        settings::ZoneSettings _settings;
        std::shared_ptr<zone::Scheduler> _scheduler;

        static std::mutex _mutex;
        static std::unordered_map<std::string, std::weak_ptr<NapaZone>> _zones;
    };
} // namespace zone
} // namespace napa