set(srcs
	"spi.c")
	
idf_component_register(
	SRCS "${srcs}"
	INCLUDE_DIRS .
	
)	