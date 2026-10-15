# Fishing prototype

`LV_FISHING_master/fishing_master.umap` is the main fishing level.
Its sublevels, alternate layouts, and `fishing_assets.umap` share props and materials.

Third-party assets were removed for the open-source release. The level layout,
4D geometry, water materials, and most original base-color textures remain.
Many surfaces lost normal maps and ambient occlusion, so they appear flatter.
The firepit lost its burned-wood textures; its black body and red center are now
approximated with plain colors. Hammered iron lost its base-color and roughness
textures and now uses a dark metal color with a fixed roughness value.
Some rock and mountain surfaces also use fixed roughness after texture removal.
