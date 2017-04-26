#pragma once

#include <napa/zone.h>

#include "scheduler/scheduler.h"
#include "settings/settings.h"

#include <memory>
#include <string>
#include <unordered_map>


namespace napa {

    /// <summary> Concrete implementation of a zone. </summary>
    class ZoneImpl : public Zone {
    public:

        /// <summary> Creates a new zone with the provided id and settings. </summary>
        static std::shared_ptr<ZoneImpl> Create(const ZoneSettings& settings);

        /// <summary> Retrieves an existing zone by id. </summary>
        static std::shared_ptr<ZoneImpl> Get(const std::string& id);

        /// <see cref="Zone::GetId" />
        virtual const std::string& GetId() const override;

        /// <see cref="Zone::Broadcast" />
        virtual void Broadcast(const std::string& source, BroadcastCallback callback) override;

        /// <see cref="Zone::Execute" />
        virtual void Execute(const ExecuteRequest& request, ExecuteCallback callback) override;

        /// <summary> Retrieves the zone settings. </summary>
        const ZoneSettings& GetSettings() const;

        /// <summary> Retrieves the zone scheduler. </summary>
        scheduler::Scheduler* GetScheduler();

    private:
        explicit ZoneImpl(const ZoneSettings& settings);
        void Init();

        ZoneSettings _settings;
        std::unique_ptr<scheduler::Scheduler> _scheduler;

        static std::mutex _mutex;
        static std::unordered_map<std::string, std::weak_ptr<ZoneImpl>> _zones;
    };

}