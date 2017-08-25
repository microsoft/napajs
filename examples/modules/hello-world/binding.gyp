{
  "variables": {
    "napajs_lib": "<!(node -e \"require('napajs/build').paths.lib\")",
    "napajs_inc": "<!(node -e \"require('napajs/build').paths.inc\")"
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
      "include_dirs": ["<(napajs_inc)"]
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
      "include_dirs": ["<(napajs_inc)"],
      "link_settings": {
        "libraries": ["<(napajs_lib)"]
      }
    }
  ]
}
