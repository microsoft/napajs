
//#define BUILDING_NAPA_EXTENSION 1

#include <string.h>
#include <pthread.h>
#include <dlfcn.h>
#include <napa/zone.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <memory>
#include <future>


using namespace std;

#define DEBUG_NAPA 1


int main(int argc, char* argv[])
{
    napa::InitializeFromConsole(argc, argv);

    auto zone = std::make_unique<napa::Zone>("z1");

    zone->BroadcastSync(
	" \
        function estimatePI(points) { \
            var i = points; \
            var inside = 0; \
            var z = []; \
            while (i-- > 0) { \
	            if (i%10000 == 0) console.log(i); \
                var x = Math.random(); \
                var y = Math.random(); \
                z = z.slice(0); z.push(x); z.push(y); \
                if ((x * x) + (y * y) <= 1) { \
                    inside++; \
                } \
            } \
	        return inside / points * 4; \
        } \
\
        function run() { \
            console.log('start run'); \
\
            var napa = require('napajs'); \
            console.log('napajs loaded'); \
            const NUMBER_OF_WORKERS = 4; \
            var zone = napa.zone.create('zone1', { workers: NUMBER_OF_WORKERS }); \
            console.log('zone created with 4 workers'); \
\
            var promises = []; \
            for (var i = 0; i < 4; i++) { \
                promises[i] = zone.execute(estimatePI, [40000]); \
            } \
\
            console.log('4 times napa.zone.execution issued.'); \
            return Promise.all(promises).then(values => { \
                var aggregate = 0; \
                values.forEach(result => aggregate += result.value); \
                console.log('PI: ', aggregate / 4); \
            }) \
        } \
    "
   );

    napa::FunctionSpec spec;
    spec.module = EMPTY_NAPA_STRING_REF;
    spec.function = NAPA_STRING_REF("run");
    spec.arguments = { };

    zone->ExecuteSync(spec);

    /// If you call zone.execute and returns directly, instead of await the result promise get resolved,
    /// the below "while(true)" line is required to allow napa execution to complete. 
    // while(true);

    return 0;
}

