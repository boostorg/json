if (INCLUDED__BOOST_JSON_CMAKE__MAKE_CMAKE_INSTALLER__CMAKE)
    return()
else()
    set(INCLUDED__BOOST_JSON_CMAKE__MAKE_CMAKE_INSTALLER__CMAKE 1)
endif()

macro(make_cmake_installer)

    set(generated_dir "${CMAKE_CURRENT_BINARY_DIR}/generated")

    set(config_install_dir "lib/cmake/${PROJECT_NAME}")
    set(include_install_dir "include")
    set(lib_install_dir "lib")
    set(pkgconfig_install_dir "lib/pkgconfig")

    set(version_config "${generated_dir}/${PROJECT_NAME}ConfigVersion.cmake")
    set(project_config "${generated_dir}/${PROJECT_NAME}Config.cmake")
    set(pkg_config "${generated_dir}/${PROJECT_NAME}.pc")
    set(targets_export_name "${PROJECT_NAME}Targets")
    set(namespace "Boost::")

    include(CMakePackageConfigHelpers)
    write_basic_package_version_file(
            "${version_config}" COMPATIBILITY SameMajorVersion
    )

    configure_file("cmake/BoostJson.pc.in" "${pkg_config}" @ONLY)

    configure_package_config_file(
            "cmake/Config.cmake.in"
            "${project_config}"
            INSTALL_DESTINATION "${config_install_dir}"
    )

    install(TARGETS ${BOOST_JSON_TARGET}
            EXPORT "${targets_export_name}"
            INCLUDES DESTINATION "${include_install_dir}"
            ARCHIVE DESTINATION "${lib_install_dir}")

    install(
            FILES "${project_config}" "${version_config}"
            DESTINATION "${config_install_dir}"
    )

    install(
            DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/boost"
            DESTINATION "${include_install_dir}"
    )

    install(
            FILES "${pkg_config}"
            DESTINATION "${pkgconfig_install_dir}"
    )

    install(
            EXPORT "${targets_export_name}"
            NAMESPACE "${namespace}"
            DESTINATION "${config_install_dir}"
    )

endmacro()