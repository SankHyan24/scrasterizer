aux_source_directory(./camera _CAMERA_SRC_)
target_sources(${_EXE_NAME_} PUBLIC ${_CAMERA_SRC_})

aux_source_directory(./obj_loader _OBJ_LOADER_SRC_)
target_sources(${_EXE_NAME_} PUBLIC ${_OBJ_LOADER_SRC_})

aux_source_directory(./rasterizer _RASTERIZER_SRC_)
target_sources(${_EXE_NAME_} PUBLIC ${_RASTERIZER_SRC_})

aux_source_directory(./window _WINDOW_SRC_)
target_sources(${_EXE_NAME_} PUBLIC ${_WINDOW_SRC_})