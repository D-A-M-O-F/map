FetchContent_Declare(
  ure
  GIT_REPOSITORY https://github.com/fe-dagostino/ure.git
  GIT_TAG        master # master
)

FetchContent_MakeAvailable(ure)


message( STATUS " ADD SIGC++ SRC DIR: [${sigc_SOURCE_DIR}]" )
message( STATUS " ADD SIGC++ BIN DIR: [${sigc_BINARY_DIR}]" )
include_directories( ${sigc_SOURCE_DIR} ${sigc_BINARY_DIR} )

message( STATUS " URE SOURCE DIR    : [${ure_SOURCE_DIR}]" )
include_directories( ${ure_SOURCE_DIR}/3rd-party/glad/include )