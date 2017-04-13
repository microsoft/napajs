import * as transportable from '../transport/transportable';
import { Handle } from './handle';

/// <summary> Javascript interface for allocator.
/// An allocator must be a native object shared by all isolates, which allocate memory.
/// </summary> 
export interface Allocator extends Handle {
    /// <summary> Allocate memory of requested size in bytes. </summary>
    allocate(size: number): Handle;

    /// <summary> Deallocate memory of requested pointer. </summary>
    deallocate(pointer: Handle): void;
}

/// <summary> Javascript interface for allocator debugger. </summary>
export interface AllocatorDebugger extends Allocator {
    /// <summary> Get debug info. </summary>
    readonly debugInfo: any;
}

// TODO: @dapeng, export CrtAllocator from addon.
export let CrtAllocator: Allocator = null;

export function debugAllocator(allocator: Allocator): AllocatorDebugger {
    /// TODO: @dapeng, return SimpleDebugAllocator from add-on.
    return null;
}