add_subdirectory(include/glog)
target_link_libraries(ezsmt PRIVATE glog::glog ${Boost_LIBRARIES})
target_link_libraries(test PRIVATE glog::glog ${Boost_LIBRARIES})


target_sources(ezsmt
  PUBLIC
    ctable.cc
    api.cc
    atomrule.cc
    read.cc
    timer.cc
    cmodels.cc
    program.cc
    interpret.cc
    graphscc.cc
    solver/smtsolver.cc
    solver/smtprocess.cc
    logics/QF_LIA_logic.cc
    main.cc
)

target_sources(test
  PUBLIC
    ctable.cc
    api.cc
    atomrule.cc
    read.cc
    timer.cc
    cmodels.cc
    program.cc
    interpret.cc
    graphscc.cc
    solver/smtsolver.cc
    solver/smtprocess.cc
    logics/QF_LIA_logic.cc
    # main.cc
)
