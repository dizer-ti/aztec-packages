include(ExternalProject)

# External project: Download msgpack-c from GitHub
set(MSGPACK_PREFIX "${CMAKE_BINARY_DIR}/_deps/msgpack-c")
set(MSGPACK_INCLUDE "${MSGPACK_PREFIX}/src/msgpack-c/include")

ExternalProject_Add(
    msgpack-c
    PREFIX ${MSGPACK_PREFIX}
    GIT_REPOSITORY "https://github.com/AztecProtocol/msgpack-c.git"
    GIT_TAG 5ee9a1c8c325658b29867829677c7eb79c433a98
    CONFIGURE_COMMAND ""  # No configure step
    BUILD_COMMAND ""      # No build step
    INSTALL_COMMAND ""    # No install step
    UPDATE_COMMAND ""     # No update step
)
