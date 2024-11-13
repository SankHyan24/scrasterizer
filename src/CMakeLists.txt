aux_source_directory(./obj_loader _OBJ_LOADER_SRC_)
target_sources(${_EXE_NAME_} PUBLIC ${_OBJ_LOADER_SRC_})
aux_source_directory(./zbuffer _ZBUFFER_SRC_)
target_sources(${_EXE_NAME_} PUBLIC ${_ZBUFFER_SRC_})
