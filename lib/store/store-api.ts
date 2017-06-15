import { Store } from './store';

let binding = require('../binding');

/// <summary> Create a store with an id. </summary>
/// <param name="id"> String identifier which can be used to get the store from all isolates. </summary>
/// <returns> A store object or throws Error if store with this id already exists. </returns>
/// <remarks> Store object will be destroyed when reference from all isolates are unreferenced. 
/// It's usually a best practice to keep a long-living reference in user modules or global scope. </remarks>
export function create(id: string): Store {
    return binding.createStore(id);
}

/// <summary> Get a store with an id. </summary>
/// <param name="id"> String identifier which is passed to create/getOrCreate earlier. </summary>
/// <returns> A store object if exists, otherwise undefined. </returns>
export function get(id: string): Store {
    return binding.getStore(id);
}

/// <summary> Get a store with an id, or create it if not exist. </summary>
/// <param name="id"> String identifier which can be used to get the store from all isolates. </summary>
/// <returns> A store object associated with the id. </returns>
/// <remarks> Store object will be destroyed when reference from all isolates are unreferenced. 
/// It's usually a best practice to keep a long-living reference in user modules or global scope. </remarks>
export function getOrCreate(id: string): Store {
    return binding.getOrCreateStore(id);
}

/// <summary> Returns number of stores that is alive. </summary>
export function count(): number {
    return binding.getStoreCount();
}