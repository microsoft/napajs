import { Handle } from './handle';
import { Shareable } from './shareable';

/// <summary> Javascript interface for allocator.
/// An allocator must be a native object shared by all isolates, which allocate memory.
/// </summary> 
export interface Allocator extends Shareable {
    /// <summary> Allocate memory of requested size in bytes. </summary>
    /// <param name="size"> Size in bytes to allocate. </param>
    allocate(size: number): Handle;

    /// <summary> Deallocate memory of requested handle. </summary>
    /// <param name="handle"> Handle of memory to deallocate. </param>
    /// <param name="sizeHint"> Hint for the size of memory to deallocate. Pass 0 if unknown. </param>
    deallocate(handle: Handle, sizeHint: number): void;

    /// <summary> Type of allocator for better debuggability. </summary>
    readonly type: string;
}

/// <summary> Javascript interface for allocator debugger. </summary>
export interface AllocatorDebugger extends Allocator {
    /// <summary> Get debug info. </summary>
    getDebugInfo(): string;
}

let binding = require('../binding');

/// <summary> Export Crt allocator from napa.dll. </summary>
export let crtAllocator: Allocator = binding.getCrtAllocator();

/// <summary> Export default allocator from napa.dll. </summary>
export let defaultAllocator: Allocator = binding.getDefaultAllocator();

/// <summary> Create a debug allocator around allocator. </summary>
/// <param name="allocator"> User allocator. </param>
export function debugAllocator(allocator: Allocator): AllocatorDebugger {
    return new binding.AllocatorDebuggerWrap(allocator);
}
