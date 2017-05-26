export * from './memory/allocator';
export * from './memory/store';
export * from './memory/handle';
export * from './memory/shareable';

import { Allocator, AllocatorDebugger } from './memory/allocator';

let binding = require('./binding');

/// <summary> Export Crt allocator from napa.dll. </summary>
export let crtAllocator: Allocator = binding.getCrtAllocator();

/// <summary> Export default allocator from napa.dll. </summary>
export let defaultAllocator: Allocator = binding.getDefaultAllocator();

/// <summary> Create a debug allocator around allocator. </summary>
/// <param name="allocator"> User allocator. </param>
export function debugAllocator(allocator: Allocator): AllocatorDebugger {
    return new binding.AllocatorDebuggerWrap(allocator);
}

import { Store } from './memory/store';

/// <summary> Find or create a store with an id. </summary>
export function findOrCreateStore(id: string): Store {
    return binding.findOrCreateStore(id);
}

/// <summary> Find a store with an id. </summary>
export function findStore(id: string): Store {
    return binding.findStore(id);
}

/// <summary> Get store count in use. </summary>
export function getStoreCount(): number {
    return binding.getStoreCount();
}