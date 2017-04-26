/// <summary> Barrel is a container that creates boundary for sharing objects across isolates. </summary>
/// TODO: @dapeng, implement add-on and connect with this class.
export declare class Barrel {
    /// <summary> ID of this barrel. </summary>
    readonly id: number;

    /// <summary> Number of keys in this barrel. </summary>
    readonly size: number;
    
    /// <summary> Check if this barrel has a key. </summary>
    /// <param name="key"> Case-sensitive string key. </summary>
    /// <returns> True if this barrel has the key. </returns>
    has(key: string): boolean;

    /// <summary> Get JavaScript value by key. </summary>
    /// <param name="key"> Case-sensitive string key. </summary>
    /// <returns> Value for key, undefined if not found. </returns>
    get(key: string): any;

    /// <summary> Insert or update a JavaScript value by key. </summary>
    /// <param name="key"> Case-sensitive string key. </summary>
    /// <param name="value"> Value. Any value of built-in JavaScript types or Transportable subclasses can be accepted. </summary>
    set(key: string, value: any): void;

    /// <summary> Remove a key with its value from this barrel. </summary>
    /// <param name="key"> Case-sensitive string key. </summary>
    remove(key: string): void;

    /// <summary> Return current barrel to Napa.JS barrel pool. </summary>
    release(): void;
} 

/// <summary> Create a Barrel object from Napa.JS barrel pool. </summary>
export function createBarrel(): Barrel {
    // TODO:
    return new Barrel();
} 

/// <summary> Find a living Barrel by id. </summary>
export function findBarrel(id: number): Barrel {
    // TODO:
    return null;
}

/// <summary> Return number of living barrels. </summary>
export function barrelCount(): number {
    // TODO:
    return 0;
}

// TODO:
export let global: Barrel = undefined;