import {RequestContext} from '../../lib/app';
import * as objectModel from '../../lib/object-model';

export namespace types {
    export function createA(input: {_type: "TypeA", value: number}): number {
        return input.value;
    }

    export function createB(input: {_type: "TypeB", value: any}, context: objectModel.ObjectContext): any {
        return context.create(input.value);
    }
}

export namespace providers {
    export function provideA(uri: objectModel.Uri): string {
        return uri.path;
    }
}

export namespace entrypoints {
    export function foo(context: RequestContext, input: string): string {
        return input;
    }

    export function bar(context: RequestContext, input: string): Promise<string> {
        return new Promise(resolve => {
            setTimeout(() => {
                resolve(input);
            }, 20);
        });
    }

    export function alwaysThrow() {
        throw new Error("You hit an always-throw entrypoint.");
    }
}