# Zip in node worker
This example is to show that in napa's node worker, it is easy to call node's core module, like zip.
It generate many uuid sequence into several temp files, and gzip those files into .gz format. After job finished, all will temp files will be unlinked.
By default those generate/zip operation run in a napa zone. If any parameter given, it will run only in node main.

## How to run
1. It depends on napa directory structure, so first please build napa.
2. Go to directory of "examples/tutorial/zip_in_node_worker".
2. Run "npm install" to install dependencies other than napa.
3. Run "npm start" or "node zip_in_node_worker.js" to run the example.

## Note
1. Some temp files may be left if the example's running is interrupted, remove them manully please.
