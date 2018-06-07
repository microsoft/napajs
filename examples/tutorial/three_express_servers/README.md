# Three Express Servers
This example is to show that in napa's node worker, it is easy to run top npm modules like express, and node core modules, like http, etc.
The program will run two express server severing static contents under two directories (docs/server{1,2}), listening on 3001/3002 respectively, in two seperate napa zones.

While node main runs an express server on 3000, which basically reverse proxy http request to the above two server according to prefix of the request's url.

## How to run
1. It depends on napa directory structure, so first please build napa.
2. Go to directory of "examples/tutorial/three_express_servers".
2. Run "npm install" to install dependencies other than napa.
3. Run "npm start" or "node lib/app.js" to start the server.
4. In browser, visit "http://localhost:3000/ to see result and just click links on it to visit content from other two servers behind the proxy.

## Note
1. The express server serving static content could be started by running command like "node lib/server 3001 docs/server1".
2. They just runs in different worker (thread) now in this example.
