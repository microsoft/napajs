{
  "variables": {
    "napajs_lib": "<!(node -e \"require('napajs/build').paths.lib\")",
    "napajs_inc": "<!(node -e \"require('napajs/build').paths.inc\")"
  },
  "targets": [
    {
      "target_name": "addon.node",
      "type": "<(library)",
      "product_name": "addon",
      "product_extension": "node",
      "product_dir": "<(PRODUCT_DIR)/../../bin",
      "sources": [ "napa/addon.cpp"],
      "include_dirs": ["<(napajs_inc)"]
    }
  ]
}
