set(sources
)

set(exe_sources
		src/main.cpp
		${sources}
)

set(headers
    include/ayweb/ayweb.hpp
    include/ayweb/http_server.hpp
    include/ayweb/protocol.hpp
    include/ayweb/router.hpp
)

set(test_sources
  src/test.cpp
)
