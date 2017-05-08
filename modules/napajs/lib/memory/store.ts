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
