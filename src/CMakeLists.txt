aux_source_directory(./core _CORE_SRC_)
target_sources(${_EXE_NAME_} PUBLIC ${_CORE_SRC_})

aux_source_directory(./obj_loader _OBJ_LOADER_SRC_)
target_sources(${_EXE_NAME_} PUBLIC ${_OBJ_LOADER_SRC_})

aux_source_directory(./rasterizer _RASTERIZER_SRC_)
target_sources(${_EXE_NAME_} PUBLIC ${_RASTERIZER_SRC_})

aux_source_directory(./window _WINDOW_SRC_)
target_sources(${_EXE_NAME_} PUBLIC ${_WINDOW_SRC_})

aux_source_directory(./zbuffer _ZBUFFER_SRC_)
target_sources(${_EXE_NAME_} PUBLIC ${_ZBUFFER_SRC_})