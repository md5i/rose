if(enable-binary-analysis)
  include_directories(. ${ROSE_INCLUDES})

  add_library(bat SHARED batSupport.C)
  target_link_libraries(bat ROSE_DLL)

  add_executable(bat-ana bat-ana.C)
  target_link_libraries(bat-ana bat ROSE_DLL)
  install(TARGETS bat-ana DESTINATION bin)

  add_executable(bat-cc bat-cc.C)
  target_link_libraries(bat-cc bat ROSE_DLL)
  install(TARGETS bat-cc DESTINATION bin)

  add_executable(bat-cfg bat-cfg.C)
  target_link_libraries(bat-cfg bat ROSE_DLL)
  install(TARGETS bat-cfg DESTINATION bin)

  add_executable(bat-cg bat-cg.C)
  target_link_libraries(bat-cg bat ROSE_DLL)
  install(TARGETS bat-cg DESTINATION bin)

  add_executable(bat-container bat-container.C)
  target_link_libraries(bat-container bat ROSE_DLL)
  install(TARGETS bat-container DESTINATION bin)

  add_executable(bat-dis bat-dis.C)
  target_link_libraries(bat-dis bat ROSE_DLL)
  install(TARGETS bat-dis DESTINATION bin)

  add_executable(bat-flir-ascribe bat-flir-ascribe.C)
  target_link_libraries(bat-flir-ascribe bat ROSE_DLL)
  install(TARGETS bat-flir-ascribe DESTINATION bin)

  add_executable(bat-flir-insert bat-flir-insert.C)
  target_link_libraries(bat-flir-insert bat ROSE_DLL)
  install(TARGETS bat-flir-insert DESTINATION bin)

  add_executable(bat-flir-lib bat-flir-lib.C)
  target_link_libraries(bat-flir-lib bat ROSE_DLL)
  install(TARGETS bat-flir-lib DESTINATION bin)

  add_executable(bat-insnfreq bat-insnfreq.C)
  target_link_libraries(bat-insnfreq bat ROSE_DLL)
  install(TARGETS bat-insnfreq DESTINATION bin)

  add_executable(bat-insnfreq-cmp bat-insnfreq-cmp.C)
  target_link_libraries(bat-insnfreq-cmp bat ROSE_DLL)
  install(TARGETS bat-insnfreq-cmp DESTINATION bin)

  add_executable(bat-entropy bat-entropy.C)
  target_link_libraries(bat-entropy bat ROSE_DLL)
  install(TARGETS bat-entropy DESTINATION bin)

  add_executable(bat-lsb bat-lsb.C)
  target_link_libraries(bat-lsb bat ROSE_DLL)
  install(TARGETS bat-lsb DESTINATION bin)

  add_executable(bat-lsd bat-lsd.C)
  target_link_libraries(bat-lsd bat ROSE_DLL)
  install(TARGETS bat-lsd DESTINATION bin)

  add_executable(bat-lsf bat-lsf.C)
  target_link_libraries(bat-lsf bat ROSE_DLL)
  install(TARGETS bat-lsf DESTINATION bin)

  add_executable(bat-mem bat-mem.C)
  target_link_libraries(bat-mem bat ROSE_DLL)
  install(TARGETS bat-mem DESTINATION bin)

  add_executable(bat-prop bat-prop.C)
  target_link_libraries(bat-prop bat ROSE_DLL)
  install(TARGETS bat-prop DESTINATION bin)

  add_executable(bat-stack-deltas bat-stack-deltas.C)
  target_link_libraries(bat-stack-deltas bat ROSE_DLL)
  install(TARGETS bat-stack-deltas DESTINATION bin)

  add_executable(bat-trace bat-trace.C)
  target_link_libraries(bat-trace bat ROSE_DLL)
  install(TARGETS bat-trace DESTINATION bin)

  add_executable(bat-var bat-var.C)
  target_link_libraries(bat-var bat ROSE_DLL)
  install(TARGETS bat-var DESTINATION bin)

endif()
