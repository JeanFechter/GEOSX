####################################
# 3rd Party Dependencies
####################################

################################
# Conduit
################################
#message( "CONDUIT_DIR=${CONDUIT_DIR}")
if (CONDUIT_DIR)

  include(components/cmake/thirdparty/FindConduit.cmake)
  blt_register_library( NAME conduit
                        INCLUDES ${CONDUIT_INCLUDE_DIRS} 
                        LIBRARIES  conduit)
  blt_register_library( NAME conduit_io
                        INCLUDES ${CONDUIT_INCLUDE_DIRS}
                        LIBRARIES  conduit_io)
endif()


################################
# HDF5
################################
if (HDF5_DIR)
  include(components/cmake/thirdparty/FindHDF5.cmake)
  blt_register_library(NAME hdf5
                       INCLUDES ${HDF5_INCLUDE_DIRS}
                       LIBRARIES ${HDF5_LIBRARY} )
endif()


################################
# ATK
################################
#if (ATK_DIR)
#  include(cmake/thirdparty/FindATK.cmake)
#  blt_register_library( NAME atk
#                        INCLUDES ${ATK_INCLUDE_DIRS} 
#                        LIBRARIES  sidre)
#endif()

if (ATK_DIR)
  include(components/cmake/thirdparty/FindATK.cmake)
  blt_register_library( NAME sidre
                        INCLUDES ${ATK_INCLUDE_DIRS} 
                        LIBRARIES  sidre)

  blt_register_library( NAME slic
                        INCLUDES ${ATK_INCLUDE_DIRS} 
                        LIBRARIES  slic)
endif()


#if (UNCRUSTIFY_EXECUTABLE)
  include(blt/cmake/thirdparty/FindUncrustify.cmake)
#endif()