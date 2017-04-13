
/// <summary> Class decorator 'nontransportable' that indicates a class cannot be transported between isolates. </summary>
export function nontransportable<T extends { new(...args: any[]): {}}>(
    constructor: T) {
    let className = constructor.name;
    return class extends constructor {
        toJSON = function(): string {
            throw new Error(`Object of class "${className}" is not transportable.`);
        }
    };
}
