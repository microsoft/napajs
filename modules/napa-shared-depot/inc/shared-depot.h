#pragma once

#include "shared-barrel.h"

#include <queue>

namespace napa {
namespace module {

    /// <summary> A shared object depot to manage a set of shared barrels. </summary>
    class SharedDepot {
    public:

        /// <summary> It returns a shared barrel instance. </summary>
        /// <returns> SharedBarrel instance if it succeeds, otherwise, return nullptr. </summary>
        std::shared_ptr<SharedBarrel> Assign();

        /// <summary> It releases a shared barrel. </summary>
        /// <param name="barrel"> SharedBarrel instance created by Assign(). </param>
        /// <remarks> Shared barrel owner must call this function after it's not used any more. </remarks>
        void Release(const std::shared_ptr<SharedBarrel>& barrel);

        /// <summary> It releases a shared barrel. </summary>
        /// <param name="id"> Shared barrel Id. </param>
        /// <remarks> Shared barrel owner must call this function after it's not used any more. </remarks>
        void Release(SharedBarrel::IdType id);

        /// <summary> It finds a shared barrel using a given Id. </summary>
        /// <param name="id"> Shared barral id. </summary>
        /// <returns> SharedBarrel instance if found, otherwise, return nullptr. </summary>
        std::shared_ptr<SharedBarrel> Find(SharedBarrel::IdType id) const;

        /// <summary> It returns the number of assigned shared barrels. </summary>
        size_t GetCount() const;

    private:

        using SharedBarrels = std::vector<std::shared_ptr<SharedBarrel>>;
        using FreeSharedBarrelIds = std::queue<SharedBarrel::IdType>;

        /// <summary> Shared barrels. </summary>
        SharedBarrels _sharedBarrels;

        /// <summary> Free shared barrel Ids. </summary>
        FreeSharedBarrelIds _freeList;

        /// <summary> Critical section for shared barrels. </summary>
        mutable boost::shared_mutex _mutex;
    };

}  // napespace module
}  // namespace napa