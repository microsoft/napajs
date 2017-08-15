{
  "variables": {
    "napajs_root": "<!(node -e \" \
      var path = require('path'); \
      process.stdout.write(path.resolve(path.dirname(require.resolve('napajs')), '..')); \
    \")"
  },
  "targets": [
    {
      "target_name": "addon.node",
      "type": "<(library)",
      "product_extension": "",
      "product_dir": "<(PRODUCT_DIR)/../../bin",
      "sources": [ "src/addon.cpp"],
      "defines": [
        "NAPA_BINDING_EXPORTS",
        "BUILDING_NODE_EXTENSION"
      ],
      "include_dirs": ["<(napajs_root)/inc"]
    },
    {
      "target_name": "addon.napa",
      "type": "<(library)",
      "product_extension": "",
      "product_dir": "<(PRODUCT_DIR)/../../bin",
      "sources": [ "src/addon.cpp" ],
      "defines": [
        "NAPA_EXPORTS",
        "NAPA_BINDING_EXPORTS",
        "BUILDING_NAPA_EXTENSION"
      ],
      "include_dirs": ["<(napajs_root)/inc"],
      "link_settings": {
        "libraries": ["<(napajs_root)/bin/napa"]
      }
    }
  ]
}
