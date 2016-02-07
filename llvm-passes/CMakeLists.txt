set(JIT_LLVM_MODULES
    ReadAttributes
)

# automatically create modules based on JIT_LLVM_MODULES and directory content
foreach(MODULE ${JIT_LLVM_MODULES})
    aux_source_directory(${CMAKE_SOURCE_DIR}/llvm-passes/${MODULE} ${MODULE}_SRC)
    add_llvm_loadable_module(LLVM${MODULE} ${${MODULE}_SRC})
endforeach()

