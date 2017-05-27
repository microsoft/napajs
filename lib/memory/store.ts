/// <summary> Store is a facility to share (built-in JavaScript types or Transportable subclasses) objects across isolates. </summary>
export interface Store {
    /// <summary> Id of this store. </summary>
    readonly id: string;

    /// <summary> Number of keys in this store. </summary>
    readonly size: number;
    
    /// <summary> Check if this store has a key. </summary>
    /// <param name="key"> Case-sensitive string key. </summary>
    /// <returns> True if this store has the key. </returns>
    has(key: string): boolean;

    /// <summary> Get JavaScript value by key. </summary>
    /// <param name="key"> Case-sensitive string key. </summary>
    /// <returns> Value for key, undefined if not found. </returns>
    get(key: string): any;

    /// <summary> Insert or update a JavaScript value by key. </summary>
    /// <param name="key"> Case-sensitive string key. </summary>
    /// <param name="value"> Value. Any value of built-in JavaScript types or Transportable subclasses can be accepted. </summary>
    set(key: string, value: any): void;

    /// <summary> Remove a key with its value from this store. </summary>
    /// <param name="key"> Case-sensitive string key. </summary>
    delete(key: string): void;
}

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