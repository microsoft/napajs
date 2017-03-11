#include "shared-depot.h"

#define CATCH_CONFIG_MAIN
#include <catch/catch.hpp>

#include <array>
#include <cstdlib>
#include <mutex>
#include <queue>
#include <vector>

using namespace napa::module;

template <typename T>
class Assigned {
public:

    void Push(T id) {
        std::lock_guard<std::mutex> lock(_mutex);
        _assigned.emplace_back(id);
    }

    std::pair<T, bool> PopAny() {
        std::lock_guard<std::mutex> lock(_mutex);
        if (_assigned.empty()) {
            return std::make_pair(T(), false);
        } else {
            auto index = rand() % _assigned.size();
            auto result = std::make_pair(_assigned[index], true);
            _assigned.erase(_assigned.begin() + index);
            return result;
        }
    }

    size_t GetCount() const {
        return _assigned.size();
    }

private:
    std::vector<T> _assigned;
    std::mutex _mutex;
};

using Action = std::function<bool ()>;

template <typename T>
size_t RunCommands(const std::vector<Action>& actions, size_t numCommands) {
    std::queue<T> commands;
    std::mutex commandsLock;

    for (size_t i = 0; i < numCommands; ++i) {
        T commandType =
            static_cast<T>(std::rand() % static_cast<size_t>(T::NUM_COMMAND_TYPES));
        commands.emplace(commandType);
    }

    const size_t NUM_WORKERS = 4;
    std::vector<std::thread> workers;
    workers.reserve(NUM_WORKERS);

    std::atomic<uint32_t> failed(0);

    for (size_t i = 0; i < NUM_WORKERS; ++i) {
        workers.emplace_back([&] {
            while (true) {
                T commandType;
                {
                    std::lock_guard<std::mutex> lock(commandsLock);

                    if (commands.empty()) {
                        break;
                    }

                    commandType = commands.front();
                    commands.pop();
                }

                auto action = actions[static_cast<size_t>(commandType)];
                auto result = action();

                // Can't use REQUIRE() here since Catch only interacts with one thread.
                // Refer to https://github.com/philsquared/Catch/blob/a38ccec33ac309a3495c277cf76f6a3e3df12093/docs/limitations.md
                if (!result) {
                    failed++;
                }
            }
        });
    }

    for (auto& worker: workers) {
        worker.join();
    }

    return failed;
}

TEST_CASE("shared-depot assigns and releases barrels properly.", "[shared-depot]") {
    SharedDepot sharedDepot;
    Assigned<SharedBarrel::IdType> assigned;

    enum class CommandType {
        ASSIGN,
        RELEASE,
        NUM_COMMAND_TYPES
    };

    std::vector<Action> actions = {
        [&]() {
            auto barrel = sharedDepot.Assign();
            if (barrel == nullptr) {
                return false;
            }
            assigned.Push(barrel->GetId());
            return true;
        },
        [&]() {
            auto result = assigned.PopAny();
            if (!result.second) {
                return true;
            }
            auto id = result.first;
            auto barrel = sharedDepot.Find(id);
            if (barrel == nullptr) {
                return false;
            }
            sharedDepot.Release(barrel);
            return true;
        }
    };

    auto failed = RunCommands<CommandType>(actions, 4096);

    REQUIRE(failed == 0);
    REQUIRE(sharedDepot.GetCount() == assigned.GetCount());
}

TEST_CASE("shared-barrel is shared among multiple threads properly.", "[shared-barrel]") {
    SharedDepot sharedDepot;
    auto barrel = sharedDepot.Assign();
    REQUIRE(barrel != nullptr);

    Assigned<size_t> assigned;

    enum class CommandType {
        SET,
        GET,
        REMOVE,
        NUM_COMMAND_TYPES
    };

    struct Dummy {
        Dummy(size_t member) : member(member) {}

        size_t member;
    };

    std::atomic<uint32_t> masterKey(0);

    std::vector<Action> actions = {
        [&]() {
            auto subKey = masterKey++;
            auto key(std::to_string(subKey));

            auto result = barrel->Set(key.c_str(), std::make_shared<Dummy>(subKey));
            assigned.Push(subKey);
            return result;
        },
        [&]() {
            if (masterKey == 0) {
                return true;
            }

            auto result = assigned.PopAny();
            if (!result.second) {
                return true;
            }

            auto subKey = result.first;
            auto key(std::to_string(subKey));

            if (!barrel->Exists(key.c_str())) {
                return false;
            }

            auto dummy = barrel->Get<Dummy>(key.c_str());
            if (dummy->member != subKey) {
                return false;
            }

            assigned.Push(subKey);
            return true;
        },
        [&]() {
            if (masterKey == 0) {
                return true;
            }

            auto result = assigned.PopAny();
            if (!result.second) {
                return true;
            }

            auto subKey = result.first;
            auto key(std::to_string(subKey));

            barrel->Remove(key.c_str());

            auto exist = barrel->Exists(key.c_str());
            return !exist;
        }
    };

    auto failed = RunCommands<CommandType>(actions, 1024);

    REQUIRE(failed == 0);
    REQUIRE(barrel->GetCount() == assigned.GetCount());
}